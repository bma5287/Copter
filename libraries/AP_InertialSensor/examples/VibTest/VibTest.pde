// -*- tab-width: 4; Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*-

//
// test harness for vibration testing
//

#include <stdarg.h>
#include <AP_Common.h>
#include <AP_Progmem.h>
#include <AP_HAL.h>
#include <AP_HAL_AVR.h>
#include <AP_HAL_AVR_SITL.h>
#include <AP_HAL_PX4.h>
#include <AP_HAL_Empty.h>
#include <AP_Math.h>
#include <AP_Param.h>
#include <AP_ADC.h>
#include <AP_InertialSensor.h>
#include <AP_Notify.h>
#include <AP_GPS.h>
#include <AP_Baro.h>
#include <Filter.h>
#include <DataFlash.h>
#include <GCS_MAVLink.h>
#include <AP_Mission.h>
#include <AP_AHRS.h>
#include <AP_Airspeed.h>
#include <AP_Vehicle.h>
#include <AP_ADC_AnalogSource.h>
#include <AP_Compass.h>
#include <AP_Scheduler.h>
#include <AP_Declination.h>
#include <AP_Notify.h>
#include <AP_NavEKF.h>
#if CONFIG_HAL_BOARD == HAL_BOARD_PX4

#include <drivers/drv_accel.h>
#include <drivers/drv_hrt.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

const AP_HAL::HAL& hal = AP_HAL_BOARD_DRIVER;

static int accel_fd[INS_MAX_INSTANCES];
static int gyro_fd[INS_MAX_INSTANCES];
static uint32_t total_samples[INS_MAX_INSTANCES];
static uint64_t last_accel_timestamp[INS_MAX_INSTANCES];
static uint64_t last_gyro_timestamp[INS_MAX_INSTANCES];
static DataFlash_File DataFlash("/fs/microsd/VIBTEST");

#define LOG_ACC1_MSG 215
#define LOG_GYR1_MSG 225

struct PACKED log_ACCEL {
    LOG_PACKET_HEADER;
    uint32_t timestamp;
    uint32_t timestamp_us;
    float AccX, AccY, AccZ;
};

struct PACKED log_GYRO {
    LOG_PACKET_HEADER;
    uint32_t timestamp;
    uint32_t timestamp_us;
    float GyrX, GyrY, GyrZ;
};

static const struct LogStructure log_structure[] PROGMEM = {
    LOG_COMMON_STRUCTURES,
    { LOG_ACC1_MSG, sizeof(log_ACCEL),       
      "ACC1", "IIfff",        "TimeMS,TimeUS,AccX,AccY,AccZ" },
    { LOG_ACC1_MSG+1, sizeof(log_ACCEL),       
      "ACC2", "IIfff",        "TimeMS,TimeUS,AccX,AccY,AccZ" },
    { LOG_ACC1_MSG+2, sizeof(log_ACCEL),       
      "ACC3", "IIfff",        "TimeMS,TimeUS,AccX,AccY,AccZ" },
    { LOG_GYR1_MSG, sizeof(log_GYRO),       
      "GYR1", "IIfff",        "TimeMS,TimeUS,GyrX,GyrY,GyrZ" },
    { LOG_GYR1_MSG+1, sizeof(log_GYRO),       
      "GYR2", "IIfff",        "TimeMS,TimeUS,GyrX,GyrY,GyrZ" },
    { LOG_GYR1_MSG+2, sizeof(log_GYRO),       
      "GYR3", "IIfff",        "TimeMS,TimeUS,GyrX,GyrY,GyrZ" }
};

void setup(void)
{
    for (uint8_t i=0; i<INS_MAX_INSTANCES; i++) {
        char accel_path[] = ACCEL_DEVICE_PATH "n";
        char gyro_path[] = GYRO_DEVICE_PATH "n";
        accel_path[strlen(accel_path)-1] = (i==0?0:'1'+(i-1));
        gyro_path[strlen(gyro_path)-1] = (i==0?0:'1'+(i-1));
        accel_fd[i] = open(accel_path, O_RDONLY);
        gyro_fd[i] = open(gyro_path, O_RDONLY);
    }
    if (accel_fd[0] == -1 || gyro_fd[-1] == -1) {
            hal.scheduler->panic("Failed to open accel/gyro 0");
    }

    for (uint8_t i=0; i<INS_MAX_INSTANCES; i++) {
        // disable software filtering
        if (accel_fd[i] != -1) {
            ioctl(accel_fd[i], ACCELIOCSLOWPASS, 0);
            ioctl(accel_fd[i], SENSORIOCSQUEUEDEPTH, 100);
        }
        if (gyro_fd[i] != -1) {
            ioctl(gyro_fd[i], GYROIOCSLOWPASS, 0);
            ioctl(gyro_fd[i], SENSORIOCSQUEUEDEPTH, 100);
        }
    }

    DataFlash.Init(log_structure, sizeof(log_structure)/sizeof(log_structure[0]));
    DataFlash.StartNewLog();
}

void loop(void)
{
    bool got_sample = false;
    do {
        got_sample = false;
        for (uint8_t i=0; i<INS_MAX_INSTANCES; i++) {
            struct accel_report	accel_report;
            struct gyro_report	gyro_report;
            
            if (accel_fd[i] != -1 && ::read(accel_fd[i], &accel_report, sizeof(accel_report)) == 
                sizeof(accel_report) &&
                accel_report.timestamp != last_accel_timestamp[i]) {        
                last_accel_timestamp[i] = accel_report.timestamp;

                struct log_ACCEL pkt = {
                    LOG_PACKET_HEADER_INIT((uint8_t)(LOG_ACC1_MSG+i)),
                    timestamp : (uint32_t)(accel_report.timestamp/1000),
                    timestamp_us : (uint32_t)accel_report.timestamp,
                    AccX      : accel_report.x,
                    AccY      : accel_report.y,
                    AccZ      : accel_report.z
                };
                DataFlash.WriteBlock(&pkt, sizeof(pkt));
                got_sample = true;
                total_samples[i]++;
            }
            if (gyro_fd[i] != -1 && ::read(gyro_fd[i], &gyro_report, sizeof(gyro_report)) == 
                sizeof(gyro_report) &&
                gyro_report.timestamp != last_gyro_timestamp[i]) {        
                last_gyro_timestamp[i] = gyro_report.timestamp;

                struct log_GYRO pkt = {
                    LOG_PACKET_HEADER_INIT((uint8_t)(LOG_GYR1_MSG+i)),
                    timestamp : (uint32_t)(accel_report.timestamp/1000),
                    timestamp_us : (uint32_t)accel_report.timestamp,
                    GyrX      : gyro_report.x,
                    GyrY      : gyro_report.y,
                    GyrZ      : gyro_report.z
                };
                DataFlash.WriteBlock(&pkt, sizeof(pkt));
                got_sample = true;
                total_samples[i]++;
            }
        }
        if (got_sample) {
            if (total_samples[0] % 2000 == 0) {
                hal.console->printf("t=%lu total_samples=%lu/%lu/%lu\n",
                                    hal.scheduler->millis(), 
                                    total_samples[0], total_samples[1],total_samples[2]);
            }
        }
    } while (got_sample);
    hal.scheduler->delay_microseconds(200);
}

#else
const AP_HAL::HAL& hal = AP_HAL_BOARD_DRIVER;
void setup() {}
void loop() {}
#endif // CONFIG_HAL_BOARD

AP_HAL_MAIN();
