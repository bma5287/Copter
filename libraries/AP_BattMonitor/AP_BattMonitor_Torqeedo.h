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
#pragma once

#include <AP_Common/AP_Common.h>
#include <AP_HAL/AP_HAL.h>
#include <AP_Torqeedo/AP_Torqeedo.h>
#include "AP_BattMonitor_Backend.h"

#if HAL_TORQEEDO_ENABLED

class AP_BattMonitor_Torqeedo: public AP_BattMonitor_Backend
{
public:
    // constructor. This incorporates initialisation as well.
    AP_BattMonitor_Torqeedo(AP_BattMonitor &mon, AP_BattMonitor::BattMonitor_State &mon_state, AP_BattMonitor_Params &params):
        AP_BattMonitor_Backend(mon, mon_state, params)
    {};

    // read the latest battery voltage
    void read() override;

    /// returns true if battery monitor instance provides current info
    bool has_current() const override { return have_info; };

    // returns true if battery monitor provides temperature
    bool has_temperature() const override { return have_info; };

    // capacity_remaining_pct - returns true if the battery % is available and writes to the percentage argument
    // returns false if the battery is unhealthy, does not have current monitoring, or the pack_capacity is too small
    bool capacity_remaining_pct(uint8_t &percentage) const override WARN_IF_UNUSED;

private:

    bool have_info;             // true if torqeedo has provided battery info at least once
    bool have_capacity;         // true once torqeedo has provided battery capacity
    uint8_t remaining_pct;      // battery remaining capacity as a percentage
};

#endif
