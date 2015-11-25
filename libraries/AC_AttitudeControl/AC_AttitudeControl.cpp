// -*- tab-width: 4; Mode: C++; c-basic-offset: 4; indent-tabs-mode: t -*-

#include "AC_AttitudeControl.h"
#include <AP_HAL/AP_HAL.h>
#include <AP_Math/AP_Math.h>

// table of user settable parameters
const AP_Param::GroupInfo AC_AttitudeControl::var_info[] = {

    // 0, 1 were RATE_RP_MAX, RATE_Y_MAX

    // BUG: SLEW_YAW's behavior does not match its parameter documentation
    // @Param: SLEW_YAW
    // @DisplayName: Yaw target slew rate
    // @Description: Maximum rate the yaw target can be updated in Loiter, RTL, Auto flight modes
    // @Units: Centi-Degrees/Sec
    // @Range: 500 18000
    // @Increment: 100
    // @User: Advanced
    AP_GROUPINFO("SLEW_YAW",    2, AC_AttitudeControl, _slew_yaw, AC_ATTITUDE_CONTROL_SLEW_YAW_DEFAULT_CDS),

    // 3 was for ACCEL_RP_MAX

    // @Param: ACCEL_Y_MAX
    // @DisplayName: Acceleration Max for Yaw
    // @Description: Maximum acceleration in yaw axis
    // @Units: Centi-Degrees/Sec/Sec
    // @Range: 0 72000
    // @Values: 0:Disabled, 18000:Slow, 36000:Medium, 54000:Fast
    // @Increment: 1000
    // @User: Advanced
    AP_GROUPINFO("ACCEL_Y_MAX",  4, AC_AttitudeControl, _accel_yaw_max, AC_ATTITUDE_CONTROL_ACCEL_Y_MAX_DEFAULT_CDSS),

    // @Param: RATE_FF_ENAB
    // @DisplayName: Rate Feedforward Enable
    // @Description: Controls whether body-frame rate feedfoward is enabled or disabled
    // @Values: 0:Disabled, 1:Enabled
    // @User: Advanced
    AP_GROUPINFO("RATE_FF_ENAB", 5, AC_AttitudeControl, _rate_bf_ff_enabled, AC_ATTITUDE_CONTROL_RATE_BF_FF_DEFAULT),

    // @Param: ACCEL_R_MAX
    // @DisplayName: Acceleration Max for Roll
    // @Description: Maximum acceleration in roll axis
    // @Units: Centi-Degrees/Sec/Sec
    // @Range: 0 180000
    // @Increment: 1000
    // @Values: 0:Disabled, 72000:Slow, 108000:Medium, 162000:Fast
    // @User: Advanced
    AP_GROUPINFO("ACCEL_R_MAX", 6, AC_AttitudeControl, _accel_roll_max, AC_ATTITUDE_CONTROL_ACCEL_RP_MAX_DEFAULT_CDSS),

    // @Param: ACCEL_P_MAX
    // @DisplayName: Acceleration Max for Pitch
    // @Description: Maximum acceleration in pitch axis
    // @Units: Centi-Degrees/Sec/Sec
    // @Range: 0 180000
    // @Increment: 1000
    // @Values: 0:Disabled, 72000:Slow, 108000:Medium, 162000:Fast
    // @User: Advanced
    AP_GROUPINFO("ACCEL_P_MAX", 7, AC_AttitudeControl, _accel_pitch_max, AC_ATTITUDE_CONTROL_ACCEL_RP_MAX_DEFAULT_CDSS),

    // IDs 8,9,10,11 RESERVED (in use on Solo)

    AP_GROUPEND
};

void AC_AttitudeControl::set_dt(float delta_sec)
{
    _dt = delta_sec;
    _pid_rate_roll.set_dt(_dt);
    _pid_rate_pitch.set_dt(_dt);
    _pid_rate_yaw.set_dt(_dt);
}

void AC_AttitudeControl::relax_bf_rate_controller()
{
    // Set reference angular velocity used in angular velocity controller equal
    // to the input angular velocity and reset the angular velocity integrators.
    // This zeros the output of the angular velocity controller.
    _ang_vel_target_rads = _ahrs.get_gyro();
    _pid_rate_roll.reset_I();
    _pid_rate_pitch.reset_I();
    _pid_rate_yaw.reset_I();

    // Write euler derivatives derived from vehicle angular velocity to
    // _att_target_euler_deriv_rads. This resets the state of the input shapers.
    ang_vel_to_euler_derivative(Vector3f(_ahrs.roll,_ahrs.pitch,_ahrs.yaw), _ang_vel_target_rads, _att_target_euler_deriv_rads);
}

void AC_AttitudeControl::shift_ef_yaw_target(float yaw_shift_cd)
{
    _att_target_euler_rad.z = wrap_2PI(_att_target_euler_rad.z + radians(yaw_shift_cd*0.01f));
}

void AC_AttitudeControl::euler_angle_roll_pitch_euler_rate_yaw_smooth(float euler_roll_angle_cd, float euler_pitch_angle_cd, float euler_yaw_rate_cds, float smoothing_gain)
{
    // Convert from centidegrees on public interface to radians
    float euler_roll_angle_rad = radians(euler_roll_angle_cd*0.01f);
    float euler_pitch_angle_rad = radians(euler_pitch_angle_cd*0.01f);
    float euler_yaw_rate_rads = radians(euler_yaw_rate_cds*0.01f);

    // Sanity check smoothing gain
    smoothing_gain = constrain_float(smoothing_gain,1.0f,50.0f);

    // Add roll trim to compensate tail rotor thrust in heli (will return zero on multirotors)
    euler_roll_angle_rad += get_roll_trim_rad();

    Vector3f att_error_euler_rad;

    if ((get_accel_roll_max_radss() > 0.0f) && _rate_bf_ff_enabled) {
        // When roll acceleration limiting and feedforward are enabled, the sqrt controller is used to compute an euler roll-axis
        // angular velocity that will cause the euler roll angle to smoothly stop at the input angle with limited deceleration
        // and an exponential decay specified by smoothing_gain at the end.
        float rate_change_limit_rads = get_accel_roll_max_radss() * _dt;
        float euler_rate_desired_rads = sqrt_controller(euler_roll_angle_rad-_att_target_euler_rad.x, smoothing_gain, get_accel_roll_max_radss());

        // Acceleration is limited directly to smooth the beginning of the curve.
        _att_target_euler_deriv_rads.x = constrain_float(euler_rate_desired_rads, _att_target_euler_deriv_rads.x-rate_change_limit_rads, _att_target_euler_deriv_rads.x+rate_change_limit_rads);

        // The output rate is used to update the attitude target euler angles and is fed forward into the rate controller.
        update_att_target_and_error_roll(_att_target_euler_deriv_rads.x, att_error_euler_rad, AC_ATTITUDE_RATE_STAB_ROLL_OVERSHOOT_ANGLE_MAX_RAD);
    } else {
        // When acceleration limiting and feedforward are not enabled, the target roll euler angle is simply set to the
        // input value and the feedforward rate is zeroed.
        _att_target_euler_rad.x = euler_roll_angle_rad;
        att_error_euler_rad.x = wrap_180_cd_float(_att_target_euler_rad.x - _ahrs.roll);
        _att_target_euler_deriv_rads.x = 0;
    }
    _att_target_euler_rad.x = constrain_float(_att_target_euler_rad.x, -get_tilt_limit_rad(), get_tilt_limit_rad());

    if ((get_accel_pitch_max_radss() > 0.0f) && _rate_bf_ff_enabled) {
        // When pitch acceleration limiting and feedforward are enabled, the sqrt controller is used to compute an euler pitch-axis
        // angular velocity that will cause the euler pitch angle to smoothly stop at the input angle with limited deceleration
        // and an exponential decay specified by smoothing_gain at the end.
        float rate_change_limit_rads = get_accel_pitch_max_radss() * _dt;
        float euler_rate_desired_rads = sqrt_controller(euler_pitch_angle_rad-_att_target_euler_rad.y, smoothing_gain, get_accel_pitch_max_radss());

        // Acceleration is limited directly to smooth the beginning of the curve.
        _att_target_euler_deriv_rads.y = constrain_float(euler_rate_desired_rads, _att_target_euler_deriv_rads.y-rate_change_limit_rads, _att_target_euler_deriv_rads.y+rate_change_limit_rads);

        // The output rate is used to update the attitude target euler angles and is fed forward into the rate controller.
        update_att_target_and_error_pitch(_att_target_euler_deriv_rads.y, att_error_euler_rad, AC_ATTITUDE_RATE_STAB_ROLL_OVERSHOOT_ANGLE_MAX_RAD);
    } else {
        _att_target_euler_rad.y = euler_pitch_angle_rad;
        att_error_euler_rad.y = wrap_180_cd_float(_att_target_euler_rad.y - _ahrs.pitch);
        _att_target_euler_deriv_rads.y = 0;
    }
    _att_target_euler_rad.y = constrain_float(_att_target_euler_rad.y, -get_tilt_limit_rad(), get_tilt_limit_rad());

    if (get_accel_yaw_max_radss() > 0.0f) {
        // When yaw acceleration limiting is enabled, the yaw input shaper constrains angular acceleration about the yaw axis, slewing
        // the output rate towards the input rate.
        float rate_change_limit_rads = get_accel_yaw_max_radss() * _dt;
        _att_target_euler_deriv_rads.z += constrain_float(euler_yaw_rate_rads - _att_target_euler_deriv_rads.z, -rate_change_limit_rads, rate_change_limit_rads);

        // The output rate is used to update the attitude target euler angles and is fed forward into the rate controller.
        update_att_target_and_error_yaw(_att_target_euler_deriv_rads.z, att_error_euler_rad, AC_ATTITUDE_RATE_STAB_YAW_OVERSHOOT_ANGLE_MAX_RAD);
    } else {
        // When yaw acceleration limiting is disabled, the attitude target is simply rotated using the input rate and the input rate
        // is fed forward into the rate controller.
        _att_target_euler_deriv_rads.z = euler_yaw_rate_rads;
        update_att_target_and_error_yaw(_att_target_euler_deriv_rads.z, att_error_euler_rad, AC_ATTITUDE_RATE_STAB_YAW_OVERSHOOT_ANGLE_MAX_RAD);
    }

    // Convert 321-intrinsic euler angle errors to a body-frame rotation vector
    // NOTE: This results in an approximation of the attitude error based on a linearization about the current attitude
    euler_derivative_to_ang_vel(Vector3f(_ahrs.roll,_ahrs.pitch,_ahrs.yaw), att_error_euler_rad, _att_error_rot_vec_rad);

    // Compute the angular velocity target from the attitude error
    update_ang_vel_target_from_att_error();

    // Convert euler angle derivative of desired attitude into a body-frame angular velocity vector for feedforward
    if (_rate_bf_ff_enabled) {
        euler_derivative_to_ang_vel(Vector3f(_ahrs.roll,_ahrs.pitch,_ahrs.yaw), _att_target_euler_deriv_rads, _att_target_ang_vel_rads);
    } else {
        euler_derivative_to_ang_vel(Vector3f(_ahrs.roll,_ahrs.pitch,_ahrs.yaw), Vector3f(0,0,_att_target_euler_deriv_rads.z), _att_target_ang_vel_rads);
    }
    // NOTE: Rotation of _att_target_ang_vel_rads from desired body frame to estimated body frame is possibly omitted here

    // Add the angular velocity feedforward
    _ang_vel_target_rads += _att_target_ang_vel_rads;
}

void AC_AttitudeControl::euler_angle_roll_pitch_euler_rate_yaw(float euler_roll_angle_cd, float euler_pitch_angle_cd, float euler_yaw_rate_cds)
{
    // Convert from centidegrees on public interface to radians
    float euler_roll_angle_rad = radians(euler_roll_angle_cd*0.01f);
    float euler_pitch_angle_rad = radians(euler_pitch_angle_cd*0.01f);
    float euler_yaw_rate_rads = radians(euler_yaw_rate_cds*0.01f);

    Vector3f    att_error_euler_rad;

    // Add roll trim to compensate tail rotor thrust in heli (will return zero on multirotors)
    euler_roll_angle_rad += get_roll_trim_rad();

    // Set roll/pitch attitude targets from input.
    _att_target_euler_rad.x = constrain_float(euler_roll_angle_rad, -get_tilt_limit_rad(), get_tilt_limit_rad());
    _att_target_euler_rad.y = constrain_float(euler_pitch_angle_rad, -get_tilt_limit_rad(), get_tilt_limit_rad());

    // Update roll/pitch attitude error.
    att_error_euler_rad.x = wrap_PI(_att_target_euler_rad.x - _ahrs.roll);
    att_error_euler_rad.y = wrap_PI(_att_target_euler_rad.y - _ahrs.pitch);

    // Zero the roll and pitch feed-forward rate.
    _att_target_euler_deriv_rads.x = 0;
    _att_target_euler_deriv_rads.y = 0;

    if (get_accel_yaw_max_radss() > 0.0f) {
        // When yaw acceleration limiting is enabled, the yaw input shaper constrains angular acceleration about the yaw axis, slewing
        // the output rate towards the input rate.
        float rate_change_limit_rads = get_accel_yaw_max_radss() * _dt;
        _att_target_euler_deriv_rads.z += constrain_float(euler_yaw_rate_rads - _att_target_euler_deriv_rads.z, -rate_change_limit_rads, rate_change_limit_rads);

        // The output rate is used to update the attitude target euler angles and is fed forward into the rate controller.
        update_att_target_and_error_yaw(_att_target_euler_deriv_rads.z, att_error_euler_rad, AC_ATTITUDE_RATE_STAB_YAW_OVERSHOOT_ANGLE_MAX_RAD);
    } else {
        // When yaw acceleration limiting is disabled, the attitude target is simply rotated using the input rate and the input rate
        // is fed forward into the rate controller.
        _att_target_euler_deriv_rads.z = euler_yaw_rate_rads;
        update_att_target_and_error_yaw(_att_target_euler_deriv_rads.z, att_error_euler_rad, AC_ATTITUDE_RATE_STAB_YAW_OVERSHOOT_ANGLE_MAX_RAD);
    }

    // Convert 321-intrinsic euler angle errors to a body-frame rotation vector
    // NOTE: This results in an approximation of the attitude error based on a linearization about the current attitude
    euler_derivative_to_ang_vel(Vector3f(_ahrs.roll,_ahrs.pitch,_ahrs.yaw), att_error_euler_rad, _att_error_rot_vec_rad);

    // Compute the angular velocity target from the attitude error
    update_ang_vel_target_from_att_error();

    // Convert euler angle derivatives of desired attitude into a body-frame angular velocity vector for feedforward
    // NOTE: This should be done about the desired attitude instead of about the vehicle attitude
    euler_derivative_to_ang_vel(Vector3f(_ahrs.roll,_ahrs.pitch,_ahrs.yaw), _att_target_euler_deriv_rads, _att_target_ang_vel_rads);
    // NOTE: A rotation of _att_target_ang_vel_rads from desired body frame to estimated body frame is possibly omitted here

    // Add the angular velocity feedforward
    _ang_vel_target_rads += _att_target_ang_vel_rads;
}

void AC_AttitudeControl::euler_angle_roll_pitch_yaw(float euler_roll_angle_cd, float euler_pitch_angle_cd, float euler_yaw_angle_cd, bool slew_yaw)
{
    // Convert from centidegrees on public interface to radians
    float euler_roll_angle_rad = radians(euler_roll_angle_cd*0.01f);
    float euler_pitch_angle_rad = radians(euler_pitch_angle_cd*0.01f);
    float yaw_angle_ef_rad = radians(euler_yaw_angle_cd*0.01f);

    Vector3f    att_error_euler_rad;

    // Add roll trim to compensate tail rotor thrust in heli (will return zero on multirotors)
    euler_roll_angle_rad += get_roll_trim_rad();

    // Set attitude targets from input.
    _att_target_euler_rad.x = constrain_float(euler_roll_angle_rad, -get_tilt_limit_rad(), get_tilt_limit_rad());
    _att_target_euler_rad.y = constrain_float(euler_pitch_angle_rad, -get_tilt_limit_rad(), get_tilt_limit_rad());
    _att_target_euler_rad.z = yaw_angle_ef_rad;

    // Update attitude error.
    att_error_euler_rad.x = wrap_PI(_att_target_euler_rad.x - _ahrs.roll);
    att_error_euler_rad.y = wrap_PI(_att_target_euler_rad.y - _ahrs.pitch);
    att_error_euler_rad.z = wrap_PI(_att_target_euler_rad.z - _ahrs.yaw);

    // Constrain the yaw angle error
    // BUG: SLEW_YAW's behavior does not match its parameter documentation
    if (slew_yaw) {
        att_error_euler_rad.z = constrain_float(att_error_euler_rad.z,-get_slew_yaw_rads(),get_slew_yaw_rads());
    }

    // Convert 321-intrinsic euler angle errors to a body-frame rotation vector
    // NOTE: This results in an approximation of the attitude error based on a linearization about the current attitude
    euler_derivative_to_ang_vel(Vector3f(_ahrs.roll,_ahrs.pitch,_ahrs.yaw), att_error_euler_rad, _att_error_rot_vec_rad);

    // Compute the angular velocity target from the attitude error
    update_ang_vel_target_from_att_error();

    // Keep euler derivative updated
    ang_vel_to_euler_derivative(Vector3f(_ahrs.roll,_ahrs.pitch,_ahrs.yaw), _ang_vel_target_rads, _att_target_euler_deriv_rads);
}

void AC_AttitudeControl::euler_rate_roll_pitch_yaw(float euler_roll_rate_cds, float euler_pitch_rate_cds, float euler_yaw_rate_cds)
{
    // Convert from centidegrees on public interface to radians
    float euler_roll_rate_rads = radians(euler_roll_rate_cds*0.01f);
    float euler_pitch_rate_rads = radians(euler_pitch_rate_cds*0.01f);
    float euler_yaw_rate_rads = radians(euler_yaw_rate_cds*0.01f);

    Vector3f att_error_euler_rad;

    // Compute acceleration-limited euler roll rate
    if (get_accel_roll_max_radss() > 0.0f) {
        float rate_change_limit_rads = get_accel_roll_max_radss() * _dt;
        _att_target_euler_deriv_rads.x += constrain_float(euler_roll_rate_rads - _att_target_euler_deriv_rads.x, -rate_change_limit_rads, rate_change_limit_rads);
    } else {
        _att_target_euler_deriv_rads.x = euler_roll_rate_rads;
    }

    // Compute acceleration-limited euler pitch rate
    if (get_accel_pitch_max_radss() > 0.0f) {
        float rate_change_limit_rads = get_accel_pitch_max_radss() * _dt;
        _att_target_euler_deriv_rads.y += constrain_float(euler_pitch_rate_rads - _att_target_euler_deriv_rads.y, -rate_change_limit_rads, rate_change_limit_rads);
    } else {
        _att_target_euler_deriv_rads.y = euler_pitch_rate_rads;
    }

    // Compute acceleration-limited euler yaw rate
    if (get_accel_yaw_max_radss() > 0.0f) {
        float rate_change_limit_rads = get_accel_yaw_max_radss() * _dt;
        _att_target_euler_deriv_rads.z += constrain_float(euler_yaw_rate_rads - _att_target_euler_deriv_rads.z, -rate_change_limit_rads, rate_change_limit_rads);
    } else {
        _att_target_euler_deriv_rads.z = euler_yaw_rate_rads;
    }

    // Update the attitude target from the computed euler rates
    update_att_target_and_error_roll(_att_target_euler_deriv_rads.x, att_error_euler_rad, AC_ATTITUDE_RATE_STAB_ROLL_OVERSHOOT_ANGLE_MAX_RAD);
    update_att_target_and_error_pitch(_att_target_euler_deriv_rads.y, att_error_euler_rad, AC_ATTITUDE_RATE_STAB_PITCH_OVERSHOOT_ANGLE_MAX_RAD);
    update_att_target_and_error_yaw(_att_target_euler_deriv_rads.z, att_error_euler_rad, AC_ATTITUDE_RATE_STAB_YAW_OVERSHOOT_ANGLE_MAX_RAD);

    // Apply tilt limit
    _att_target_euler_rad.x = constrain_float(_att_target_euler_rad.x, -get_tilt_limit_rad(), get_tilt_limit_rad());
    _att_target_euler_rad.y = constrain_float(_att_target_euler_rad.y, -get_tilt_limit_rad(), get_tilt_limit_rad());

    // Convert 321-intrinsic euler angle errors to a body-frame rotation vector
    // NOTE: This results in an approximation of the attitude error based on a linearization about the current attitude
    euler_derivative_to_ang_vel(Vector3f(_ahrs.roll,_ahrs.pitch,_ahrs.yaw), att_error_euler_rad, _att_error_rot_vec_rad);

    // Compute the angular velocity target from the attitude error
    update_ang_vel_target_from_att_error();

    // Convert euler angle derivatives of desired attitude into a body-frame angular velocity vector for feedforward
    euler_derivative_to_ang_vel(Vector3f(_ahrs.roll,_ahrs.pitch,_ahrs.yaw), _att_target_euler_deriv_rads, _att_target_ang_vel_rads);
    // NOTE: Rotation of _att_target_ang_vel_rads from desired body frame to estimated body frame is possibly omitted here

    // Add the angular velocity feedforward
    _ang_vel_target_rads += _att_target_ang_vel_rads;
}

void AC_AttitudeControl::rate_bf_roll_pitch_yaw(float roll_rate_bf_cds, float pitch_rate_bf_cds, float yaw_rate_bf_cds)
{
    // Convert from centidegrees on public interface to radians
    float roll_rate_bf_rads = radians(roll_rate_bf_cds*0.01f);
    float pitch_rate_bf_rads = radians(pitch_rate_bf_cds*0.01f);
    float yaw_rate_bf_rads = radians(yaw_rate_bf_cds*0.01f);

    Vector3f    att_error_euler_rad;

    // Compute acceleration-limited euler roll rate
    if (get_accel_roll_max_radss() > 0.0f) {
        float rate_change_limit_rads = get_accel_roll_max_radss() * _dt;
        _att_target_ang_vel_rads.x += constrain_float(roll_rate_bf_rads - _att_target_ang_vel_rads.x, -rate_change_limit_rads, rate_change_limit_rads);
    } else {
        _att_target_ang_vel_rads.x = roll_rate_bf_rads;
    }

    // Compute acceleration-limited euler pitch rate
    if (get_accel_pitch_max_radss() > 0.0f) {
        float rate_change_limit_rads = get_accel_pitch_max_radss() * _dt;
        _att_target_ang_vel_rads.y += constrain_float(pitch_rate_bf_rads - _att_target_ang_vel_rads.y, -rate_change_limit_rads, rate_change_limit_rads);
    } else {
        _att_target_ang_vel_rads.y = pitch_rate_bf_rads;
    }

    // Compute acceleration-limited euler yaw rate
    if (get_accel_yaw_max_radss() > 0.0f) {
        float rate_change_limit_rads = get_accel_yaw_max_radss() * _dt;
        _att_target_ang_vel_rads.z += constrain_float(yaw_rate_bf_rads - _att_target_ang_vel_rads.z, -rate_change_limit_rads, rate_change_limit_rads);
    } else {
        _att_target_ang_vel_rads.z = yaw_rate_bf_rads;
    }

    // HACK: Because the attitude controller works on euler angles, things break down near 90 degrees of pitch. So, a different type of
    // controller is selected based on tilt angle.
    if (fabsf(_ahrs.pitch)<_acro_angle_switch_rad) {
        _acro_angle_switch_rad = radians(60.0f);

        // Convert body-frame demanded angular velocity into 321-intrinsic euler angle derivatives
        // NOTE: A rotation from vehicle body frame to demanded body frame is possibly omitted here
        // NOTE: This will never return false, since _ahrs.pitch cannot be +/- 90deg within this else statement
        ang_vel_to_euler_derivative(Vector3f(_ahrs.roll,_ahrs.pitch,_ahrs.yaw), _att_target_ang_vel_rads, _att_target_euler_deriv_rads);

        // Update the attitude target from the computed euler rates
        update_att_target_and_error_roll(_att_target_euler_deriv_rads.x, att_error_euler_rad, AC_ATTITUDE_RATE_STAB_ACRO_OVERSHOOT_ANGLE_MAX_RAD);
        update_att_target_and_error_pitch(_att_target_euler_deriv_rads.y, att_error_euler_rad, AC_ATTITUDE_RATE_STAB_ACRO_OVERSHOOT_ANGLE_MAX_RAD);
        update_att_target_and_error_yaw(_att_target_euler_deriv_rads.z, att_error_euler_rad, AC_ATTITUDE_RATE_STAB_ACRO_OVERSHOOT_ANGLE_MAX_RAD);

        // Convert 321-intrinsic euler angle errors to a body-frame rotation vector
        // NOTE: This results in an approximation of the attitude error based on a linearization about the current attitude
        euler_derivative_to_ang_vel(Vector3f(_ahrs.roll,_ahrs.pitch,_ahrs.yaw), att_error_euler_rad, _att_error_rot_vec_rad);
    } else {
        _acro_angle_switch_rad = radians(45.0f);

        // Integrate the angular velocity error into the attitude error
        integrate_bf_rate_error_to_angle_errors();

        // Convert angle error rotation vector into 321-intrinsic euler angle difference
        // NOTE: This results an an approximation linearized about the vehicle's attitude
        if(ang_vel_to_euler_derivative(Vector3f(_ahrs.roll,_ahrs.pitch,_ahrs.yaw), _att_error_rot_vec_rad, att_error_euler_rad)) {
            _att_target_euler_rad.x = wrap_PI(att_error_euler_rad.x + _ahrs.roll);
            _att_target_euler_rad.y = wrap_PI(att_error_euler_rad.y + _ahrs.pitch);
            _att_target_euler_rad.z = wrap_2PI(att_error_euler_rad.z + _ahrs.yaw);
        }
        if (_att_target_euler_rad.y > M_PI/2.0f) {
            _att_target_euler_rad.x = wrap_PI(_att_target_euler_rad.x + M_PI);
            _att_target_euler_rad.y = wrap_PI(M_PI - _att_target_euler_rad.y);
            _att_target_euler_rad.z = wrap_2PI(_att_target_euler_rad.z + M_PI);
        }
        if (_att_target_euler_rad.y < -M_PI/2.0f) {
            _att_target_euler_rad.x = wrap_PI(_att_target_euler_rad.x + M_PI);
            _att_target_euler_rad.y = wrap_PI(-M_PI - _att_target_euler_rad.y);
            _att_target_euler_rad.z = wrap_2PI(_att_target_euler_rad.z + M_PI);
        }
    }

    // Compute the angular velocity target from the attitude error
    update_ang_vel_target_from_att_error();

    // Add the angular velocity feedforward
    _ang_vel_target_rads += _att_target_ang_vel_rads;
}

void AC_AttitudeControl::rate_controller_run()
{
    _motors.set_roll(rate_bf_to_motor_roll(_ang_vel_target_rads.x));
    _motors.set_pitch(rate_bf_to_motor_pitch(_ang_vel_target_rads.y));
    _motors.set_yaw(rate_bf_to_motor_yaw(_ang_vel_target_rads.z));
}

void AC_AttitudeControl::euler_derivative_to_ang_vel(const Vector3f& euler_rad, const Vector3f& euler_dot_rads, Vector3f& ang_vel_rads)
{
    float sin_theta = sinf(euler_rad.y);
    float cos_theta = cosf(euler_rad.y);
    float sin_phi = sinf(euler_rad.x);
    float cos_phi = cosf(euler_rad.x);

    ang_vel_rads.x = euler_dot_rads.x - sin_theta * euler_dot_rads.z;
    ang_vel_rads.y = cos_phi  * euler_dot_rads.y + sin_phi * cos_theta * euler_dot_rads.z;
    ang_vel_rads.z = -sin_phi * euler_dot_rads.y + cos_theta * cos_phi * euler_dot_rads.z;
}

bool AC_AttitudeControl::ang_vel_to_euler_derivative(const Vector3f& euler_rad, const Vector3f& ang_vel_rads, Vector3f& euler_dot_rads)
{
    float sin_theta = sinf(euler_rad.y);
    float cos_theta = cosf(euler_rad.y);
    float sin_phi = sinf(euler_rad.x);
    float cos_phi = cosf(euler_rad.x);

    // When the vehicle pitches all the way up or all the way down, the euler angles become discontinuous. In this case, we just return false.
    if (is_zero(cos_theta)) {
        return false;
    }

    euler_dot_rads.x = ang_vel_rads.x + sin_phi * (sin_theta/cos_theta) * ang_vel_rads.y + cos_phi * (sin_theta/cos_theta) * ang_vel_rads.z;
    euler_dot_rads.y = cos_phi  * ang_vel_rads.y - sin_phi * ang_vel_rads.z;
    euler_dot_rads.z = (sin_phi / cos_theta) * ang_vel_rads.y + (cos_phi / cos_theta) * ang_vel_rads.z;
    return true;
}

void AC_AttitudeControl::update_att_target_and_error_roll(float euler_roll_rate_rads, Vector3f &att_error_euler_rad, float overshoot_max_rad)
{
    // Compute constrained angle error
    att_error_euler_rad.x = wrap_PI(_att_target_euler_rad.x - _ahrs.roll);
    att_error_euler_rad.x  = constrain_float(att_error_euler_rad.x, -overshoot_max_rad, overshoot_max_rad);

    // Update attitude target from constrained angle error
    _att_target_euler_rad.x = att_error_euler_rad.x + _ahrs.roll;

    // Increment the attitude target
    _att_target_euler_rad.x += euler_roll_rate_rads * _dt;
    _att_target_euler_rad.x = wrap_PI(_att_target_euler_rad.x);
}

void AC_AttitudeControl::update_att_target_and_error_pitch(float euler_pitch_rate_rads, Vector3f &att_error_euler_rad, float overshoot_max_rad)
{
    // Compute constrained angle error
    att_error_euler_rad.y = wrap_PI(_att_target_euler_rad.y - _ahrs.pitch);
    att_error_euler_rad.y  = constrain_float(att_error_euler_rad.y, -overshoot_max_rad, overshoot_max_rad);

    // Update attitude target from constrained angle error
    _att_target_euler_rad.y = att_error_euler_rad.y + _ahrs.pitch;

    // Increment the attitude target
    _att_target_euler_rad.y += euler_pitch_rate_rads * _dt;
    _att_target_euler_rad.y = wrap_PI(_att_target_euler_rad.y);
}

void AC_AttitudeControl::update_att_target_and_error_yaw(float euler_yaw_rate_rads, Vector3f &att_error_euler_rad, float overshoot_max_rad)
{
    // Compute constrained angle error
    att_error_euler_rad.z = wrap_PI(_att_target_euler_rad.z - _ahrs.yaw);
    att_error_euler_rad.z  = constrain_float(att_error_euler_rad.z, -overshoot_max_rad, overshoot_max_rad);

    // Update attitude target from constrained angle error
    _att_target_euler_rad.z = att_error_euler_rad.z + _ahrs.yaw;

    // Increment the attitude target
    _att_target_euler_rad.z += euler_yaw_rate_rads * _dt;
    _att_target_euler_rad.z = wrap_2PI(_att_target_euler_rad.z);
}

void AC_AttitudeControl::integrate_bf_rate_error_to_angle_errors()
{
    // Integrate the angular velocity error into the attitude error
    _att_error_rot_vec_rad += (_att_target_ang_vel_rads - _ahrs.get_gyro()) * _dt;

    // Constrain attitude error
    _att_error_rot_vec_rad.x = constrain_float(_att_error_rot_vec_rad.x, -AC_ATTITUDE_RATE_STAB_ACRO_OVERSHOOT_ANGLE_MAX_RAD, AC_ATTITUDE_RATE_STAB_ACRO_OVERSHOOT_ANGLE_MAX_RAD);
    _att_error_rot_vec_rad.y = constrain_float(_att_error_rot_vec_rad.y, -AC_ATTITUDE_RATE_STAB_ACRO_OVERSHOOT_ANGLE_MAX_RAD, AC_ATTITUDE_RATE_STAB_ACRO_OVERSHOOT_ANGLE_MAX_RAD);
    _att_error_rot_vec_rad.z = constrain_float(_att_error_rot_vec_rad.z, -AC_ATTITUDE_RATE_STAB_ACRO_OVERSHOOT_ANGLE_MAX_RAD, AC_ATTITUDE_RATE_STAB_ACRO_OVERSHOOT_ANGLE_MAX_RAD);
}

void AC_AttitudeControl::update_ang_vel_target_from_att_error()
{
    // Compute the roll angular velocity demand from the roll angle error
    if (_att_ctrl_use_accel_limit && _accel_roll_max > 0.0f) {
        _ang_vel_target_rads.x = sqrt_controller(_att_error_rot_vec_rad.x, _p_angle_roll.kP(), constrain_float(get_accel_roll_max_radss()/2.0f,  AC_ATTITUDE_ACCEL_RP_CONTROLLER_MIN_RADSS, AC_ATTITUDE_ACCEL_RP_CONTROLLER_MAX_RADSS));
    }else{
        _ang_vel_target_rads.x = _p_angle_roll.kP() * _att_error_rot_vec_rad.x;
    }

    // Compute the pitch angular velocity demand from the roll angle error
    if (_att_ctrl_use_accel_limit && _accel_pitch_max > 0.0f) {
        _ang_vel_target_rads.y = sqrt_controller(_att_error_rot_vec_rad.y, _p_angle_pitch.kP(), constrain_float(get_accel_pitch_max_radss()/2.0f,  AC_ATTITUDE_ACCEL_RP_CONTROLLER_MIN_RADSS, AC_ATTITUDE_ACCEL_RP_CONTROLLER_MAX_RADSS));
    }else{
        _ang_vel_target_rads.y = _p_angle_pitch.kP() * _att_error_rot_vec_rad.y;
    }

    // Compute the yaw angular velocity demand from the roll angle error
    if (_att_ctrl_use_accel_limit && _accel_yaw_max > 0.0f) {
        _ang_vel_target_rads.z = sqrt_controller(_att_error_rot_vec_rad.z, _p_angle_yaw.kP(), constrain_float(get_accel_yaw_max_radss()/2.0f,  AC_ATTITUDE_ACCEL_Y_CONTROLLER_MIN_RADSS, AC_ATTITUDE_ACCEL_Y_CONTROLLER_MAX_RADSS));
    }else{
        _ang_vel_target_rads.z = _p_angle_yaw.kP() * _att_error_rot_vec_rad.z;
    }

    // Account for precession of desired attitude about the body frame yaw axis
    _ang_vel_target_rads.x += _att_error_rot_vec_rad.y * _ahrs.get_gyro().z;
    _ang_vel_target_rads.y += -_att_error_rot_vec_rad.x * _ahrs.get_gyro().z;
}

float AC_AttitudeControl::rate_bf_to_motor_roll(float rate_target_rads)
{
    float current_rate_rads = _ahrs.get_gyro().x;
    float rate_error_rads = rate_target_rads - current_rate_rads;

    // For legacy reasons, we convert to centi-degrees before inputting to the PID
    _pid_rate_roll.set_input_filter_d(degrees(rate_error_rads)*100.0f);
    _pid_rate_roll.set_desired_rate(degrees(rate_target_rads)*100.0f);

    float integrator = _pid_rate_roll.get_integrator();

    // Ensure that integrator can only be reduced if the output is saturated
    if (!_motors.limit.roll_pitch || ((integrator > 0 && rate_error_rads < 0) || (integrator < 0 && rate_error_rads > 0))) {
        integrator = _pid_rate_roll.get_i();
    }

    // Compute output
    float output = _pid_rate_roll.get_p() + integrator + _pid_rate_roll.get_d();

    // Constrain output
    return constrain_float(output, -AC_ATTITUDE_RATE_RP_CONTROLLER_OUT_MAX, AC_ATTITUDE_RATE_RP_CONTROLLER_OUT_MAX);
}

float AC_AttitudeControl::rate_bf_to_motor_pitch(float rate_target_rads)
{
    float current_rate_rads = _ahrs.get_gyro().y;
    float rate_error_rads = rate_target_rads - current_rate_rads;

    // For legacy reasons, we convert to centi-degrees before inputting to the PID
    _pid_rate_pitch.set_input_filter_d(degrees(rate_error_rads)*100.0f);
    _pid_rate_pitch.set_desired_rate(degrees(rate_target_rads)*100.0f);

    float integrator = _pid_rate_pitch.get_integrator();

    // Ensure that integrator can only be reduced if the output is saturated
    if (!_motors.limit.roll_pitch || ((integrator > 0 && rate_error_rads < 0) || (integrator < 0 && rate_error_rads > 0))) {
        integrator = _pid_rate_pitch.get_i();
    }

    // Compute output
    float output = _pid_rate_pitch.get_p() + integrator + _pid_rate_pitch.get_d();

    // Constrain output
    return constrain_float(output, -AC_ATTITUDE_RATE_RP_CONTROLLER_OUT_MAX, AC_ATTITUDE_RATE_RP_CONTROLLER_OUT_MAX);
}

float AC_AttitudeControl::rate_bf_to_motor_yaw(float rate_target_rads)
{
    float current_rate_rads = _ahrs.get_gyro().z;
    float rate_error_rads = rate_target_rads - current_rate_rads;

    // For legacy reasons, we convert to centi-degrees before inputting to the PID
    _pid_rate_yaw.set_input_filter_d(degrees(rate_error_rads)*100.0f);
    _pid_rate_yaw.set_desired_rate(degrees(rate_target_rads)*100.0f);

    float integrator = _pid_rate_yaw.get_integrator();

    // Ensure that integrator can only be reduced if the output is saturated
    if (!_motors.limit.yaw || ((integrator > 0 && rate_error_rads < 0) || (integrator < 0 && rate_error_rads > 0))) {
        integrator = _pid_rate_yaw.get_i();
    }

    // Compute output
    float output = _pid_rate_yaw.get_p() + integrator + _pid_rate_yaw.get_d();

    // Constrain output
    return constrain_float(output, -AC_ATTITUDE_RATE_YAW_CONTROLLER_OUT_MAX, AC_ATTITUDE_RATE_YAW_CONTROLLER_OUT_MAX);
}

void AC_AttitudeControl::accel_limiting(bool enable_limits)
{
    if (enable_limits) {
        // If enabling limits, reload from eeprom or set to defaults
        if (is_zero(_accel_roll_max)) {
            _accel_roll_max.load();
        }
        if (is_zero(_accel_pitch_max)) {
            _accel_pitch_max.load();
        }
        if (is_zero(_accel_yaw_max)) {
            _accel_yaw_max.load();
        }
    } else {
        _accel_roll_max = 0.0f;
        _accel_pitch_max = 0.0f;
        _accel_yaw_max = 0.0f;
    }
}

void AC_AttitudeControl::set_throttle_out(float throttle_in, bool apply_angle_boost, float filter_cutoff)
{
    _throttle_in_filt.apply(throttle_in, _dt);
    _motors.set_stabilizing(true);
    _motors.set_throttle_filter_cutoff(filter_cutoff);
    if (apply_angle_boost) {
        _motors.set_throttle(get_boosted_throttle(throttle_in));
    }else{
        _motors.set_throttle(throttle_in);
        // Clear angle_boost for logging purposes
        _angle_boost = 0;
    }
}

void AC_AttitudeControl::set_throttle_out_unstabilized(float throttle_in, bool reset_attitude_control, float filter_cutoff)
{
    _throttle_in_filt.apply(throttle_in, _dt);
    if (reset_attitude_control) {
        relax_bf_rate_controller();
        set_yaw_target_to_current_heading();
    }
    _motors.set_throttle_filter_cutoff(filter_cutoff);
    _motors.set_stabilizing(false);
    _motors.set_throttle(throttle_in);
    _angle_boost = 0;
}

float AC_AttitudeControl::sqrt_controller(float error, float p, float second_ord_lim)
{
    if (is_zero(second_ord_lim) || is_zero(p)) {
        return error*p;
    }

    float linear_dist = second_ord_lim/sq(p);

    if (error > linear_dist) {
        return safe_sqrt(2.0f*second_ord_lim*(error-(linear_dist/2.0f)));
    } else if (error < -linear_dist) {
        return -safe_sqrt(2.0f*second_ord_lim*(-error-(linear_dist/2.0f)));
    } else {
        return error*p;
    }
}

float AC_AttitudeControl::max_rate_step_bf_roll()
{
    float alpha = _pid_rate_roll.get_filt_alpha();
    float alpha_remaining = 1-alpha;
    return AC_ATTITUDE_RATE_RP_CONTROLLER_OUT_MAX/((alpha_remaining*alpha_remaining*alpha_remaining*alpha*_pid_rate_roll.kD())/_dt + _pid_rate_roll.kP());
}

float AC_AttitudeControl::max_rate_step_bf_pitch()
{
    float alpha = _pid_rate_pitch.get_filt_alpha();
    float alpha_remaining = 1-alpha;
    return AC_ATTITUDE_RATE_RP_CONTROLLER_OUT_MAX/((alpha_remaining*alpha_remaining*alpha_remaining*alpha*_pid_rate_pitch.kD())/_dt + _pid_rate_pitch.kP());
}

float AC_AttitudeControl::max_rate_step_bf_yaw()
{
    float alpha = _pid_rate_yaw.get_filt_alpha();
    float alpha_remaining = 1-alpha;
    return AC_ATTITUDE_RATE_RP_CONTROLLER_OUT_MAX/((alpha_remaining*alpha_remaining*alpha_remaining*alpha*_pid_rate_yaw.kD())/_dt + _pid_rate_yaw.kP());
}
