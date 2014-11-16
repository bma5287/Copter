// -*- tab-width: 4; Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*-
//
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//
// WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING
//
//  DO NOT EDIT this file to adjust your configuration.  Create your own
//  APM_Config.h and use APM_Config.h.example as a reference.
//
// WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING WARNING
///
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//
// Default and automatic configuration details.
//
// Notes for maintainers:
//
// - Try to keep this file organised in the same order as APM_Config.h.example
//

#include "defines.h"

///
/// DO NOT EDIT THIS INCLUDE - if you want to make a local change, make that
/// change in your local copy of APM_Config.h.
///
#include "APM_Config.h"  // <== THIS INCLUDE, DO NOT EDIT IT. EVER.
///
/// DO NOT EDIT THIS INCLUDE - if you want to make a local change, make that
/// change in your local copy of APM_Config.h.
///

#if defined( __AVR_ATmega1280__ )
 // default choices for a 1280. We can't fit everything in, so we 
 // make some popular choices by default
 #define LOGGING_ENABLED DISABLED
 #ifndef MOUNT
 # define MOUNT DISABLED
 #endif
 #ifndef CAMERA
 # define CAMERA DISABLED
 #endif
#endif

// Just so that it's completely clear...
#define ENABLED			1
#define DISABLED		0

//////////////////////////////////////////////////////////////////////////////
// sensor types

//////////////////////////////////////////////////////////////////////////////
// HIL_MODE                                 OPTIONAL

#ifndef HIL_MODE
 #define HIL_MODE        HIL_MODE_DISABLED
#endif

#if CONFIG_HAL_BOARD == HAL_BOARD_APM1
# define BATTERY_PIN_1	  0
# define CURRENT_PIN_1	  1
#elif CONFIG_HAL_BOARD == HAL_BOARD_APM2
# define BATTERY_PIN_1	  1
# define CURRENT_PIN_1	  2
#elif CONFIG_HAL_BOARD == HAL_BOARD_AVR_SITL
# define BATTERY_PIN_1	  1
# define CURRENT_PIN_1	  2
#elif CONFIG_HAL_BOARD == HAL_BOARD_PX4
# define BATTERY_PIN_1	  -1
# define CURRENT_PIN_1	  -1
#elif CONFIG_HAL_BOARD == HAL_BOARD_FLYMAPLE
# define BATTERY_PIN_1     20
# define CURRENT_PIN_1	   19
#elif CONFIG_HAL_BOARD == HAL_BOARD_LINUX
# define BATTERY_PIN_1     -1
# define CURRENT_PIN_1	   -1
#elif CONFIG_HAL_BOARD == HAL_BOARD_VRBRAIN
# define BATTERY_PIN_1	  -1
# define CURRENT_PIN_1	  -1
#endif

//////////////////////////////////////////////////////////////////////////////
// HIL_MODE                                 OPTIONAL

#ifndef HIL_MODE
#define HIL_MODE	HIL_MODE_DISABLED
#endif

#ifndef MAV_SYSTEM_ID
# define MAV_SYSTEM_ID		1
#endif


//////////////////////////////////////////////////////////////////////////////
// FrSky telemetry support
//

#ifndef FRSKY_TELEM_ENABLED
#if CONFIG_HAL_BOARD == HAL_BOARD_APM1 || CONFIG_HAL_BOARD == HAL_BOARD_APM2
 # define FRSKY_TELEM_ENABLED DISABLED
#else
 # define FRSKY_TELEM_ENABLED ENABLED
#endif
#endif


#ifndef CH7_OPTION
# define CH7_OPTION		          CH7_SAVE_WP
#endif

#ifndef TUNING_OPTION
# define TUNING_OPTION		          TUN_NONE
#endif

//////////////////////////////////////////////////////////////////////////////
// INPUT_VOLTAGE
//
#ifndef INPUT_VOLTAGE
# define INPUT_VOLTAGE			4.68	//  4.68 is the average value for a sample set.  This is the value at the processor with 5.02 applied at the servo rail
#endif

//////////////////////////////////////////////////////////////////////////////
//  MAGNETOMETER
#ifndef MAGNETOMETER
# define MAGNETOMETER			ENABLED
#endif

//////////////////////////////////////////////////////////////////////////////
// MODE
// MODE_CHANNEL
//
#ifndef MODE_CHANNEL
# define MODE_CHANNEL	8
#endif
#if (MODE_CHANNEL != 5) && (MODE_CHANNEL != 6) && (MODE_CHANNEL != 7) && (MODE_CHANNEL != 8)
# error XXX
# error XXX You must set MODE_CHANNEL to 5, 6, 7 or 8
# error XXX
#endif

#if !defined(MODE_1)
# define MODE_1			LEARNING
#endif
#if !defined(MODE_2)
# define MODE_2			LEARNING
#endif
#if !defined(MODE_3)
# define MODE_3			LEARNING
#endif
#if !defined(MODE_4)
# define MODE_4			LEARNING
#endif
#if !defined(MODE_5)
# define MODE_5			LEARNING
#endif
#if !defined(MODE_6)
# define MODE_6			MANUAL
#endif


//////////////////////////////////////////////////////////////////////////////
// failsafe defaults
#ifndef THROTTLE_FAILSAFE
# define THROTTLE_FAILSAFE		ENABLED
#endif
#ifndef THROTTLE_FS_VALUE
# define THROTTLE_FS_VALUE		950
#endif
#ifndef LONG_FAILSAFE_ACTION
# define LONG_FAILSAFE_ACTION		0
#endif
#ifndef GCS_HEARTBEAT_FAILSAFE
# define GCS_HEARTBEAT_FAILSAFE		DISABLED
#endif


//////////////////////////////////////////////////////////////////////////////
// THROTTLE_OUT
//
#ifndef THROTTE_OUT
# define THROTTLE_OUT			ENABLED
#endif


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
// STARTUP BEHAVIOUR
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// GROUND_START_DELAY
//
#ifndef GROUND_START_DELAY
# define GROUND_START_DELAY		0
#endif

//////////////////////////////////////////////////////////////////////////////
// MOUNT (ANTENNA OR CAMERA)
//
#ifndef MOUNT
# define MOUNT		ENABLED
#endif

//////////////////////////////////////////////////////////////////////////////
// CAMERA control
//
#ifndef CAMERA
# define CAMERA ENABLED
#endif

//////////////////////////////////////////////////////////////////////////////
// AIRSPEED_CRUISE
//
#ifndef SPEED_CRUISE
# define SPEED_CRUISE		3 // 3 m/s
#endif

#ifndef GSBOOST
# define GSBOOST		0
#endif
#ifndef GSBOOST
# define GSBOOST		0
#endif
#ifndef NUDGE_OFFSET
# define NUDGE_OFFSET		0
#endif

#ifndef E_GLIDER
# define E_GLIDER		ENABLED
#endif

#ifndef TURN_GAIN
# define TURN_GAIN		5
#endif

//////////////////////////////////////////////////////////////////////////////
// Servo Mapping
//
#ifndef THROTTLE_MIN
# define THROTTLE_MIN			0 // percent
#endif
#ifndef THROTTLE_CRUISE
# define THROTTLE_CRUISE		45
#endif
#ifndef THROTTLE_MAX
# define THROTTLE_MAX			100
#endif

//////////////////////////////////////////////////////////////////////////////
// Attitude control gains
//
#ifndef SERVO_STEER_P
# define SERVO_STEER_P         0.4
#endif
#ifndef SERVO_STEER_I
# define SERVO_STEER_I         0.0
#endif
#ifndef SERVO_STEER_D
# define SERVO_STEER_D         0.0
#endif
#ifndef SERVO_STEER_INT_MAX
# define SERVO_STEER_INT_MAX   5
#endif
#define SERVO_STEER_INT_MAX_CENTIDEGREE SERVO_STEER_INT_MAX*100


//////////////////////////////////////////////////////////////////////////////
// Crosstrack compensation
//
#ifndef XTRACK_GAIN
# define XTRACK_GAIN          1 // deg/m
#endif
#ifndef XTRACK_ENTRY_ANGLE
# define XTRACK_ENTRY_ANGLE   50 // deg
#endif
# define XTRACK_GAIN_SCALED XTRACK_GAIN*100
# define XTRACK_ENTRY_ANGLE_CENTIDEGREE XTRACK_ENTRY_ANGLE*100

//////////////////////////////////////////////////////////////////////////////
// Dataflash logging control
//
#ifndef LOGGING_ENABLED
# define LOGGING_ENABLED		ENABLED
#endif

#if CONFIG_HAL_BOARD == HAL_BOARD_APM1 || CONFIG_HAL_BOARD == HAL_BOARD_APM2
#define DEFAULT_LOG_BITMASK     \
    MASK_LOG_ATTITUDE_MED | \
    MASK_LOG_GPS | \
    MASK_LOG_PM | \
    MASK_LOG_CTUN | \
    MASK_LOG_NTUN | \
    MASK_LOG_MODE | \
    MASK_LOG_CMD | \
    MASK_LOG_SONAR | \
    MASK_LOG_COMPASS | \
    MASK_LOG_CURRENT | \
    MASK_LOG_STEERING | \
    MASK_LOG_CAMERA
#else
// other systems have plenty of space for full logs
#define DEFAULT_LOG_BITMASK   0xffff
#endif



//////////////////////////////////////////////////////////////////////////////
// Developer Items
//

#ifndef STANDARD_SPEED
# define STANDARD_SPEED		3.0
#define STANDARD_SPEED_SQUARED (STANDARD_SPEED * STANDARD_SPEED)
#endif
#define STANDARD_THROTTLE_SQUARED (THROTTLE_CRUISE * THROTTLE_CRUISE)

// use this to enable servos in HIL mode
#ifndef HIL_SERVOS
# define HIL_SERVOS DISABLED
#endif

// use this to completely disable the CLI
#ifndef CLI_ENABLED
# define CLI_ENABLED ENABLED
#endif

// if RESET_SWITCH_CH is not zero, then this is the PWM value on
// that channel where we reset the control mode to the current switch
// position (to for example return to switched mode after failsafe or
// fence breach)
#ifndef RESET_SWITCH_CHAN_PWM
# define RESET_SWITCH_CHAN_PWM 1750
#endif

#ifndef BOOSTER
# define BOOSTER              2    // booster factor x1 = 1 or x2 = 2
#endif

#ifndef SONAR_ENABLED
# define SONAR_ENABLED       DISABLED
#endif

/*
  build a firmware version string.
  GIT_VERSION comes from Makefile builds
*/
#ifndef GIT_VERSION
#define FIRMWARE_STRING THISFIRMWARE
#else
#define FIRMWARE_STRING THISFIRMWARE " (" GIT_VERSION ")"
#endif
