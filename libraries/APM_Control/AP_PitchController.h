// -*- tab-width: 4; Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*-

#ifndef __AP_PITCH_CONTROLLER_H__
#define __AP_PITCH_CONTROLLER_H__

#include <AP_AHRS.h>
#include <AP_Common.h>
#include <AP_Vehicle.h>
#include <AP_AutoTune.h>
#include <DataFlash.h>
#include <AP_Math.h>

class AP_PitchController {
public:
	AP_PitchController(AP_AHRS &ahrs, const AP_Vehicle::FixedWing &parms, DataFlash_Class &_dataflash) :
		aparm(parms),
        autotune(gains, AP_AutoTune::AUTOTUNE_PITCH, _dataflash),
        _ahrs(ahrs)
    { 
		AP_Param::setup_object_defaults(this, var_info);
	}

	int32_t get_rate_out(float desired_rate, float scaler);
	int32_t get_servo_out(int32_t angle_err, float scaler, bool disable_integrator);

	void reset_I();

    void autotune_start(void) { autotune.start(); }
    void autotune_restore(void) { autotune.stop(); }

	static const struct AP_Param::GroupInfo var_info[];

private:
	const AP_Vehicle::FixedWing &aparm;
    AP_AutoTune::ATGains gains;
    AP_AutoTune autotune;
	AP_Int16 _max_rate_neg;
	AP_Float _roll_ff;
	uint32_t _last_t;
	float _last_out;
	
	float _integrator;

	int32_t _get_rate_out(float desired_rate, float scaler, bool disable_integrator, float aspeed);
    float   _get_coordination_rate_offset(float &aspeed, bool &inverted) const;
	
	AP_AHRS &_ahrs;
	
};

#endif // __AP_PITCH_CONTROLLER_H__
