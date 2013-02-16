// -*- tab-width: 4; Mode: C++; c-basic-offset: 4; indent-tabs-mode: t -*-
/*
  SITL handling

  This simulates a GPS on a serial port

  Andrew Tridgell November 2011
 */

#include <AP_HAL.h>
#if CONFIG_HAL_BOARD == HAL_BOARD_AVR_SITL

#include <AP_HAL_AVR.h>
#include <AP_HAL_AVR_SITL.h>
#include "AP_HAL_AVR_SITL_Namespace.h"
#include "HAL_AVR_SITL_Class.h"

#include <AP_Math.h>
#include "../SITL/SITL.h"
#include "Scheduler.h"
#include "UARTDriver.h"
#include "../AP_GPS/AP_GPS.h"
#include "../AP_GPS/AP_GPS_UBLOX.h"
#include <sys/ioctl.h>
#include <unistd.h>
#include <time.h>

using namespace AVR_SITL;
extern const AP_HAL::HAL& hal;

#define MAX_GPS_DELAY 100

struct gps_data {
	double latitude;
	double longitude;
	float altitude;
	double speedN;
	double speedE;
	bool have_lock;
} gps_data[MAX_GPS_DELAY];

static uint8_t next_gps_index;
static uint8_t gps_delay;

// state of GPS emulation
static struct {
	/* pipe emulating UBLOX GPS serial stream */
	int gps_fd, client_fd;
	uint32_t last_update; // milliseconds
} gps_state;

/*
  hook for reading from the GPS pipe
 */
ssize_t SITL_State::gps_read(int fd, void *buf, size_t count)
{
#ifdef FIONREAD
	// use FIONREAD to get exact value if possible
	int num_ready;
	while (ioctl(fd, FIONREAD, &num_ready) == 0 && num_ready > 256) {
		// the pipe is filling up - drain it
		uint8_t tmp[128];
		if (read(fd, tmp, sizeof(tmp)) != sizeof(tmp)) {
			break;
		}
	}
#endif
	return read(fd, buf, count);
}

/*
  setup GPS input pipe
 */
int SITL_State::gps_pipe(void)
{
	int fd[2];
	if (gps_state.client_fd != 0) {
		return gps_state.client_fd;
	}
	pipe(fd);
	gps_state.gps_fd    = fd[1];
	gps_state.client_fd = fd[0];
	gps_state.last_update = _scheduler->millis();
	AVR_SITL::SITLUARTDriver::_set_nonblocking(gps_state.gps_fd);
	AVR_SITL::SITLUARTDriver::_set_nonblocking(fd[0]);
	return gps_state.client_fd;
}


/*
  send a UBLOX GPS message
 */
static void _gps_send_ubx(uint8_t msgid, uint8_t *buf, uint16_t size)
{
        const uint8_t PREAMBLE1 = 0xb5;
        const uint8_t PREAMBLE2 = 0x62;
        const uint8_t CLASS_NAV = 0x1;
	uint8_t hdr[6], chk[2];
	hdr[0] = PREAMBLE1;
	hdr[1] = PREAMBLE2;
	hdr[2] = CLASS_NAV;
	hdr[3] = msgid;
	hdr[4] = size & 0xFF;
	hdr[5] = size >> 8;
	chk[0] = chk[1] = hdr[2];
	chk[1] += (chk[0] += hdr[3]);
	chk[1] += (chk[0] += hdr[4]);
	chk[1] += (chk[0] += hdr[5]);
	for (uint8_t i=0; i<size; i++) {
		chk[1] += (chk[0] += buf[i]);
	}
	write(gps_state.gps_fd, hdr, sizeof(hdr));
	write(gps_state.gps_fd, buf, size);
	write(gps_state.gps_fd, chk, sizeof(chk));
}


/*
  send a new set of GPS UBLOX packets
 */
static void _update_gps_ubx(const struct gps_data *d)
{
	#pragma pack(push,1)
	struct ubx_nav_posllh {
		uint32_t	time; // GPS msToW
		int32_t		longitude;
		int32_t		latitude;
		int32_t		altitude_ellipsoid;
		int32_t		altitude_msl;
		uint32_t	horizontal_accuracy;
		uint32_t	vertical_accuracy;
	} pos;
	struct ubx_nav_status {
		uint32_t	time;				// GPS msToW
		uint8_t		fix_type;
		uint8_t		fix_status;
		uint8_t		differential_status;
		uint8_t		res;
		uint32_t	time_to_first_fix;
		uint32_t	uptime;				// milliseconds
	} status;
	struct ubx_nav_velned {
		uint32_t	time;				// GPS msToW
		int32_t		ned_north;
		int32_t		ned_east;
		int32_t		ned_down;
		uint32_t	speed_3d;
		uint32_t	speed_2d;
		int32_t		heading_2d;
		uint32_t	speed_accuracy;
		uint32_t	heading_accuracy;
	} velned;
	struct ubx_nav_solution {
		uint32_t time;
		int32_t time_nsec;
		int16_t week;
		uint8_t fix_type;
		uint8_t fix_status;
		int32_t ecef_x;
		int32_t ecef_y;
		int32_t ecef_z;
		uint32_t position_accuracy_3d;
		int32_t ecef_x_velocity;
		int32_t ecef_y_velocity;
		int32_t ecef_z_velocity;
		uint32_t speed_accuracy;
		uint16_t position_DOP;
		uint8_t res;
		uint8_t satellites;
		uint32_t res2;
	} sol;
	#pragma pack(pop)
        const uint8_t MSG_POSLLH = 0x2;
	const uint8_t MSG_STATUS = 0x3;
	const uint8_t MSG_VELNED = 0x12;
        const uint8_t MSG_SOL = 0x6;

	pos.time = hal.scheduler->millis(); // FIX
	pos.longitude = d->longitude * 1.0e7;
	pos.latitude  = d->latitude * 1.0e7;
	pos.altitude_ellipsoid = d->altitude*1000.0;
	pos.altitude_msl = d->altitude*1000.0;
	pos.horizontal_accuracy = 5;
	pos.vertical_accuracy = 10;

	status.time = pos.time;
	status.fix_type = d->have_lock?3:0;
	status.fix_status = d->have_lock?1:0;
	status.differential_status = 0;
	status.res = 0;
	status.time_to_first_fix = 0;
	status.uptime = hal.scheduler->millis();

	velned.time = pos.time;
	velned.ned_north = 100.0 * d->speedN;
	velned.ned_east  = 100.0 * d->speedE;
	velned.ned_down  = 0;
	velned.speed_2d = pythagorous2(d->speedN, d->speedE) * 100;
	velned.speed_3d = velned.speed_2d;
	velned.heading_2d = ToDeg(atan2f(d->speedE, d->speedN)) * 100000.0;
	if (velned.heading_2d < 0.0) {
		velned.heading_2d += 360.0 * 100000.0;
	}
	velned.speed_accuracy = 2;
	velned.heading_accuracy = 4;

	memset(&sol, 0, sizeof(sol));
	sol.fix_type = d->have_lock?3:0;
	sol.fix_status = 221;
	sol.satellites = d->have_lock?10:3;

	_gps_send_ubx(MSG_POSLLH, (uint8_t*)&pos, sizeof(pos));
	_gps_send_ubx(MSG_STATUS, (uint8_t*)&status, sizeof(status));
	_gps_send_ubx(MSG_VELNED, (uint8_t*)&velned, sizeof(velned));
	_gps_send_ubx(MSG_SOL,    (uint8_t*)&sol, sizeof(sol));
}

static void swap_uint32(uint32_t *v, uint8_t n)
{
	while (n--) {
		*v = htonl(*v);
		v++;
	}
}

/*
  MTK type simple checksum
 */
static void mtk_checksum(const uint8_t *data, uint8_t n, uint8_t *ck_a, uint8_t *ck_b)
{
	*ck_a = *ck_b = 0;
	while (n--) {
		*ck_a += *data++;
		*ck_b += *ck_a;
	}
}

/*
  send a new GPS MTK packet
 */
static void _update_gps_mtk(const struct gps_data *d)
{
    #pragma pack(push,1)
    struct mtk_msg {
	    uint8_t preamble1;
	    uint8_t preamble2;
	    uint8_t msg_class;
	    uint8_t msg_id;
	    int32_t latitude;
	    int32_t longitude;
	    int32_t altitude;
	    int32_t ground_speed;
	    int32_t ground_course;
	    uint8_t satellites;
	    uint8_t fix_type;
	    uint32_t utc_time;
	    uint8_t ck_a;
	    uint8_t ck_b;
    } p;
    #pragma pack(pop)

	p.preamble1     = 0xb5;
	p.preamble2     = 0x62;
	p.msg_class     = 1;
	p.msg_id        = 5;
    p.latitude      = d->latitude  * 1.0e6;
    p.longitude     = d->longitude * 1.0e6;
    p.altitude      = d->altitude * 100;
    p.ground_speed  = pythagorous2(d->speedN, d->speedE) * 100;
    p.ground_course = ToDeg(atan2f(d->speedE, d->speedN)) * 1000000.0;
	if (p.ground_course < 0.0) {
		p.ground_course += 360.0 * 1000000.0;
	}
    p.satellites    = d->have_lock?10:3;
    p.fix_type      = d->have_lock?3:1;
    p.utc_time      = time(NULL);

    swap_uint32((uint32_t *)&p.latitude, 5);
    swap_uint32((uint32_t *)&p.utc_time, 1);
	mtk_checksum(&p.msg_class, sizeof(p)-4, &p.ck_a, &p.ck_b);

	write(gps_state.gps_fd, &p, sizeof(p));
}

/*
  possibly send a new GPS packet
 */
void SITL_State::_update_gps(double latitude, double longitude, float altitude,
			     double speedN, double speedE, bool have_lock)
{
	struct gps_data d;

	// 5Hz, to match the real config in APM
	if (hal.scheduler->millis() - gps_state.last_update < 200) {
		return;
	}
	gps_state.last_update = hal.scheduler->millis();

	d.latitude = latitude;
	d.longitude = longitude;
	d.altitude = altitude;
	d.speedN = speedN;
	d.speedE = speedE;
	d.have_lock = have_lock;

	// add in some GPS lag
	gps_data[next_gps_index++] = d;
	if (next_gps_index >= gps_delay) {
		next_gps_index = 0;
	}

	d = gps_data[next_gps_index];

	if (_sitl->gps_delay != gps_delay) {
		// cope with updates to the delay control
		gps_delay = _sitl->gps_delay;
		for (uint8_t i=0; i<gps_delay; i++) {
			gps_data[i] = d;
		}
	}

	if (gps_state.gps_fd == 0) {
		return;
	}

	switch ((SITL::GPSType)_sitl->gps_type.get()) {
	case SITL::GPS_TYPE_UBLOX:
		_update_gps_ubx(&d);
		break;

	case SITL::GPS_TYPE_MTK:
		_update_gps_mtk(&d);
		break;

	case SITL::GPS_TYPE_MTK16:
	case SITL::GPS_TYPE_MTK19:
		break;
	}
}

#endif
