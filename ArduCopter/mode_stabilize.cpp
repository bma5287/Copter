#include "Copter.h"

/*
 * Init and run calls for stabilize flight mode
 */

// stabilize_run - runs the main stabilize controller
// should be called at 100hz or more
void Copter::ModeStabilize::run()
{
    // apply simple mode transform to pilot inputs
    update_simple_mode();

    // convert pilot input to lean angles
    float target_roll, target_pitch;
    get_pilot_desired_lean_angles(target_roll, target_pitch, copter.aparm.angle_max, copter.aparm.angle_max);

    // get pilot's desired yaw rate
    float target_yaw_rate = get_pilot_desired_yaw_rate(channel_yaw->get_control_in());

    if (!motors->armed()) {
        // Motors should be Stopped
        motors->set_desired_spool_state(AP_Motors::DESIRED_SHUT_DOWN);
    } else if (ap.throttle_zero) {
        // Attempting to Land
        motors->set_desired_spool_state(AP_Motors::DESIRED_GROUND_IDLE);
    } else {
        motors->set_desired_spool_state(AP_Motors::DESIRED_THROTTLE_UNLIMITED);
    }

    if (motors->get_spool_mode() == AP_Motors::SHUT_DOWN) {
        // Motors Stopped
        attitude_control->set_yaw_target_to_current_heading();
        attitude_control->reset_rate_controller_I_terms();
    } else if (motors->get_spool_mode() == AP_Motors::GROUND_IDLE) {
        // Landed
        attitude_control->set_yaw_target_to_current_heading();
        attitude_control->reset_rate_controller_I_terms();
    } else if (motors->get_spool_mode() == AP_Motors::THROTTLE_UNLIMITED) {
        // clear landing flag above zero throttle
        if (!motors->limit.throttle_lower) {
            set_land_complete(false);
        }
    }

    // call attitude controller
    attitude_control->input_euler_angle_roll_pitch_euler_rate_yaw(target_roll, target_pitch, target_yaw_rate);

    // body-frame rate controller is run directly from 100hz loop

    // output pilot's throttle
    attitude_control->set_throttle_out(get_pilot_desired_throttle(),
                                       true,
                                       g.throttle_filt);
}
