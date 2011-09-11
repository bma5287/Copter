// -*- tab-width: 4; Mode: C++; c-basic-offset: 4; indent-tabs-mode: t -*-

/// @file	RC_Channel_aux.h
/// @brief	RC_Channel manager for Channels 5..8, with EEPROM-backed storage of constants.
/// @author Amilcar Lucas

#ifndef RC_CHANNEL_AUX_H_
#define RC_CHANNEL_AUX_H_

#include "RC_Channel.h"

/// @class	RC_Channel_aux
/// @brief	Object managing one aux. RC channel (CH5-8), with information about its function
/// 	    Also contains physical min,max angular deflection, to allow calibrating open-loop servo movements
class RC_Channel_aux : public RC_Channel{
public:
	/// Constructor
	///
	/// @param key      EEPROM storage key for the channel trim parameters.
	/// @param name     Optional name for the group.
	///
	RC_Channel_aux(AP_Var::Key key, const prog_char_t *name) :
		RC_Channel(key, name),
		function  (&_group, 4,  k_none, name ? PSTR("FUNCTION")  : 0), // suppress name if group has no name
		angle_min (&_group, 5, -4500, name ? PSTR("ANGLE_MIN") : 0), // assume -45 degrees min deflection
		angle_max (&_group, 6,  4500, name ? PSTR("ANGLE_MAX") : 0)  // assume  45 degrees max deflection
	{}

	typedef enum
	{
		k_none			= 0,	// disabled
		k_manual		= 1,	// manual, just pass-thru the RC in signal
		k_flap			= 2,	// flap
		k_flap_auto		= 3,	// flap automated
		k_aileron		= 4,	// aileron
		k_flaperon		= 5,	// flaperon (flaps and aileron combined, needs two independent servos one for each wing)
		k_mount_yaw		= 6,	// mount yaw (pan)
		k_mount_pitch	= 7,	// mount pitch (tilt)
		k_mount_roll	= 8,	// mount roll
		k_cam_trigger	= 9,	// camera trigger
		k_cam_open		= 10,	// camera open
		k_egg_drop		= 11,	// egg drop
		k_nr_aux_servo_functions // This must be the last enum value (only add new values _before_ this one)
	} Aux_servo_function_t;

	AP_Int8 	function;	// 0=disabled, 1=manual, 2=flap, 3=flap auto, 4=aileron, 5=flaperon, 6=mount yaw (pan), 7=mount pitch (tilt), 8=mount roll, 9=camera trigger, 10=camera open, 11=egg drop
	AP_Int16 	angle_min;	// min angle limit of actuated surface in 0.01 degree units
	AP_Int16 	angle_max;	// max angle limit of actuated surface in 0.01 degree units

	int16_t closest_limit(int16_t angle);	// saturate to the closest angle limit if outside of min max angle interval

	void output_ch(unsigned char ch_nr);	// map a function to a servo channel and output it

};

void update_aux_servo_function(RC_Channel_aux* rc_5, RC_Channel_aux* rc_6, RC_Channel_aux* rc_7, RC_Channel_aux* rc_8);

#endif /* RC_CHANNEL_AUX_H_ */
