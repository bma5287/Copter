#include "Copter.h"

// Traditional helicopter variables and functions

#if FRAME_CONFIG == HELI_FRAME

#ifndef HELI_DYNAMIC_FLIGHT_SPEED_MIN
 #define HELI_DYNAMIC_FLIGHT_SPEED_MIN      500     // we are in "dynamic flight" when the speed is over 5m/s for 2 seconds
#endif

// counter to control dynamic flight profile
static int8_t heli_dynamic_flight_counter;

// heli_init - perform any special initialisation required for the tradheli
void Copter::heli_init()
{
    // pre-load stab col values as mode is initialized as Stabilize, but stabilize_init() function is not run on start-up.
    input_manager.set_use_stab_col(true);
    input_manager.set_stab_col_ramp(1.0);
}

// heli_check_dynamic_flight - updates the dynamic_flight flag based on our horizontal velocity
// should be called at 50hz
void Copter::check_dynamic_flight(void)
{
    if (motors->get_spool_state() != AP_Motors::SpoolState::THROTTLE_UNLIMITED ||
        flightmode->is_landing()) {
        heli_dynamic_flight_counter = 0;
        heli_flags.dynamic_flight = false;
        return;
    }

    bool moving = false;

    // with GPS lock use inertial nav to determine if we are moving
    if (position_ok()) {
        // get horizontal speed
        const float speed = inertial_nav.get_speed_xy();
        moving = (speed >= HELI_DYNAMIC_FLIGHT_SPEED_MIN);
    }else{
        // with no GPS lock base it on throttle and forward lean angle
        moving = (motors->get_throttle() > 0.8f || ahrs.pitch_sensor < -1500);
    }

    if (!moving && rangefinder_state.enabled && rangefinder.status_orient(ROTATION_PITCH_270) == RangeFinder::RangeFinder_Good) {
        // when we are more than 2m from the ground with good
        // rangefinder lock consider it to be dynamic flight
        moving = (rangefinder.distance_cm_orient(ROTATION_PITCH_270) > 200);
    }
    
    if (moving) {
        // if moving for 2 seconds, set the dynamic flight flag
        if (!heli_flags.dynamic_flight) {
            heli_dynamic_flight_counter++;
            if (heli_dynamic_flight_counter >= 100) {
                heli_flags.dynamic_flight = true;
                heli_dynamic_flight_counter = 100;
            }
        }
    }else{
        // if not moving for 2 seconds, clear the dynamic flight flag
        if (heli_flags.dynamic_flight) {
            if (heli_dynamic_flight_counter > 0) {
                heli_dynamic_flight_counter--;
            }else{
                heli_flags.dynamic_flight = false;
            }
        }
    }
}

// update_heli_control_dynamics - pushes several important factors up into AP_MotorsHeli.
// should be run between the rate controller and the servo updates.
void Copter::update_heli_control_dynamics(void)
{
    // Use Leaky_I if we are not moving fast
    attitude_control->use_leaky_i(!heli_flags.dynamic_flight);

    if (ap.land_complete || (is_zero(motors->get_desired_rotor_speed()))){
        // if we are landed or there is no rotor power demanded, decrement slew scalar
        hover_roll_trim_scalar_slew--;        
    } else {
        // if we are not landed and motor power is demanded, increment slew scalar
        hover_roll_trim_scalar_slew++;
    }
    hover_roll_trim_scalar_slew = constrain_int16(hover_roll_trim_scalar_slew, 0, scheduler.get_loop_rate_hz());

    // set hover roll trim scalar, will ramp from 0 to 1 over 1 second after we think helicopter has taken off
    attitude_control->set_hover_roll_trim_scalar((float) hover_roll_trim_scalar_slew/(float) scheduler.get_loop_rate_hz());
}

bool Copter::should_use_landing_swash() const
{
    if (flightmode->has_manual_throttle() ||
        control_mode == Mode::Number::DRIFT ||
        control_mode == Mode::Number::SPORT) {
        // manual modes always uses full swash range
        return false;
    }
    if (flightmode->is_landing()) {
        // landing with non-manual throttle mode always uses limit swash range
        return true;
    }
    if (ap.land_complete) {
        // when landed in non-manual throttle mode limit swash range
        return true;
    }
    if (!ap.auto_armed) {
        // when waiting to takeoff in non-manual throttle mode limit swash range
        return true;
    }
    if (!heli_flags.dynamic_flight) {
        // Just in case we are unsure of being in non-manual throttle
        // mode, limit swash range in low speed and hovering flight.
        // This will catch any non-manual throttle mode attempting a
        // landing and driving the collective too low before the land
        // complete flag is set.
        return true;
    }
    return false;
}

// heli_update_landing_swash - sets swash plate flag so higher minimum is used when landed or landing
// should be called soon after update_land_detector in main code
void Copter::heli_update_landing_swash()
{
    motors->set_collective_for_landing(should_use_landing_swash());
}

// heli_update_rotor_speed_targets - reads pilot input and passes new rotor speed targets to heli motors object
void Copter::heli_update_rotor_speed_targets()
{

    static bool rotor_runup_complete_last = false;

    // get rotor control method
    uint8_t rsc_control_mode = motors->get_rsc_mode();
    float rsc_control_deglitched = 0.0f;
    RC_Channel *rc_ptr = rc().find_channel_for_option(RC_Channel::AUX_FUNC::MOTOR_INTERLOCK);
    if (rc_ptr != nullptr) {
        rsc_control_deglitched = rotor_speed_deglitch_filter.apply((float)rc_ptr->get_control_in()) * 0.001f;
    }
    switch (rsc_control_mode) {
        case ROTOR_CONTROL_MODE_SPEED_PASSTHROUGH:
            // pass through pilot desired rotor speed from the RC
            if (motors->get_interlock()) {
                motors->set_desired_rotor_speed(rsc_control_deglitched);
            } else {
                motors->set_desired_rotor_speed(0.0f);
            }
            break;
        case ROTOR_CONTROL_MODE_SPEED_SETPOINT:
        case ROTOR_CONTROL_MODE_OPEN_LOOP_POWER_OUTPUT:
        case ROTOR_CONTROL_MODE_CLOSED_LOOP_POWER_OUTPUT:
            // pass setpoint through as desired rotor speed. Needs work, this is pointless as it is
            // not used by closed loop control. Being used as a catch-all for other modes regardless
            // of whether or not they actually use it
            // set rpm from rotor speed sensor
            if (motors->get_interlock()) {
#if RPM_ENABLED == ENABLED
                motors->set_rpm(rpm_sensor.get_rpm(0));
#endif
                motors->set_desired_rotor_speed(motors->get_rsc_setpoint());
            }else{
                motors->set_desired_rotor_speed(0.0f);
            }
            break;
    }

    // when rotor_runup_complete changes to true, log event
    if (!rotor_runup_complete_last && motors->rotor_runup_complete()){
        AP::logger().Write_Event(LogEvent::ROTOR_RUNUP_COMPLETE);
    } else if (rotor_runup_complete_last && !motors->rotor_runup_complete()){
        AP::logger().Write_Event(LogEvent::ROTOR_SPEED_BELOW_CRITICAL);
    }
    rotor_runup_complete_last = motors->rotor_runup_complete();
}

#endif  // FRAME_CONFIG == HELI_FRAME
