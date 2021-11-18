#pragma once

#include <AP_Common/AP_Common.h>
#include <AP_Math/SCurve.h>
#include <APM_Control/AR_AttitudeControl.h>
#include <APM_Control/AR_PosControl.h>
#include <AC_Avoidance/AP_OAPathPlanner.h>
#include "AR_PivotTurn.h"

const float AR_WPNAV_HEADING_UNKNOWN = 99999.0f; // used to indicate to set_desired_location method that next leg's heading is unknown

class AR_WPNav {
public:

    // constructor
    AR_WPNav(AR_AttitudeControl& atc, AR_PosControl &pos_control);

    // initialise waypoint controller
    // speed_max should be the max speed (in m/s) the vehicle will travel to waypoint.  Leave as zero to use the default speed
    // accel_max should be the max forward-back acceleration (in m/s/s).  Leave as zero to use the attitude controller's default acceleration
    // lat_accel_max should be the max right-left acceleration (in m/s/s).  Leave as zero to use the attitude controller's default acceleration
    // jerk_max should be the max forward-back and lateral jerk (in m/s/s/s).  Leave as zero to use the attitude controller's default acceleration
    void init(float speed_max, float accel_max, float lat_accel_max, float jerk_max);

    // update navigation
    void update(float dt);

    // return desired speed
    float get_desired_speed() const { return _desired_speed; }

    // set desired speed in m/s
    void set_desired_speed(float speed) { _desired_speed = MAX(speed, 0.0f); }

    // restore desired speed to default from parameter value
    void set_desired_speed_to_default() { _desired_speed = _speed_max; }

    // execute the mission in reverse (i.e. drive backwards to destination)
    bool get_reversed() const { return _reversed; }
    void set_reversed(bool reversed) { _reversed = reversed; }

    // get navigation outputs for speed (in m/s) and turn rate (in rad/sec)
    float get_speed() const { return _desired_speed_limited; }
    float get_turn_rate_rads() const { return _desired_turn_rate_rads; }

    // get desired lateral acceleration (for reporting purposes only because will be zero during pivot turns)
    float get_lat_accel() const { return _desired_lat_accel; }

    // set desired location and (optionally) next_destination
    // next_destination should be provided if known to allow smooth cornering
    bool set_desired_location(const Location &destination, Location next_destination = Location()) WARN_IF_UNUSED;

    // set desired location to a reasonable stopping point, return true on success
    bool set_desired_location_to_stopping_location()  WARN_IF_UNUSED;

    // set desired location as offset from the EKF origin, return true on success
    bool set_desired_location_NED(const Vector3f& destination) WARN_IF_UNUSED;
    bool set_desired_location_NED(const Vector3f &destination, const Vector3f &next_destination) WARN_IF_UNUSED;

    // true if vehicle has reached desired location. defaults to true because this is normally used by missions and we do not want the mission to become stuck
    bool reached_destination() const { return _reached_destination; }

    // return distance (in meters) to destination
    float get_distance_to_destination() const { return _distance_to_destination; }

    // return true if destination is valid
    bool is_destination_valid() const { return _orig_and_dest_valid; }

    // get current destination. Note: this is not guaranteed to be valid (i.e. _orig_and_dest_valid is not checked)
    const Location &get_destination() { return _destination; }

    // get object avoidance adjusted destination. Note: this is not guaranteed to be valid (i.e. _orig_and_dest_valid is not checked)
    const Location &get_oa_destination() { return _oa_destination; }

    // return heading (in centi-degrees) and cross track error (in meters) for reporting to ground station (NAV_CONTROLLER_OUTPUT message)
    float wp_bearing_cd() const { return _wp_bearing_cd; }
    float nav_bearing_cd() const { return _desired_heading_cd; }
    float crosstrack_error() const { return _cross_track_error; }

    // return the heading (in centi-degrees) to the next waypoint accounting for OA, (used by sailboats)
    float oa_wp_bearing_cd() const { return _oa_wp_bearing_cd; }

    // settor to allow vehicle code to provide turn related param values to this library (should be updated regularly)
    void set_turn_params(float turn_radius, bool pivot_possible);

    // accessors for parameter values
    float get_default_speed() const { return _speed_max; }
    float get_radius() const { return _radius; }
    float get_pivot_rate() const { return _pivot.get_rate_max(); }

    // calculate stopping location using current position and attitude controller provided maximum deceleration
    // returns true on success, false on failure
    bool get_stopping_location(Location& stopping_loc) WARN_IF_UNUSED;

    // parameter var table
    static const struct AP_Param::GroupInfo var_info[];

private:

    // true if update has been called recently
    bool is_active() const;

    // move target location along track from origin to destination
    void advance_wp_target_along_track(const Location &current_loc, float dt);

    // update distance and bearing from vehicle's current position to destination
    void update_distance_and_bearing_to_destination();

    // calculate steering and speed to drive along line from origin to destination waypoint
    void update_steering_and_speed(const Location &current_loc, float dt);

    // adjust speed to ensure it does not fall below value held in SPEED_MIN
    // desired_speed should always be positive (or zero)
    void apply_speed_min(float &desired_speed) const;

    // calculate the crosstrack error (does not rely on L1 controller)
    float calc_crosstrack_error(const Location& current_loc) const;

    // parameters
    AP_Float _speed_max;            // target speed between waypoints in m/s
    AP_Float _speed_min;            // target speed minimum in m/s.  Vehicle will not slow below this speed for corners
    AP_Float _radius;               // distance in meters from a waypoint when we consider the waypoint has been reached
    AR_PivotTurn _pivot;            // pivot turn controller

    // references
    AR_AttitudeControl& _atc;       // rover attitude control library
    AR_PosControl &_pos_control;    // rover position control library

    // scurve
    SCurve _scurve_prev_leg;        // previous scurve trajectory used to blend with current scurve trajectory
    SCurve _scurve_this_leg;        // current scurve trajectory
    SCurve _scurve_next_leg;        // next scurve trajectory used to blend with current scurve trajectory
    float _scurve_jerk;             // scurve jerk max in m/s/s/s
    bool _fast_waypoint;            // true if vehicle will stop at the next waypoint
    float _track_scalar_dt;         // time scaler to ensure scurve target doesn't get too far ahead of vehicle

    // variables held in vehicle code (for now)
    float _turn_radius;             // vehicle turn radius in meters

    // variables for navigation
    uint32_t _last_update_ms;       // system time of last call to update
    Location _origin;               // origin Location (vehicle will travel from the origin to the destination)
    Location _destination;          // destination Location when in Guided_WP
    bool _orig_and_dest_valid;      // true if the origin and destination have been set
    bool _reversed;                 // execute the mission by backing up

    // main outputs from navigation library
    float _desired_speed;           // desired speed in m/s
    float _desired_speed_limited;   // desired speed (above) but accel/decel limited
    float _desired_turn_rate_rads;  // desired turn-rate in rad/sec (negative is counter clockwise, positive is clockwise)
    float _desired_lat_accel;       // desired lateral acceleration (for reporting only)
    float _desired_heading_cd;      // desired heading (back towards line between origin and destination)
    float _wp_bearing_cd;           // heading to waypoint in centi-degrees
    float _cross_track_error;       // cross track error (in meters).  distance from current position to closest point on line between origin and destination

    // variables for reporting
    float _distance_to_destination; // distance from vehicle to final destination in meters
    bool _reached_destination;      // true once the vehicle has reached the destination

    // object avoidance variables
    bool _oa_active;                // true if we should use alternative destination to avoid obstacles
    Location _oa_origin;            // intermediate origin during avoidance
    Location _oa_destination;       // intermediate destination during avoidance
    float _oa_distance_to_destination; // OA (object avoidance) distance from vehicle to _oa_destination in meters
    float _oa_wp_bearing_cd;        // OA adjusted heading to _oa_destination in centi-degrees
};
