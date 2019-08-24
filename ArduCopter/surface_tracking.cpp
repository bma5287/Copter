#include "Copter.h"

// adjust_climb_rate - hold copter at the desired distance above the
//      ground; returns climb rate (in cm/s) which should be passed to
//      the position controller
float Copter::SurfaceTracking::adjust_climb_rate(float target_rate)
{
#if RANGEFINDER_ENABLED == ENABLED
    // check tracking state and that range finders are healthy
    if ((tracking_state == SurfaceTrackingState::SURFACE_TRACKING_DISABLED) ||
        ((tracking_state == SurfaceTrackingState::SURFACE_TRACKING_GROUND) && (!copter.rangefinder_alt_ok() || (copter.rangefinder_state.glitch_count != 0))) ||
        ((tracking_state == SurfaceTrackingState::SURFACE_TRACKING_CEILING) && !copter.rangefinder_up_ok()) || (copter.rangefinder_up_state.glitch_count != 0)) {
        return target_rate;
    }

    // calculate current ekf based altitude error
    const float current_alt_error = copter.pos_control->get_alt_target() - copter.inertial_nav.get_altitude();

    // init based on tracking direction/state
    RangeFinderState &rf_state = (tracking_state == SurfaceTrackingState::SURFACE_TRACKING_GROUND) ? copter.rangefinder_state : copter.rangefinder_up_state;
    const float dir = (tracking_state == SurfaceTrackingState::SURFACE_TRACKING_GROUND) ? 1.0f : -1.0f;

    // reset target altitude if this controller has just been engaged
    // target has been changed between upwards vs downwards
    // or glitch has cleared
    const uint32_t now = millis();
    if ((now - last_update_ms > SURFACE_TRACKING_TIMEOUT_MS) ||
        reset_target ||
        (last_glitch_cleared_ms != rf_state.glitch_cleared_ms)) {
        target_dist_cm = rf_state.alt_cm + (dir * current_alt_error);
        reset_target = false;
        last_glitch_cleared_ms = rf_state.glitch_cleared_ms;\
    }
    last_update_ms = now;

    // adjust rangefinder target alt if motors have not hit their limits
    if ((target_rate<0 && !copter.motors->limit.throttle_lower) || (target_rate>0 && !copter.motors->limit.throttle_upper)) {
        target_dist_cm += dir * target_rate * copter.G_Dt;
    }
    valid_for_logging = true;

    // calc desired velocity correction from target rangefinder alt vs actual rangefinder alt (remove the error already passed to Altitude controller to avoid oscillations)
    const float distance_error = (target_dist_cm - rf_state.alt_cm) - (dir * current_alt_error);
    float velocity_correction = dir * distance_error * copter.g.rangefinder_gain;
    velocity_correction = constrain_float(velocity_correction, -SURFACE_TRACKING_VELZ_MAX, SURFACE_TRACKING_VELZ_MAX);

    // return combined pilot climb rate + rate to correct rangefinder alt error
    return (target_rate + velocity_correction);
#else
    return target_rate;
#endif
}

// get target altitude (in cm) above ground
// returns true if there is a valid target
bool Copter::SurfaceTracking::get_target_alt_cm(float &_target_alt_cm) const
{
    // fail if we are not tracking downwards
    if (tracking_state != SurfaceTrackingState::SURFACE_TRACKING_GROUND) {
        return false;
    }
    // check target has been updated recently
    if (AP_HAL::millis() - last_update_ms > SURFACE_TRACKING_TIMEOUT_MS) {
        return false;
    }
    _target_alt_cm = target_dist_cm;
    return true;
}

// set target altitude (in cm) above ground
void Copter::SurfaceTracking::set_target_alt_cm(float _target_alt_cm)
{
    // fail if we are not tracking downwards
    if (tracking_state != SurfaceTrackingState::SURFACE_TRACKING_GROUND) {
        return;
    }
    target_dist_cm = _target_alt_cm;
    last_update_ms = AP_HAL::millis();
}

bool Copter::SurfaceTracking::get_dist_for_logging(float &target_dist, float &actual_dist) const
{
    if (!valid_for_logging || (tracking_state == SurfaceTrackingState::SURFACE_TRACKING_DISABLED)) {
        return false;
    }
    target_dist = target_dist_cm * 0.01f;
    actual_dist = ((tracking_state == SurfaceTrackingState::SURFACE_TRACKING_GROUND) ? copter.rangefinder_state.alt_cm : copter.rangefinder_up_state.alt_cm) * 0.01f;
    return true;
}

// set direction
void Copter::SurfaceTracking::set_state(SurfaceTrackingState state)
{
    if (tracking_state == state) {
        return;
    }
    // check we have a range finder in the correct direction
    if ((state == SurfaceTrackingState::SURFACE_TRACKING_GROUND) && !copter.rangefinder.has_orientation(ROTATION_PITCH_270)) {
        copter.gcs().send_text(MAV_SEVERITY_WARNING, "SurfaceTracking: no downward rangefinder");
        AP_Notify::events.user_mode_change_failed = 1;
        return;
    }
    if ((state == SurfaceTrackingState::SURFACE_TRACKING_CEILING) && !copter.rangefinder.has_orientation(ROTATION_PITCH_90)) {
        copter.gcs().send_text(MAV_SEVERITY_WARNING, "SurfaceTracking: no upward rangefinder");
        AP_Notify::events.user_mode_change_failed = 1;
        return;
    }
    tracking_state = state;
    reset_target = true;
}
