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

//	Initial Code by Jon Challinger
//  Modified by Paul Riseborough

#include <AP_HAL/AP_HAL.h>
#include "AP_PitchController.h"

extern const AP_HAL::HAL& hal;

const AP_Param::GroupInfo AP_PitchController::var_info[] = {

    // @Param: 2SRV_TCONST
	// @DisplayName: Pitch Time Constant
	// @Description: Time constant in seconds from demanded to achieved pitch angle. Most models respond well to 0.5. May be reduced for faster responses, but setting lower than a model can achieve will not help.
	// @Range: 0.4 1.0
	// @Units: s
	// @Increment: 0.1
	// @User: Advanced
    AP_GROUPINFO("2SRV_TCONST",      0, AP_PitchController, gains.tau,       0.5f),

    // index 1 to 3 reserved for old PID values

    // @Param: 2SRV_RMAX_UP
	// @DisplayName: Pitch up max rate
	// @Description: Maximum pitch up rate that the pitch controller demands (degrees/sec) in ACRO mode.
	// @Range: 0 100
	// @Units: deg/s
	// @Increment: 1
	// @User: Advanced
    AP_GROUPINFO("2SRV_RMAX_UP",     4, AP_PitchController, gains.rmax,   0.0f),

    // @Param: 2SRV_RMAX_DN
	// @DisplayName: Pitch down max rate
	// @Description: This sets the maximum nose down pitch rate that the controller will demand (degrees/sec). Setting it to zero disables the limit.
	// @Range: 0 100
	// @Units: deg/s
	// @Increment: 1
	// @User: Advanced
    AP_GROUPINFO("2SRV_RMAX_DN",     5, AP_PitchController, _max_rate_neg,   0.0f),

    // @Param: 2SRV_RLL
	// @DisplayName: Roll compensation
	// @Description: Gain added to pitch to keep aircraft from descending or ascending in turns. Increase in increments of 0.05 to reduce altitude loss. Decrease for altitude gain.
	// @Range: 0.7 1.5
	// @Increment: 0.05
    // @User: Standard
    AP_GROUPINFO("2SRV_RLL",      6, AP_PitchController, _roll_ff,        1.0f),

    // @Param: 2SRV_IMAX
	// @DisplayName: Integrator limit
	// @Description: Limit of pitch integrator gain in centi-degrees of servo travel. Servos are assumed to have +/- 4500 centi-degrees of travel, so a value of 3000 allows trim of up to 2/3 of servo travel range.
	// @Range: 0 4500
	// @Increment: 1
	// @User: Advanced
    AP_GROUPINFO("2SRV_IMAX",      7, AP_PitchController, gains.imax,     3000),

    // index 8 reserved for old FF

    // @Param: 2SRV_SRMAX
    // @DisplayName: Servo slew rate limit
    // @Description: Sets an upper limit on the servo slew rate produced by the D-gain (pitch rate feedback). If the amplitude of the control action produced by the pitch rate feedback exceeds this value, then the D-gain is reduced to respect the limit. This limits the amplitude of high frequency oscillations caused by an excessive D-gain. The limit should be set to no more than 25% of the servo's specified slew rate to allow for inertia and aerodynamic load effects. Note: The D-gain will not be reduced to less than 10% of the nominal value. A value of zero will disable this feature.
    // @Units: deg/s
    // @Range: 0 500
    // @Increment: 10.0
    // @User: Advanced
    AP_GROUPINFO("2SRV_SRMAX", 9, AP_PitchController, _slew_rate_max, 150.0f),

    // @Param: 2SRV_SRTAU
    // @DisplayName: Servo slew rate decay time constant
    // @Description: This sets the time constant used to recover the D gain after it has been reduced due to excessive servo slew rate.
    // @Units: s
    // @Range: 0.5 5.0
    // @Increment: 0.1
    // @User: Advanced
    AP_GROUPINFO("2SRV_SRTAU", 10, AP_PitchController, _slew_rate_tau, 1.0f),

    // @Param: _RATE_P
    // @DisplayName: Pitch axis rate controller P gain
    // @Description: Pitch axis rate controller P gain.  Converts the difference between desired roll rate and actual roll rate into a motor speed output
    // @Range: 0.08 0.35
    // @Increment: 0.005
    // @User: Standard

    // @Param: _RATE_I
    // @DisplayName: Pitch axis rate controller I gain
    // @Description: Pitch axis rate controller I gain.  Corrects long-term difference in desired roll rate vs actual roll rate
    // @Range: 0.01 0.6
    // @Increment: 0.01
    // @User: Standard

    // @Param: _RATE_IMAX
    // @DisplayName: Pitch axis rate controller I gain maximum
    // @Description: Pitch axis rate controller I gain maximum.  Constrains the maximum motor output that the I gain will output
    // @Range: 0 1
    // @Increment: 0.01
    // @User: Standard

    // @Param: _RATE_D
    // @DisplayName: Pitch axis rate controller D gain
    // @Description: Pitch axis rate controller D gain.  Compensates for short-term change in desired roll rate vs actual roll rate
    // @Range: 0.001 0.03
    // @Increment: 0.001
    // @User: Standard

    // @Param: _RATE_FF
    // @DisplayName: Pitch axis rate controller feed forward
    // @Description: Pitch axis rate controller feed forward
    // @Range: 0 3.0
    // @Increment: 0.001
    // @User: Standard

    // @Param: _RATE_FLTT
    // @DisplayName: Pitch axis rate controller target frequency in Hz
    // @Description: Pitch axis rate controller target frequency in Hz
    // @Range: 2 50
    // @Increment: 1
    // @Units: Hz
    // @User: Standard

    // @Param: _RATE_FLTE
    // @DisplayName: Pitch axis rate controller error frequency in Hz
    // @Description: Pitch axis rate controller error frequency in Hz
    // @Range: 2 50
    // @Increment: 1
    // @Units: Hz
    // @User: Standard

    // @Param: _RATE_FLTD
    // @DisplayName: Pitch axis rate controller derivative frequency in Hz
    // @Description: Pitch axis rate controller derivative frequency in Hz
    // @Range: 0 50
    // @Increment: 1
    // @Units: Hz
    // @User: Standard

    // @Param: _RATE_SMAX
    // @DisplayName: Pitch slew rate limit
    // @Description: Sets an upper limit on the slew rate produced by the combined P and D gains. If the amplitude of the control action produced by the rate feedback exceeds this value, then the D+P gain is reduced to respect the limit. This limits the amplitude of high frequency oscillations caused by an excessive gain. The limit should be set to no more than 25% of the actuators maximum slew rate to allow for load effects. Note: The gain will not be reduced to less than 10% of the nominal value. A value of zero will disable this feature.
    // @Range: 0 200
    // @Increment: 0.5
    // @User: Advanced

    // @Param: _RATE_STAU
    // @DisplayName: Pitch slew rate decay time constant
    // @Description: This sets the time constant used to recover the P+D gain after it has been reduced due to excessive slew rate.
    // @Units: s
    // @Range: 0.5 5.0
    // @Increment: 0.1
    // @User: Advanced

    AP_SUBGROUPINFO(rate_pid, "_RATE_", 11, AP_PitchController, AC_PID),
    
    AP_GROUPEND
};

/*
  AC_PID based rate controller
*/
int32_t AP_PitchController::_get_rate_out(float desired_rate, float scaler, bool disable_integrator, float aspeed)
{
    const float dt = AP::scheduler().get_loop_period_s();
    const float eas2tas = _ahrs.get_EAS2TAS();
    bool limit_I = fabsf(last_ac_out) >= 45;
    float rate_y = _ahrs.get_gyro().y;
    float old_I = rate_pid.get_i();

    rate_pid.set_dt(dt);

    bool underspeed = aspeed <= 0.5*float(aparm.airspeed_min);
    if (underspeed) {
        limit_I = true;
    }

    // the P and I elements are scaled by sq(scaler). To use an
    // unmodified AC_PID object we scale the inputs and calculate FF separately
    //
    // note that we run AC_PID in radians so that the normal scaling
    // range for IMAX in AC_PID applies (usually an IMAX value less than 1.0)
    rate_pid.update_all(radians(desired_rate) * scaler * scaler, rate_y * scaler * scaler, limit_I);

    if (underspeed) {
        // when underspeed we lock the integrator
        rate_pid.set_integrator(old_I);
    }
    
    // FF should be scaled by scaler/eas2tas, but since we have scaled
    // the AC_PID target above by scaler*scaler we need to instead
    // divide by scaler*eas2tas to get the right scaling
    const float ff = degrees(rate_pid.get_ff() / (scaler * eas2tas));

    if (disable_integrator) {
        rate_pid.reset_I();
    }

    // convert AC_PID info object to same scale as old controller
    _pid_info = rate_pid.get_pid_info();
    auto &pinfo = _pid_info;

    const float deg_scale = degrees(1);
    pinfo.FF = ff;
    pinfo.P *= deg_scale;
    pinfo.I *= deg_scale;
    pinfo.D *= deg_scale;

    // fix the logged target and actual values to not have the scalers applied
    pinfo.target = desired_rate;
    pinfo.actual = degrees(rate_y);

    // sum components
    float out = pinfo.FF + pinfo.P + pinfo.I + pinfo.D;

    /*
      when we are past the users defined roll limit for the
      aircraft our priority should be to bring the aircraft back
      within the roll limit. Using elevator for pitch control at
      large roll angles is ineffective, and can be counter
      productive as it induces earth-frame yaw which can reduce
      the ability to roll. We linearly reduce elevator input when
      beyond the configured roll limit, reducing to zero at 90
      degrees
    */
    float roll_wrapped = labs(_ahrs.roll_sensor);
    if (roll_wrapped > 9000) {
        roll_wrapped = 18000 - roll_wrapped;
    }
    if (roll_wrapped > aparm.roll_limit_cd + 500 && aparm.roll_limit_cd < 8500 &&
        labs(_ahrs.pitch_sensor) < 7000) {
        float roll_prop = (roll_wrapped - (aparm.roll_limit_cd+500)) / (float)(9000 - aparm.roll_limit_cd);
        _last_out *= (1 - roll_prop);
    }

    // remember the last output to trigger the I limit
    last_ac_out = out;

    // output is scaled to notional centidegrees of deflection
    return constrain_int32(out * 100, -4500, 4500);
}

/*
 Function returns an equivalent elevator deflection in centi-degrees in the range from -4500 to 4500
 A positive demand is up
 Inputs are: 
 1) demanded pitch rate in degrees/second
 2) control gain scaler = scaling_speed / aspeed
 3) boolean which is true when stabilise mode is active
 4) minimum FBW airspeed (metres/sec)
 5) maximum FBW airspeed (metres/sec)
*/
int32_t AP_PitchController::get_rate_out(float desired_rate, float scaler)
{
    float aspeed;
	if (!_ahrs.airspeed_estimate(aspeed)) {
	    // If no airspeed available use average of min and max
        aspeed = 0.5f*(float(aparm.airspeed_min) + float(aparm.airspeed_max));
	}
    return _get_rate_out(desired_rate, scaler, false, aspeed);
}

/*
  get the rate offset in degrees/second needed for pitch in body frame
  to maintain height in a coordinated turn.
  
  Also returns the inverted flag and the estimated airspeed in m/s for
  use by the rest of the pitch controller
 */
float AP_PitchController::_get_coordination_rate_offset(float &aspeed, bool &inverted) const
{
	float rate_offset;
	float bank_angle = _ahrs.roll;

	// limit bank angle between +- 80 deg if right way up
	if (fabsf(bank_angle) < radians(90))	{
	    bank_angle = constrain_float(bank_angle,-radians(80),radians(80));
        inverted = false;
	} else {
		inverted = true;
		if (bank_angle > 0.0f) {
			bank_angle = constrain_float(bank_angle,radians(100),radians(180));
		} else {
			bank_angle = constrain_float(bank_angle,-radians(180),-radians(100));
		}
	}
	if (!_ahrs.airspeed_estimate(aspeed)) {
	    // If no airspeed available use average of min and max
        aspeed = 0.5f*(float(aparm.airspeed_min) + float(aparm.airspeed_max));
	}
    if (abs(_ahrs.pitch_sensor) > 7000) {
        // don't do turn coordination handling when at very high pitch angles
        rate_offset = 0;
    } else {
        rate_offset = cosf(_ahrs.pitch)*fabsf(ToDeg((GRAVITY_MSS / MAX((aspeed * _ahrs.get_EAS2TAS()) , MAX(aparm.airspeed_min, 1))) * tanf(bank_angle) * sinf(bank_angle))) * _roll_ff;
    }
	if (inverted) {
		rate_offset = -rate_offset;
	}
    return rate_offset;
}

// Function returns an equivalent elevator deflection in centi-degrees in the range from -4500 to 4500
// A positive demand is up
// Inputs are: 
// 1) demanded pitch angle in centi-degrees
// 2) control gain scaler = scaling_speed / aspeed
// 3) boolean which is true when stabilise mode is active
// 4) minimum FBW airspeed (metres/sec)
// 5) maximum FBW airspeed (metres/sec)
//
int32_t AP_PitchController::get_servo_out(int32_t angle_err, float scaler, bool disable_integrator)
{
	// Calculate offset to pitch rate demand required to maintain pitch angle whilst banking
	// Calculate ideal turn rate from bank angle and airspeed assuming a level coordinated turn
	// Pitch rate offset is the component of turn rate about the pitch axis
	float aspeed;
	float rate_offset;
	bool inverted;

    if (gains.tau < 0.1f) {
        gains.tau.set(0.1f);
    }

    rate_offset = _get_coordination_rate_offset(aspeed, inverted);
	
	// Calculate the desired pitch rate (deg/sec) from the angle error
	float desired_rate = angle_err * 0.01f / gains.tau;
	
	// limit the maximum pitch rate demand. Don't apply when inverted
	// as the rates will be tuned when upright, and it is common that
	// much higher rates are needed inverted	
	if (!inverted) {
		if (_max_rate_neg && desired_rate < -_max_rate_neg) {
			desired_rate = -_max_rate_neg;
		} else if (gains.rmax && desired_rate > gains.rmax) {
			desired_rate = gains.rmax;
		}
	}
	
	if (inverted) {
		desired_rate = -desired_rate;
	}

	// Apply the turn correction offset
	desired_rate = desired_rate + rate_offset;

    return _get_rate_out(desired_rate, scaler, disable_integrator, aspeed);
}

void AP_PitchController::reset_I()
{
    _pid_info.I = 0;
    rate_pid.reset_I();
}

/*
  convert from old to new PIDs
  this is a temporary conversion function during development
 */
void AP_PitchController::convert_pid()
{
    AP_Float &ff = rate_pid.ff();
    if (ff.configured_in_storage()) {
        return;
    }

    float old_ff=0, old_p=1.0, old_i=0.3, old_d=0.08;
    bool have_old = AP_Param::get_param_by_index(this, 1, AP_PARAM_FLOAT, &old_p);
    have_old |= AP_Param::get_param_by_index(this, 3, AP_PARAM_FLOAT, &old_i);
    have_old |= AP_Param::get_param_by_index(this, 2, AP_PARAM_FLOAT, &old_d);
    have_old |= AP_Param::get_param_by_index(this, 8, AP_PARAM_FLOAT, &old_ff);
    if (!have_old) {
        // none of the old gains were set
        return;
    }

    const float kp_ff = MAX((old_p - old_i * gains.tau) * gains.tau  - old_d, 0);
    rate_pid.ff().set_and_save(old_ff + kp_ff);
    rate_pid.kI().set_and_save_ifchanged(old_i * gains.tau);
    rate_pid.kP().set_and_save_ifchanged(old_d);
    rate_pid.kD().set_and_save_ifchanged(0);
    rate_pid.kIMAX().set_and_save_ifchanged(gains.imax/4500.0);
}
