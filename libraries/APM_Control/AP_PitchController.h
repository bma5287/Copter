#pragma once

#include <AP_AHRS/AP_AHRS.h>
#include <AP_Common/AP_Common.h>
#include <AP_Vehicle/AP_Vehicle.h>
#include "AP_AutoTune.h"
#include <AP_Logger/AP_Logger.h>
#include <AP_Math/AP_Math.h>

class AP_PitchController {
public:
    AP_PitchController(AP_AHRS &ahrs, const AP_Vehicle::FixedWing &parms)
        : aparm(parms)
        , autotune(gains, AP_AutoTune::AUTOTUNE_PITCH, parms)
        , _ahrs(ahrs)
    {
        AP_Param::setup_object_defaults(this, var_info);
        _slew_rate_filter.set_cutoff_frequency(10.0f);
        _slew_rate_filter.reset(0.0f);
    }

    /* Do not allow copies */
    AP_PitchController(const AP_PitchController &other) = delete;
    AP_PitchController &operator=(const AP_PitchController&) = delete;

	int32_t get_rate_out(float desired_rate, float scaler);
	int32_t get_servo_out(int32_t angle_err, float scaler, bool disable_integrator);

	void reset_I();

    /*
      reduce the integrator, used when we have a low scale factor in a quadplane hover
    */
    void decay_I() {
        // this reduces integrator by 95% over 2s
        _pid_info.I *= 0.995f;
    }
    
    void autotune_start(void) { autotune.start(); }
    void autotune_restore(void) { autotune.stop(); }

    const AP_Logger::PID_Info& get_pid_info(void) const { return _pid_info; }

	static const struct AP_Param::GroupInfo var_info[];

    AP_Float &kP(void) { return gains.P; }
    AP_Float &kI(void) { return gains.I; }
    AP_Float &kD(void) { return gains.D; }
    AP_Float &kFF(void) { return gains.FF; }

private:
    const AP_Vehicle::FixedWing &aparm;
    AP_AutoTune::ATGains gains;
    AP_AutoTune autotune;
	AP_Int16 _max_rate_neg;
	AP_Float _roll_ff;
	uint32_t _last_t;
	float _last_out;
	
    AP_Logger::PID_Info _pid_info;

	int32_t _get_rate_out(float desired_rate, float scaler, bool disable_integrator, float aspeed);
    float   _get_coordination_rate_offset(float &aspeed, bool &inverted) const;
	
	AP_AHRS &_ahrs;

    // D gain limit cycle control
    float _last_pid_info_D;                 // value of the D term (angular rate control feedback) from the previous time step (deg)
    LowPassFilterFloat _slew_rate_filter;   // LPF applied to the derivative of the control action generated by the angular rate feedback
    float _slew_rate_amplitude;             // Amplitude of the servo slew rate produced by the angular rate feedback (deg/sec)
    float _D_gain_modifier = 1.0f;          // Gain modifier applied to the angular rate feedback to prevent excessive slew rate
    AP_Float _slew_rate_max;                // Maximum permitted angular rate control feedback servo slew rate (deg/sec)
    AP_Float _slew_rate_tau;                // Time constant used to recover gain after a slew rate exceedance (sec)

};
