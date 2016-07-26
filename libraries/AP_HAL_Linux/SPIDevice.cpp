/// -*- tab-width: 4; Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*-
/*
 * Copyright (C) 2015  Intel Corporation. All rights reserved.
 *
 * This file is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "SPIDevice.h"

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/spi/spidev.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

#include <AP_HAL/AP_HAL.h>
#include <AP_HAL/utility/OwnPtr.h>

#include "GPIO.h"
#include "Util.h"

namespace Linux {

static const AP_HAL::HAL &hal = AP_HAL::get_HAL();

#define MHZ (1000U*1000U)
#define KHZ (1000U)
#define SPI_CS_KERNEL -1

struct SPIDesc {
    SPIDesc(const char *name_, uint16_t bus_, uint16_t subdev_, uint8_t mode_,
            uint8_t bits_per_word_, int16_t cs_pin_, uint32_t lowspeed_,
            uint32_t highspeed_)
        : name(name_), bus(bus_), subdev(subdev_), mode(mode_)
        , bits_per_word(bits_per_word_), cs_pin(cs_pin_), lowspeed(lowspeed_)
        , highspeed(highspeed_)
    {
    }

    const char *name;
    uint16_t bus;
    uint16_t subdev;
    uint8_t mode;
    uint8_t bits_per_word;
    int16_t cs_pin;
    uint32_t lowspeed;
    uint32_t highspeed;
};


#if CONFIG_HAL_BOARD_SUBTYPE == HAL_BOARD_SUBTYPE_LINUX_PXF || CONFIG_HAL_BOARD_SUBTYPE == HAL_BOARD_SUBTYPE_LINUX_ERLEBOARD
SPIDesc SPIDeviceManager::_device[] = {
    // different SPI tables per board subtype
    SPIDesc("lsm9ds0_am", 1, 0, SPI_MODE_3, 8, BBB_P9_17,  10*MHZ,10*MHZ),
    SPIDesc("lsm9ds0_g",  1, 0, SPI_MODE_3, 8, BBB_P8_9,   10*MHZ,10*MHZ),
    SPIDesc("ms5611",     2, 0, SPI_MODE_3, 8, BBB_P9_42,  10*MHZ,10*MHZ),
    SPIDesc("mpu6000",    2, 0, SPI_MODE_3, 8, BBB_P9_28,  500*1000, 20*MHZ),
    /* MPU9250 is restricted to 1MHz for non-data and interrupt registers */
    SPIDesc("mpu9250",    2, 0, SPI_MODE_3, 8, BBB_P9_23,  1*MHZ, 20*MHZ),
};
#elif CONFIG_HAL_BOARD_SUBTYPE == HAL_BOARD_SUBTYPE_LINUX_MINLURE
SPIDesc SPIDeviceManager::_device[] = {
    SPIDesc("mpu6000",    0, 0, SPI_MODE_3, 8, SPI_CS_KERNEL, 1*MHZ, 15*MHZ)
};
#elif CONFIG_HAL_BOARD_SUBTYPE == HAL_BOARD_SUBTYPE_LINUX_NAVIO || CONFIG_HAL_BOARD_SUBTYPE == HAL_BOARD_SUBTYPE_LINUX_NAVIO2
SPIDesc SPIDeviceManager::_device[] = {
    /* MPU9250 is restricted to 1MHz for non-data and interrupt registers */
    SPIDesc("mpu9250",    0, 1, SPI_MODE_0, 8, SPI_CS_KERNEL,  1*MHZ, 20*MHZ),
    SPIDesc("ublox",      0, 0, SPI_MODE_0, 8, SPI_CS_KERNEL,  5*MHZ, 5*MHZ),
#if CONFIG_HAL_BOARD_SUBTYPE == HAL_BOARD_SUBTYPE_LINUX_NAVIO2
    SPIDesc("lsm9ds1_m",  0, 2, SPI_MODE_0, 8, SPI_CS_KERNEL,  1*MHZ, 10*MHZ),
#endif
};
#elif CONFIG_HAL_BOARD_SUBTYPE == HAL_BOARD_SUBTYPE_LINUX_ERLEBRAIN2 || \
      CONFIG_HAL_BOARD_SUBTYPE == HAL_BOARD_SUBTYPE_LINUX_PXFMINI
SPIDesc SPIDeviceManager::_device[] = {
    /* MPU9250 is restricted to 1MHz for non-data and interrupt registers */
    SPIDesc("mpu9250",    0, 1, SPI_MODE_0, 8, SPI_CS_KERNEL,  1*MHZ, 20*MHZ),
    SPIDesc("ms5611",     0, 0, SPI_MODE_0, 8, SPI_CS_KERNEL,  1*KHZ, 10*MHZ),
};
#elif CONFIG_HAL_BOARD_SUBTYPE == HAL_BOARD_SUBTYPE_LINUX_BBBMINI
SPIDesc SPIDeviceManager::_device[] = {
    /* MPU9250 is restricted to 1MHz for non-data and interrupt registers */
    SPIDesc("mpu9250",    2, 0, SPI_MODE_3, 8, SPI_CS_KERNEL,  1*MHZ, 20*MHZ),
    SPIDesc("mpu9250ext", 1, 0, SPI_MODE_3, 8, SPI_CS_KERNEL,  1*MHZ, 20*MHZ),
    SPIDesc("ms5611",     2, 1, SPI_MODE_3, 8, SPI_CS_KERNEL,  10*MHZ,10*MHZ),
};
#elif CONFIG_HAL_BOARD_SUBTYPE == HAL_BOARD_SUBTYPE_LINUX_RASPILOT
SPIDesc SPIDeviceManager::_device[] = {
    /* MPU9250 is restricted to 1MHz for non-data and interrupt registers */
    SPIDesc("mpu6000",    0, 0, SPI_MODE_3, 8, RPI_GPIO_25,  1*MHZ, 20*MHZ),
    SPIDesc("ms5611",     0, 0, SPI_MODE_3, 8, RPI_GPIO_23,  10*MHZ, 10*MHZ),
    SPIDesc("lsm9ds0_am", 0, 0, SPI_MODE_3, 8, RPI_GPIO_22,  10*MHZ, 10*MHZ),
    SPIDesc("lsm9ds0_g",  0, 0, SPI_MODE_3, 8, RPI_GPIO_12,  10*MHZ, 10*MHZ),
    SPIDesc("raspio",     0, 0, SPI_MODE_3, 8, RPI_GPIO_7,   10*MHZ, 10*MHZ),
};
#elif CONFIG_HAL_BOARD_SUBTYPE == HAL_BOARD_SUBTYPE_LINUX_BH
SPIDesc SPIDeviceManager::_device[] = {
    SPIDesc("mpu9250", 0, 0, SPI_MODE_0, 8, RPI_GPIO_7, 1*MHZ,   20*MHZ),
    SPIDesc("ublox",   0, 0, SPI_MODE_0, 8, RPI_GPIO_8, 250*KHZ, 5*MHZ),
};
#elif CONFIG_HAL_BOARD_SUBTYPE == HAL_BOARD_SUBTYPE_LINUX_BEBOP || CONFIG_HAL_BOARD_SUBTYPE == HAL_BOARD_SUBTYPE_LINUX_DISCO
SPIDesc SPIDeviceManager::_device[] = {
    SPIDesc("bebop", 1, 0, SPI_MODE_0, 8, SPI_CS_KERNEL,  320*KHZ, 320*KHZ),
};
#else
// empty device table
SPIDesc SPIDeviceManager::_device[] = {
    SPIDesc("**dummy**",    0, 0, SPI_MODE_3, 0, 0,  0 * MHZ, 0 * MHZ),
};
#define LINUX_SPI_DEVICE_NUM_DEVICES 1
#endif

#ifndef LINUX_SPI_DEVICE_NUM_DEVICES
#define LINUX_SPI_DEVICE_NUM_DEVICES ARRAY_SIZE(SPIDeviceManager::_device)
#endif

const uint8_t SPIDeviceManager::_n_device_desc = LINUX_SPI_DEVICE_NUM_DEVICES;


/* Private struct to maintain for each bus */
class SPIBus {
public:
    ~SPIBus();

    int open(uint16_t bus_, uint16_t kernel_cs_);

    Semaphore sem;
    int fd = -1;
    uint16_t bus;
    uint16_t kernel_cs;
    uint8_t ref;
};

SPIBus::~SPIBus()
{
    if (fd >= 0) {
        ::close(fd);
    }
}


int SPIBus::open(uint16_t bus_, uint16_t kernel_cs_)
{
    char path[sizeof("/dev/spidevXXXXX.XXXXX")];

    if (fd > 0) {
        return -EBUSY;
    }

    snprintf(path, sizeof(path), "/dev/spidev%u.%u", bus_, kernel_cs_);
    fd = ::open(path, O_RDWR | O_CLOEXEC);
    if (fd < 0) {
        AP_HAL::panic("SPI: unable to open SPI bus %s: %s",
                      path, strerror(errno));
    }

    bus = bus_;
    kernel_cs = kernel_cs_;

    return fd;
}


SPIDevice::SPIDevice(SPIBus &bus, SPIDesc &device_desc)
    : _bus(bus)
    , _desc(device_desc)
{
    if (_desc.cs_pin != SPI_CS_KERNEL) {
        _cs = hal.gpio->channel(_desc.cs_pin);
        if (!_cs) {
            AP_HAL::panic("Unable to instantiate cs pin");
        }

        _cs->mode(HAL_GPIO_OUTPUT);

        // do not hold the SPI bus initially
        _cs_release();
    }
}

SPIDevice::~SPIDevice()
{
    // Unregister itself from the SPIDeviceManager
    SPIDeviceManager::from(hal.spi)->_unregister(_bus);
}

bool SPIDevice::set_speed(AP_HAL::Device::Speed speed)
{
    switch (speed) {
    case AP_HAL::Device::SPEED_HIGH:
        _speed = _desc.highspeed;
        break;
    case AP_HAL::Device::SPEED_LOW:
        _speed = _desc.lowspeed;
        break;
    }

    return true;
}

bool SPIDevice::transfer(const uint8_t *send, uint32_t send_len,
                         uint8_t *recv, uint32_t recv_len)
{
    struct spi_ioc_transfer msgs[2] = { };
    unsigned nmsgs = 0;

    assert(_bus.fd >= 0);

    if (send && send_len != 0) {
        msgs[nmsgs].tx_buf = (uint64_t) send;
        msgs[nmsgs].rx_buf = 0;
        msgs[nmsgs].len = send_len;
        msgs[nmsgs].speed_hz = _speed;
        msgs[nmsgs].delay_usecs = 0;
        msgs[nmsgs].bits_per_word = _desc.bits_per_word;
        msgs[nmsgs].cs_change = 0;
        nmsgs++;
    }

    if (recv && recv_len != 0) {
        msgs[nmsgs].tx_buf = 0;
        msgs[nmsgs].rx_buf = (uint64_t) recv;
        msgs[nmsgs].len = recv_len;
        msgs[nmsgs].speed_hz = _speed;
        msgs[nmsgs].delay_usecs = 0;
        msgs[nmsgs].bits_per_word = _desc.bits_per_word;
        msgs[nmsgs].cs_change = 0;
        nmsgs++;
    }

    if (!nmsgs) {
        return false;
    }

    int r = ioctl(_bus.fd, SPI_IOC_WR_MODE, &_desc.mode);
    if (r < 0) {
        hal.console->printf("SPIDevice: error on setting mode fd=%d (%s)\n",
                            _bus.fd, strerror(errno));
        return false;
    }

    _cs_assert();
    r = ioctl(_bus.fd, SPI_IOC_MESSAGE(nmsgs), &msgs);
    _cs_release();

    if (r == -1) {
        hal.console->printf("SPIDevice: error transferring data fd=%d (%s)\n",
                            _bus.fd, strerror(errno));
        return false;
    }

    return true;
}

bool SPIDevice::transfer_fullduplex(const uint8_t *send, uint8_t *recv,
                                    uint32_t len)
{
    struct spi_ioc_transfer msgs[1] = { };

    assert(_bus.fd >= 0);

    if (!send || !recv || len == 0) {
        return false;
    }

    msgs[0].tx_buf = (uint64_t) send;
    msgs[0].rx_buf = (uint64_t) recv;
    msgs[0].len = len;
    msgs[0].speed_hz = _speed;
    msgs[0].delay_usecs = 0;
    msgs[0].bits_per_word = _desc.bits_per_word;
    msgs[0].cs_change = 0;

    int r = ioctl(_bus.fd, SPI_IOC_WR_MODE, &_desc.mode);
    if (r < 0) {
        hal.console->printf("SPIDevice: error on setting mode fd=%d (%s)\n",
                            _bus.fd, strerror(errno));
        return false;
    }

    _cs_assert();
    r = ioctl(_bus.fd, SPI_IOC_MESSAGE(1), &msgs);
    _cs_release();

    if (r == -1) {
        hal.console->printf("SPIDevice: error transferring data fd=%d (%s)\n",
                            _bus.fd, strerror(errno));
        return false;
    }

    return true;
}


void SPIDevice::_cs_assert()
{
    if (_desc.cs_pin == SPI_CS_KERNEL) {
        return;
    }

    _cs->write(0);
}

void SPIDevice::_cs_release()
{
    if (_desc.cs_pin == SPI_CS_KERNEL) {
        return;
    }

    _cs->write(1);
}

AP_HAL::Semaphore *SPIDevice::get_semaphore()
{
    return &_bus.sem;
}

int SPIDevice::get_fd()
{
    return _bus.fd;
}

AP_HAL::OwnPtr<AP_HAL::SPIDevice>
SPIDeviceManager::get_device(const char *name)
{
    SPIDesc *desc = nullptr;

    /* Find the bus description in the table */
    for (uint8_t i = 0; i < _n_device_desc; i++) {
        if (!strcmp(_device[i].name, name)) {
            desc = &_device[i];
            break;
        }
    }

    if (!desc) {
        AP_HAL::panic("SPI: invalid device name");
    }

    /* Find if bus is already open */
    for (uint8_t i = 0, n = _buses.size(); i < n; i++) {
        if (_buses[i]->bus == desc->bus &&
            _buses[i]->kernel_cs == desc->subdev) {
            return _create_device(*_buses[i], *desc);
        }
    }

    /* Bus not found for this device, create a new one */
    AP_HAL::OwnPtr<SPIBus> b{new SPIBus()};
    if (!b || b->open(desc->bus, desc->subdev) < 0) {
        return nullptr;
    }

    auto dev = _create_device(*b, *desc);
    if (!dev) {
        return nullptr;
    }

    _buses.push_back(b.leak());

    return dev;
}

/* Create a new device increasing the bus reference */
AP_HAL::OwnPtr<AP_HAL::SPIDevice>
SPIDeviceManager::_create_device(SPIBus &b, SPIDesc &desc) const
{
    auto dev = AP_HAL::OwnPtr<AP_HAL::SPIDevice>(new SPIDevice(b, desc));
    if (!dev) {
        return nullptr;
    }
    b.ref++;
    return dev;
}

void SPIDeviceManager::_unregister(SPIBus &b)
{
    if (b.ref == 0 || --b.ref > 0) {
        return;
    }

    for (auto it = _buses.begin(); it != _buses.end(); it++) {
        if ((*it)->bus == b.bus && (*it)->kernel_cs == b.kernel_cs) {
            _buses.erase(it);
            delete &b;
            break;
        }
    }
}

}
