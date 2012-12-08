// -*- tab-width: 4; Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*-

// Example config file. Take a look at config.h. Any term define there can be overridden by defining it here.

//#define CONFIG_APM_HARDWARE APM_HARDWARE_APM2

// Ordinary users should please ignore the following define.
// APM2_BETA_HARDWARE is used to support early (September-October 2011) APM2
// hardware which had the BMP085 barometer onboard. Only a handful of
// developers have these boards.
//#define APM2_BETA_HARDWARE

// GPS is auto-selected

//#define MAG_ORIENTATION		AP_COMPASS_COMPONENTS_DOWN_PINS_FORWARD
//#define HIL_MODE				HIL_MODE_ATTITUDE
//#define DMP_ENABLED ENABLED
//#define SECONDARY_DMP_ENABLED ENABLED       // allows running DMP in parallel with DCM for testing purposes

//#define FRAME_CONFIG QUAD_FRAME
/*
 *  options:
 *  QUAD_FRAME
 *  TRI_FRAME
 *  HEXA_FRAME
 *  Y6_FRAME
 *  OCTA_FRAME
 *  OCTA_QUAD_FRAME
 *  HELI_FRAME
 */

//#define FRAME_ORIENTATION X_FRAME
/*
 *  PLUS_FRAME
 *  X_FRAME
 *  V_FRAME
 */

//#define CH7_OPTION		CH7_SAVE_WP
/*
 *  CH7_DO_NOTHING
 *  CH7_SET_HOVER           // deprecated
 *  CH7_FLIP
 *  CH7_SIMPLE_MODE
 *  CH7_RTL
 *  CH7_SAVE_TRIM
 *  CH7_ADC_FILTER          // deprecated
 *  CH7_SAVE_WP
 *  CH7_MULTI_MODE          // deprecated
 *  CH7_CAMERA_TRIGGER
 */

//#define TOY_EDF	ENABLED
//#define TOY_MIXER TOY_LOOKUP_TABLE

// Inertia based contollers
//#define INERTIAL_NAV_XY ENABLED
#define INERTIAL_NAV_Z ENABLED


//#define RATE_ROLL_I   0.18
//#define RATE_PITCH_I	0.18
//#define MOTORS_JD880
//#define MOTORS_JD850


// agmatthews USERHOOKS
// the choice of function names is up to the user and does not have to match these
// uncomment these hooks and ensure there is a matching function on your "UserCode.pde" file
//#define USERHOOK_FASTLOOP userhook_FastLoop();
#define USERHOOK_50HZLOOP userhook_50Hz();
//#define USERHOOK_MEDIUMLOOP userhook_MediumLoop();
//#define USERHOOK_SLOWLOOP userhook_SlowLoop();
//#define USERHOOK_SUPERSLOWLOOP userhook_SuperSlowLoop();
#define USERHOOK_INIT userhook_init();

// the choice of included variables file (*.h) is up to the user and does not have to match this one
// Ensure the defined file exists and is in the arducopter directory
#define USERHOOK_VARIABLES "UserVariables.h"

//#define LOGGING_ENABLED		DISABLED

// Used to set the max roll and pitch angles in Degrees * 100
//#define MAX_INPUT_ROLL_ANGLE      8000
//#define MAX_INPUT_PITCH_ANGLE     8000

/////////////////////////////////////////////////////////////////////////////////
// Bulk defines for TradHeli. Cleans up defines.h and config.h to put these here
#if FRAME_CONFIG == HELI_FRAME
  # define RC_FAST_SPEED 				125
  # define RTL_YAW                  	YAW_LOOK_AT_HOME
  # define TILT_COMPENSATION 			5
  # define RATE_INTEGRATOR_LEAK_RATE 	0.02
  # define RATE_ROLL_D    				0
  # define RATE_PITCH_D       			0
  # define HELI_PITCH_FF				0
  # define HELI_ROLL_FF					0
  # define HELI_YAW_FF					0  
 #endif
