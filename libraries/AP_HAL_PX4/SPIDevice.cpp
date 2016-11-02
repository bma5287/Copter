/*
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

#include <arch/board/board.h>
#include "board_config.h"
#include <drivers/device/spi.h>
#include <stdio.h>
#include <AP_HAL/AP_HAL.h>
#include <AP_HAL/utility/OwnPtr.h>
#include "Scheduler.h"
#include "Semaphores.h"

extern bool _px4_thread_should_exit;

namespace PX4 {

static const AP_HAL::HAL &hal = AP_HAL::get_HAL();

#define MHZ (1000U*1000U)
#define KHZ (1000U)

SPIDesc SPIDeviceManager::device_table[] = {
    SPIDesc("mpu6000",   PX4_SPI_BUS_SENSORS, (spi_dev_e)PX4_SPIDEV_MPU, SPIDEV_MODE3, 500*KHZ, 11*MHZ),
    SPIDesc("ms5611",    PX4_SPI_BUS_SENSORS, (spi_dev_e)PX4_SPIDEV_BARO, SPIDEV_MODE3, 10*MHZ, 10*MHZ),
    SPIDesc("lsm9ds0_a", PX4_SPI_BUS_SENSORS, (spi_dev_e)PX4_SPIDEV_ACCEL_MAG, SPIDEV_MODE3, 10*MHZ, 10*MHZ),
    SPIDesc("lsm9ds0_g", PX4_SPI_BUS_SENSORS, (spi_dev_e)PX4_SPIDEV_GYRO, SPIDEV_MODE3, 10*MHZ, 10*MHZ),

    SPIDesc(nullptr, 0, (spi_dev_e)0, (spi_mode_e)0, 0, 0),
};

SPIDevice::SPIDevice(SPIBus &_bus, SPIDesc &_device_desc)
    : bus(_bus)
    , device_desc(_device_desc)
    , px4dev(device_desc.bus, device_desc.name, device_desc.devname, device_desc.device, device_desc.mode, device_desc.lowspeed)
{
}

SPIDevice::~SPIDevice()
{
}

bool SPIDevice::set_speed(AP_HAL::Device::Speed speed)
{
    switch (speed) {
    case AP_HAL::Device::SPEED_HIGH:
        px4dev.set_speed(device_desc.highspeed);
        break;
    case AP_HAL::Device::SPEED_LOW:
        px4dev.set_speed(device_desc.lowspeed);
        break;
    }
    return true;
}

bool SPIDevice::transfer(const uint8_t *send, uint32_t send_len,
                         uint8_t *recv, uint32_t recv_len)
{
    uint8_t buf[send_len+recv_len];
    if (send_len > 0) {
        memcpy(buf, send, send_len);
    }
    if (recv_len > 0) {
        memset(&buf[send_len], 0, recv_len);
    }
    bool ret = px4dev.do_transfer(buf, buf, send_len+recv_len);
    if (recv_len > 0 && ret) {
        memcpy(recv, &buf[send_len], recv_len);
    }
    return ret;
}

bool SPIDevice::transfer_fullduplex(const uint8_t *send, uint8_t *recv, uint32_t len)
{
    uint8_t buf[len];
    memcpy(buf, send, len);
    bool ret = px4dev.do_transfer(buf, buf, len);
    if (ret) {
        memcpy(recv, buf, len);
    }
    return ret;
}

AP_HAL::Semaphore *SPIDevice::get_semaphore()
{
    return &bus.semaphore;
}

/*
  per-bus spi callback thread
*/
void *SPIDevice::spi_thread(void *arg)
{
    struct SPIBus *binfo = (struct SPIBus *)arg;
    while (!_px4_thread_should_exit) {
        uint64_t now = AP_HAL::micros64();
        uint64_t next_needed = 0;
        SPIBus::callback_info *callback;

        // find a callback to run
        for (callback = binfo->callbacks; callback; callback = callback->next) {
            if (now >= callback->next_usec) {
                while (now >= callback->next_usec) {
                    callback->next_usec += callback->period_usec;
                }
                // call it with semaphore held
                if (binfo->semaphore.take(0)) {
                    callback->cb();
                    binfo->semaphore.give();
                }
            }
            if (next_needed == 0 ||
                callback->next_usec < next_needed) {
                next_needed = callback->next_usec;
            }
        }

        // delay for at most 50ms, to handle newly added callbacks
        now = AP_HAL::micros64();
        uint32_t delay = 50000;
        if (next_needed > now && next_needed - now < delay) {
            delay = next_needed - now;
        }
        hal.scheduler->delay_microseconds(delay);
    }
    return nullptr;
}
    
AP_HAL::Device::PeriodicHandle SPIDevice::register_periodic_callback(uint32_t period_usec, AP_HAL::Device::PeriodicCb cb)
{
    if (!bus.thread_started) {
        bus.thread_started = true;
    
        pthread_attr_t thread_attr;
        struct sched_param param;
    
        pthread_attr_init(&thread_attr);
        pthread_attr_setstacksize(&thread_attr, 1024);
    
        param.sched_priority = APM_SPI_PRIORITY;
        (void)pthread_attr_setschedparam(&thread_attr, &param);
        pthread_attr_setschedpolicy(&thread_attr, SCHED_FIFO);
    
        pthread_create(&bus.thread_ctx, &thread_attr, &SPIDevice::spi_thread, &bus);
    }
    SPIBus::callback_info *callback = new SPIBus::callback_info;
    if (callback == nullptr) {
        return nullptr;
    }
    callback->cb = cb;
    callback->period_usec = period_usec;
    callback->next_usec = AP_HAL::micros64() + period_usec;

    // add to linked list of callbacks on thread
    callback->next = bus.callbacks;
    bus.callbacks = callback;

    return callback;
}

bool SPIDevice::adjust_periodic_callback(AP_HAL::Device::PeriodicHandle h, uint32_t period_usec)
{
    return false;
}


/*
  return a SPIDevice given a string device name
 */    
AP_HAL::OwnPtr<AP_HAL::SPIDevice>
SPIDeviceManager::get_device(const char *name)
{
    /* Find the bus description in the table */
    uint8_t i;
    for (i = 0; device_table[i].name; i++) {
        if (strcmp(device_table[i].name, name) == 0) {
            break;
        }
    }
    if (device_table[i].name == nullptr) {
        AP_HAL::panic("SPI: invalid device name: %s", name);
    }

    SPIDesc &desc = device_table[i];

    // find the bus
    struct SPIBus *busp;
    for (busp = buses; busp; busp = busp->next) {
        if (busp->bus == desc.bus) {
            break;
        }
    }
    if (busp == nullptr) {
        // create a new one
        busp = new SPIBus;
        if (busp == nullptr) {
            return nullptr;
        }
        busp->next = buses;
        busp->bus = desc.bus;
        buses = busp;
    }

    return AP_HAL::OwnPtr<AP_HAL::SPIDevice>(new SPIDevice(*busp, desc));
}

}
