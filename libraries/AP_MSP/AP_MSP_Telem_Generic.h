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

   MSP generic telemetry library backend
*/
#pragma once

#include "AP_MSP_Telem_Backend.h"

#if HAL_MSP_ENABLED

class AP_MSP_Telem_Generic : public AP_MSP_Telem_Backend
{
    using AP_MSP_Telem_Backend::AP_MSP_Telem_Backend;
public:
    bool is_scheduler_enabled() const override { return false; }
    AP_SerialManager::SerialProtocol get_serial_protocol() const override { return AP_SerialManager::SerialProtocol::SerialProtocol_MSP; };
};

#endif //HAL_MSP_ENABLED
