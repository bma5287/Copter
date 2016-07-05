// -*- tab-width: 4; Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*-
#pragma once

/// @file    AP_L1_Control.h
/// @brief   L1 Control algorithm. This is a instance of an
/// AP_Navigation class

/*
 * Originally written by Brandon Jones 2013
 *
 *  Modified by Paul Riseborough 2013 to provide:
 *  - Explicit control over frequency and damping
 *  - Explicit control over track capture angle
 *  - Ability to use loiter radius smaller than L1 length
 */

#include <AP_Math/AP_Math.h>
#include <AP_AHRS/AP_AHRS.h>
#include <AP_Param/AP_Param.h>
#include <AP_Navigation/AP_Navigation.h>

class AP_L1_Control : public AP_Navigation {
public:
    AP_L1_Control(AP_AHRS &ahrs)
        : _ahrs(ahrs)
    {
        AP_Param::setup_object_defaults(this, var_info);
    }

    /* see AP_Navigation.h for the definitions and units of these
     * functions */
    int32_t nav_roll_cd(void) const;
    float lateral_acceleration(void) const;

    // return the desired track heading angle(centi-degrees)
    int32_t nav_bearing_cd(void) const;

    // return the heading error angle (centi-degrees) +ve to left of track
    int32_t bearing_error_cd(void) const;

    float crosstrack_error(void) const { return _crosstrack_error; }
    float crosstrack_error_integrator(void) const { return _L1_xtrack_i; }

    int32_t target_bearing_cd(void) const;
    float turn_distance(float wp_radius) const;
    float turn_distance(float wp_radius, float turn_angle) const;
    void update_waypoint(const struct Location &prev_WP, const struct Location &next_WP);
    void update_loiter(const struct Location &center_WP, float radius, int8_t loiter_direction);
    void update_heading_hold(int32_t navigation_heading_cd);
    void update_level_flight(void);
    bool reached_loiter_target(void);

    // set the default NAVL1_PERIOD
    void set_default_period(float period) {
        _L1_period.set_default(period);
    }

    void set_data_is_stale(void)  {
        _data_is_stale = true;
    }
    bool data_is_stale(void) const {
        return _data_is_stale;
    }

    // this supports the NAVl1_* user settable parameters
    static const struct AP_Param::GroupInfo var_info[];

private:
    // reference to the AHRS object
    AP_AHRS &_ahrs;

    // lateral acceration in m/s required to fly to the
    // L1 reference point (+ve to right)
    float _latAccDem = 0;

    // L1 tracking distance in meters which is dynamically updated
    float _L1_dist = 0;

    // Status which is true when the vehicle has started circling the WP
    bool _WPcircle = false;

    // bearing angle (radians) to L1 point
    float _nav_bearing = 0;

    // bearing error angle (radians) +ve to left of track
    float _bearing_error = 0;

    // crosstrack error in meters
    float _crosstrack_error = 0;

    // target bearing in centi-degrees from last update
    int32_t _target_bearing_cd = 0;

    // L1 tracking loop period (sec)
    AP_Float _L1_period;
    // L1 tracking loop damping ratio
    AP_Float _L1_damping;

    // previous value of cross-track velocity
    float _last_Nu = 0;

    // prevent indecision in waypoint tracking
    void _prevent_indecision(float &Nu);

    // integral feedback to correct crosstrack error. Used to ensure xtrack converges to zero.
    // For tuning purposes it's helpful to clear the integrator when it changes so a _prev is used
    float _L1_xtrack_i = 0;
    AP_Float _L1_xtrack_i_gain;
    float _L1_xtrack_i_gain_prev = 0;
    uint32_t _last_update_waypoint_us = 0;
    bool _data_is_stale = true;
};
