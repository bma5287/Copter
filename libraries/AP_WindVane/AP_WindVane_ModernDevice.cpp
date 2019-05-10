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

#include "AP_WindVane_ModernDevice.h"
// read wind speed from Modern Device rev p wind sensor
// https://moderndevice.com/news/calibrating-rev-p-wind-sensor-new-regression/

// constructor
AP_WindVane_ModernDevice::AP_WindVane_ModernDevice(AP_WindVane &frontend) :
    AP_WindVane_Backend(frontend)
{
    _speed_analog_source = hal.analogin->channel(ANALOG_INPUT_NONE);
    _temp_analog_source = hal.analogin->channel(ANALOG_INPUT_NONE);
}

void AP_WindVane_ModernDevice::update_speed()
{
    float analog_voltage = 0.0f;

    // only read temp pin if defined, sensor will do OK assuming constant temp
    float temp_ambient = 28.0f; // equations were generated at this temp in above data sheet
    if (is_positive(_frontend._speed_sensor_temp_pin.get())) {
        _temp_analog_source->set_pin(_frontend._speed_sensor_temp_pin.get());
        analog_voltage = _temp_analog_source->voltage_average();
        temp_ambient = (analog_voltage - 0.4f) / 0.0195f; // deg C
        // constrain to reasonable range to avoid deviating from calibration too much and potential divide by zero
        temp_ambient = constrain_float(temp_ambient, 10.0f, 40.0f);
    }

    _speed_analog_source->set_pin(_frontend._speed_sensor_speed_pin.get());
    _current_analog_voltage = _speed_analog_source->voltage_average();

    // apply voltage offset and make sure not negative
    // by default the voltage offset is the number provide by the manufacturer
    analog_voltage = _current_analog_voltage - _frontend._speed_sensor_voltage_offset;
    if (is_negative(analog_voltage)) {
        analog_voltage = 0.0f;
    }

    // simplified equation from data sheet, converted from mph to m/s
    speed_update_frontend(24.254896f * powf((analog_voltage / powf(temp_ambient, 0.115157f)), 3.009364f));
}
