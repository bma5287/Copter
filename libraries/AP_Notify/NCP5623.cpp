/*
  NCP5623 I2C LED driver
*/
/*
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "NCP5623.h"
#include <utility>

#include <AP_HAL/AP_HAL.h>

extern const AP_HAL::HAL& hal;

#define NCP5623_LED_BRIGHT  0x1f    // full brightness
#define NCP5623_LED_MEDIUM  0x18    // medium brightness
#define NCP5623_LED_DIM     0x0f    // dim
#define NCP5623_LED_OFF     0x00    // off

#define NCP5623_LED_I2C_ADDR 0x38    // default I2C bus address

#define NCP5623_LED_PWM0    0x40    // pwm0 register
#define NCP5623_LED_PWM1    0x60    // pwm1 register
#define NCP5623_LED_PWM2    0x80    // pwm2 register
#define NCP5623_LED_ENABLE  0x20    // enable register

NCP5623::NCP5623(uint8_t bus)
    : RGBLed(NCP5623_LED_OFF, NCP5623_LED_BRIGHT, NCP5623_LED_MEDIUM, NCP5623_LED_DIM)
    , _bus(bus)
{
}

bool NCP5623::write(uint8_t reg, uint8_t data)
{
    uint8_t msg[1] = { 0x00 };
    msg[0] = ((reg & 0xe0) | (data & 0x1f));
    bool ret = _dev->transfer(msg, 1, nullptr, 0);
    return ret;
}

bool NCP5623::writes(uint8_t *data, uint8_t len)
{
    bool ret = 0;
    uint8_t reg = data[0];
    for (uint8_t i = 0; i < (len - 1); i++) {
        ret = write(reg, data[i + 1]);
        if (!ret) {
            return ret;
        }
        reg = reg + 0x20;
    }
    return ret;
}

bool NCP5623::hw_init(void)
{
    // first look for led on external bus
    _dev = std::move(hal.i2c_mgr->get_device(_bus, NCP5623_LED_I2C_ADDR));
    if (!_dev || !_dev->get_semaphore()->take(HAL_SEMAPHORE_BLOCK_FOREVER)) {
        return false;
    }
    _dev->set_retries(10);

    // enable the led
    bool ret = write(NCP5623_LED_ENABLE, 0x1f);
    if (!ret) {
        _dev->get_semaphore()->give();
        return false;
    }
    // update the red, green and blue values to zero
    uint8_t val[4] = { NCP5623_LED_PWM0, _led_off, _led_off, _led_off };
    ret = writes(val, sizeof(val));
    _dev->set_retries(1);

    // give back i2c semaphore
    _dev->get_semaphore()->give();

    if (ret) {
        _dev->register_periodic_callback(20000, FUNCTOR_BIND_MEMBER(&NCP5623::_timer, void));
    }

    return ret;
}

// set_rgb - set color as a combination of red, green and blue values
bool NCP5623::hw_set_rgb(uint8_t red, uint8_t green, uint8_t blue)
{
    rgb = { red, green, blue };
    _need_update = true;
    return true;
}

void NCP5623::_timer(void)
{
    if (!_need_update) {
        return;
    }
    _need_update = false;

    /* 4-bit for each color */
    uint8_t val[4] = { NCP5623_LED_PWM0, rgb.r, rgb.g, rgb.b };

    writes(val, sizeof(val));
}
