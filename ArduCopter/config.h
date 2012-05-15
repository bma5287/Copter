// -*- tab-width: 4; Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*-
//
#ifndef __ARDUCOPTER_CONFIG_H__
#define __ARDUCOPTER_CONFIG_H__
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
#ifdef USE_CMAKE_APM_CONFIG
#include "APM_Config_cmake.h"  // <== Prefer cmake config if it exists
#else
#include "APM_Config.h"  // <== THIS INCLUDE, DO NOT EDIT IT. EVER.
#endif


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
// HARDWARE CONFIGURATION AND CONNECTIONS
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// APM HARDWARE
//

#ifndef CONFIG_APM_HARDWARE
# define CONFIG_APM_HARDWARE APM_HARDWARE_APM1
#endif

//////////////////////////////////////////////////////////////////////////////
// APM2 HARDWARE DEFAULTS
//

#if CONFIG_APM_HARDWARE == APM_HARDWARE_APM2
# define CONFIG_IMU_TYPE   CONFIG_IMU_MPU6000
# define CONFIG_PUSHBUTTON DISABLED
# define CONFIG_RELAY      DISABLED
# define MAG_ORIENTATION   AP_COMPASS_APM2_SHIELD
# define CONFIG_SONAR_SOURCE SONAR_SOURCE_ANALOG_PIN
# define MAGNETOMETER ENABLED
# ifdef APM2_BETA_HARDWARE
#  define CONFIG_BARO     AP_BARO_BMP085
# else // APM2 Production Hardware (default)
#  define CONFIG_BARO     AP_BARO_MS5611
# endif
#endif


//////////////////////////////////////////////////////////////////////////////
// FRAME_CONFIG
//
#ifndef FRAME_CONFIG
# define FRAME_CONFIG	QUAD_FRAME
#endif
#ifndef FRAME_ORIENTATION
# define FRAME_ORIENTATION	X_FRAME
#endif

//////////////////////////////////////////////////////////////////////////////
// IMU Selection
//
#ifndef CONFIG_IMU_TYPE
# define CONFIG_IMU_TYPE CONFIG_IMU_OILPAN
#endif

#if CONFIG_IMU_TYPE == CONFIG_IMU_MPU6000
# ifndef CONFIG_MPU6000_CHIP_SELECT_PIN
#  define CONFIG_MPU6000_CHIP_SELECT_PIN 53
# endif
#endif


//////////////////////////////////////////////////////////////////////////////
// ADC Enable - used to eliminate for systems which don't have ADC.
//
#ifndef CONFIG_ADC
# if CONFIG_IMU_TYPE == CONFIG_IMU_OILPAN
#   define CONFIG_ADC ENABLED
# else
#   define CONFIG_ADC DISABLED
# endif
#endif

//////////////////////////////////////////////////////////////////////////////
// PWM control
//
#ifndef INSTANT_PWM
# define INSTANT_PWM	DISABLED
#endif

// default RC speed in Hz if INSTANT_PWM is not used
#ifndef RC_FAST_SPEED
# if FRAME_CONFIG == HELI_FRAME
#   define RC_FAST_SPEED 125
# else
#   define RC_FAST_SPEED 490
# endif
#endif

////////////////////////////////////////////////////////
// LED and IO Pins
//
#if CONFIG_APM_HARDWARE == APM_HARDWARE_APM1
# define A_LED_PIN        37
# define B_LED_PIN        36
# define C_LED_PIN        35
# define LED_ON           HIGH
# define LED_OFF          LOW
# define SLIDE_SWITCH_PIN 40
# define PUSHBUTTON_PIN   41
# define USB_MUX_PIN      -1
# define CLI_SLIDER_ENABLED DISABLED
# define OPTFLOW_CS_PIN   34
# define BATTERY_PIN_1      0
# define CURRENT_PIN_1      1
#elif CONFIG_APM_HARDWARE == APM_HARDWARE_APM2
# define A_LED_PIN        27
# define B_LED_PIN        26
# define C_LED_PIN        25
# define LED_ON           LOW
# define LED_OFF          HIGH
# define SLIDE_SWITCH_PIN (-1)
# define PUSHBUTTON_PIN   (-1)
# define CLI_SLIDER_ENABLED DISABLED
# define USB_MUX_PIN      23
# define OPTFLOW_CS_PIN   A3
# define BATTERY_PIN_1      1
# define CURRENT_PIN_1      2
#endif

////////////////////////////////////////////////////////////////////////////////
// CopterLEDs
//

#ifndef COPTER_LEDS
#define COPTER_LEDS ENABLED
#endif

#define COPTER_LED_ON		HIGH
#define COPTER_LED_OFF		LOW

#if CONFIG_APM_HARDWARE == APM_HARDWARE_APM2
#define COPTER_LED_1 AN4  	// Motor or Aux LED
#define COPTER_LED_2 AN5  	// Motor LED or Beeper
#define COPTER_LED_3 AN6 	// Motor or GPS LED
#define COPTER_LED_4 AN7 	// Motor LED
#define COPTER_LED_5 AN8	// Motor LED
#define COPTER_LED_6 AN9 	// Motor LED
#define COPTER_LED_7 AN10 	// Motor LED
#define COPTER_LED_8 AN11 	// Motor LED
#elif CONFIG_APM_HARDWARE == APM_HARDWARE_APM1
#define COPTER_LED_1 AN8  	// Motor or Aux LED
#define COPTER_LED_2 AN9  	// Motor LED
#define COPTER_LED_3 AN10 	// Motor or GPS LED
#define COPTER_LED_4 AN11 	// Motor LED
#define COPTER_LED_5 AN12	// Motor LED
#define COPTER_LED_6 AN13 	// Motor LED
#define COPTER_LED_7 AN14 	// Motor LED
#define COPTER_LED_8 AN15 	// Motor LED
#endif


//////////////////////////////////////////////////////////////////////////////
// Pushbutton & Relay
//

#ifndef CONFIG_PUSHBUTTON
# define CONFIG_PUSHBUTTON ENABLED
#endif

#ifndef CONFIG_RELAY
# define CONFIG_RELAY ENABLED
#endif

//////////////////////////////////////////////////////////////////////////////
// Barometer
//

#ifndef CONFIG_BARO
# define CONFIG_BARO AP_BARO_BMP085
#endif

//////////////////////////////////////////////////////////////////////////////
// Sonar
//

#ifndef CONFIG_SONAR_SOURCE
# define CONFIG_SONAR_SOURCE SONAR_SOURCE_ADC
#endif

#if CONFIG_SONAR_SOURCE == SONAR_SOURCE_ADC && CONFIG_ADC == DISABLED
# warning Cannot use ADC for CONFIG_SONAR_SOURCE, becaude CONFIG_ADC is DISABLED
# warning Defaulting CONFIG_SONAR_SOURCE to ANALOG_PIN
# undef CONFIG_SONAR_SOURCE
# define CONFIG_SONAR_SOURCE SONAR_SOURCE_ANALOG_PIN
#endif

#if CONFIG_SONAR_SOURCE == SONAR_SOURCE_ADC
# ifndef CONFIG_SONAR_SOURCE_ADC_CHANNEL
#  define CONFIG_SONAR_SOURCE_ADC_CHANNEL 7
# endif
#elif CONFIG_SONAR_SOURCE == SONAR_SOURCE_ANALOG_PIN
# ifndef CONFIG_SONAR_SOURCE_ANALOG_PIN
#  define CONFIG_SONAR_SOURCE_ANALOG_PIN A0
# endif
#else
# warning Invalid value for CONFIG_SONAR_SOURCE, disabling sonar
# undef SONAR_ENABLED
# define SONAR_ENABLED DISABLED
#endif

#ifndef CONFIG_SONAR
# define CONFIG_SONAR ENABLED
#endif

//////////////////////////////////////////////////////////////////////////////
// Channel Config (custom MOT channel mappings)
//

#ifndef CONFIG_CHANNELS
# define CONFIG_CHANNELS CHANNEL_CONFIG_DEFAULT
#endif

//////////////////////////////////////////////////////////////////////////////
// Acrobatics
//

#ifndef CH7_OPTION
# define CH7_OPTION		CH7_SAVE_WP
#endif


//////////////////////////////////////////////////////////////////////////////
// HIL_MODE                                 OPTIONAL

#ifndef HIL_MODE
#define HIL_MODE	HIL_MODE_DISABLED
#endif

#if HIL_MODE != HIL_MODE_DISABLED	// we are in HIL mode

 # undef GPS_PROTOCOL
 # define GPS_PROTOCOL GPS_PROTOCOL_NONE

 #undef CONFIG_SONAR
 #define CONFIG_SONAR DISABLED
#endif


//////////////////////////////////////////////////////////////////////////////
// GPS_PROTOCOL
//
#ifndef GPS_PROTOCOL
# define GPS_PROTOCOL 		GPS_PROTOCOL_AUTO
#endif


#ifndef MAV_SYSTEM_ID
# define MAV_SYSTEM_ID		1
#endif


//////////////////////////////////////////////////////////////////////////////
// Serial port speeds.
//
#ifndef SERIAL0_BAUD
# define SERIAL0_BAUD			115200
#endif
#ifndef SERIAL3_BAUD
# define SERIAL3_BAUD			 57600
#endif


//////////////////////////////////////////////////////////////////////////////
// Battery monitoring
//
#ifndef BATTERY_EVENT
# define BATTERY_EVENT			DISABLED
#endif
#ifndef LOW_VOLTAGE
# define LOW_VOLTAGE			9.6
#endif
#ifndef VOLT_DIV_RATIO
# define VOLT_DIV_RATIO			3.56
#endif

#ifndef CURR_AMP_PER_VOLT
# define CURR_AMP_PER_VOLT		27.32
#endif
#ifndef CURR_AMPS_OFFSET
# define CURR_AMPS_OFFSET		0.0
#endif
#ifndef HIGH_DISCHARGE
# define HIGH_DISCHARGE			1760
#endif





//////////////////////////////////////////////////////////////////////////////
// INPUT_VOLTAGE
//
#ifndef INPUT_VOLTAGE
# define INPUT_VOLTAGE			5.0
#endif


//////////////////////////////////////////////////////////////////////////////
//  MAGNETOMETER
#ifndef MAGNETOMETER
# define MAGNETOMETER			ENABLED
#endif
#ifndef MAG_ORIENTATION
# define MAG_ORIENTATION		AP_COMPASS_COMPONENTS_DOWN_PINS_FORWARD
#endif


//////////////////////////////////////////////////////////////////////////////
//  CAMERA GAINS
#ifndef CAM_ROLL_GAIN
# define CAM_ROLL_GAIN			1.0
#endif
#ifndef CAM_PITCH_GAIN
# define CAM_PITCH_GAIN			1.0
#endif


//////////////////////////////////////////////////////////////////////////////
//  OPTICAL_FLOW
#if defined( __AVR_ATmega2560__ )  // determines if optical flow code is included
  #define OPTFLOW_ENABLED
#endif
#ifndef OPTFLOW					// sets global enabled/disabled flag for optflow (as seen in CLI)
# define OPTFLOW				DISABLED
#endif
#ifndef OPTFLOW_ORIENTATION
# define OPTFLOW_ORIENTATION 	AP_OPTICALFLOW_ADNS3080_PINS_FORWARD
#endif
#ifndef OPTFLOW_RESOLUTION
# define OPTFLOW_RESOLUTION 	ADNS3080_RESOLUTION_1600
#endif
#ifndef OPTFLOW_FOV
# define OPTFLOW_FOV 			AP_OPTICALFLOW_ADNS3080_08_FOV
#endif
// optical flow based loiter PI values
#ifndef OPTFLOW_ROLL_P
  #define OPTFLOW_ROLL_P 2.5
#endif
#ifndef OPTFLOW_ROLL_I
  #define OPTFLOW_ROLL_I 3.2
#endif
#ifndef OPTFLOW_ROLL_D
  #define OPTFLOW_ROLL_D 0.12
#endif
#ifndef OPTFLOW_PITCH_P
  #define OPTFLOW_PITCH_P 2.5
#endif
#ifndef OPTFLOW_PITCH_I
  #define OPTFLOW_PITCH_I 3.2
#endif
#ifndef OPTFLOW_PITCH_D
  #define OPTFLOW_PITCH_D 0.12
#endif
#ifndef OPTFLOW_IMAX
  #define OPTFLOW_IMAX 4
#endif


//////////////////////////////////////////////////////////////////////////////
// RADIO CONFIGURATION
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// FLIGHT_MODE
//

#if !defined(FLIGHT_MODE_1)
# define FLIGHT_MODE_1			STABILIZE
#endif
#if !defined(FLIGHT_MODE_2)
# define FLIGHT_MODE_2			STABILIZE
#endif
#if !defined(FLIGHT_MODE_3)
# define FLIGHT_MODE_3			STABILIZE
#endif
#if !defined(FLIGHT_MODE_4)
# define FLIGHT_MODE_4			STABILIZE
#endif
#if !defined(FLIGHT_MODE_5)
# define FLIGHT_MODE_5			STABILIZE
#endif
#if !defined(FLIGHT_MODE_6)
# define FLIGHT_MODE_6			STABILIZE
#endif


//////////////////////////////////////////////////////////////////////////////
// THROTTLE_FAILSAFE
// THROTTLE_FS_VALUE
// THROTTLE_FAILSAFE_ACTION
//
#ifndef THROTTLE_FAILSAFE
# define THROTTLE_FAILSAFE			DISABLED
#endif
#ifndef THROTTE_FS_VALUE
# define THROTTLE_FS_VALUE			975
#endif
#ifndef THROTTLE_FAILSAFE_ACTION
# define THROTTLE_FAILSAFE_ACTION	2
#endif
#ifndef MINIMUM_THROTTLE
# define MINIMUM_THROTTLE	130
#endif
#ifndef MAXIMUM_THROTTLE
# define MAXIMUM_THROTTLE	850
#endif

#ifndef AUTO_LAND_TIME
# define AUTO_LAND_TIME	20
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
# define GROUND_START_DELAY		3
#endif

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
// FLIGHT AND NAVIGATION CONTROL
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// Y6 Support

#ifndef TOP_BOTTOM_RATIO
# define TOP_BOTTOM_RATIO	1.00
#endif


//////////////////////////////////////////////////////////////////////////////
// Attitude Control
//

// Alt Hold Mode
#ifndef ALT_HOLD_YAW
# define ALT_HOLD_YAW 		YAW_HOLD
#endif

#ifndef ALT_HOLD_RP
# define ALT_HOLD_RP 		ROLL_PITCH_STABLE
#endif

#ifndef ALT_HOLD_THR
# define ALT_HOLD_THR		THROTTLE_HOLD
#endif

// AUTO Mode
#ifndef AUTO_YAW
# define AUTO_YAW 			YAW_AUTO
#endif

#ifndef AUTO_RP
# define AUTO_RP 			ROLL_PITCH_AUTO
#endif

#ifndef AUTO_THR
# define AUTO_THR			THROTTLE_AUTO
#endif

// CIRCLE Mode
#ifndef CIRCLE_YAW
# define CIRCLE_YAW 		YAW_AUTO
#endif

#ifndef CIRCLE_RP
# define CIRCLE_RP 			ROLL_PITCH_AUTO
#endif

#ifndef CIRCLE_THR
# define CIRCLE_THR			THROTTLE_HOLD
#endif

// LOITER Mode
#ifndef LOITER_YAW
# define LOITER_YAW 		YAW_HOLD
#endif

#ifndef LOITER_RP
# define LOITER_RP 			ROLL_PITCH_AUTO
#endif

#ifndef LOITER_THR
# define LOITER_THR			THROTTLE_HOLD
#endif


// RTL Mode
#ifndef RTL_YAW
# define RTL_YAW 			YAW_HOLD
#endif

#ifndef RTL_RP
# define RTL_RP 			ROLL_PITCH_AUTO
#endif

#ifndef RTL_THR
# define RTL_THR			THROTTLE_HOLD
#endif

#ifndef SUPER_SIMPLE
# define SUPER_SIMPLE		DISABLED
#endif

// RTL Mode
#ifndef RTL_AUTO_LAND
# define RTL_AUTO_LAND 		ENABLED
#endif

// RTL Approach Delay in seconds
#ifndef RTL_APPROACH_DELAY
# define RTL_APPROACH_DELAY	20
#endif


// LOITER Mode
#ifndef OF_LOITER_YAW
# define OF_LOITER_YAW 		YAW_HOLD
#endif

#ifndef OF_LOITER_RP
# define OF_LOITER_RP 			ROLL_PITCH_STABLE_OF
#endif

#ifndef OF_LOITER_THR
# define OF_LOITER_THR			THROTTLE_HOLD
#endif

//////////////////////////////////////////////////////////////////////////////
// Attitude Control
//

// Extra motor values that are changed from time to time by jani @ jDrones as software
// and charachteristics changes.
#ifdef MOTORS_JD880
# define STABILIZE_ROLL_P 		3.7
# define STABILIZE_ROLL_I 		0.0
# define STABILIZE_ROLL_IMAX	40.0		// degrees
# define STABILIZE_PITCH_P		3.7
# define STABILIZE_PITCH_I		0.0
# define STABILIZE_PITCH_IMAX	40.0		// degrees
#endif

#ifdef MOTORS_JD850
# define STABILIZE_ROLL_P 		4.2
# define STABILIZE_ROLL_I 		0.0
# define STABILIZE_ROLL_IMAX 	40.0		// degrees
# define STABILIZE_PITCH_P		4.2
# define STABILIZE_PITCH_I		0.0
# define STABILIZE_PITCH_IMAX	40.0		// degrees
#endif


#ifndef ACRO_P
# define ACRO_P 		4.5
#endif


#ifndef AXIS_LOCK_ENABLED
# define AXIS_LOCK_ENABLED 	DISABLED
#endif

#ifndef AXIS_LOCK_P
# define AXIS_LOCK_P 		.02
#endif


// Good for smaller payload motors.
#ifndef STABILIZE_ROLL_P
# define STABILIZE_ROLL_P 		4.5
#endif
#ifndef STABILIZE_ROLL_I
# define STABILIZE_ROLL_I 		0.1
#endif
#ifndef STABILIZE_ROLL_IMAX
# define STABILIZE_ROLL_IMAX 	40		// degrees
#endif

#ifndef STABILIZE_PITCH_P
# define STABILIZE_PITCH_P		4.5
#endif
#ifndef STABILIZE_PITCH_I
# define STABILIZE_PITCH_I		0.1
#endif
#ifndef STABILIZE_PITCH_IMAX
# define STABILIZE_PITCH_IMAX	40		// degrees
#endif

#ifndef  STABILIZE_YAW_P
# define STABILIZE_YAW_P		7.0		// increase for more aggressive Yaw Hold, decrease if it's bouncy
#endif
#ifndef  STABILIZE_YAW_I
# define STABILIZE_YAW_I		0.01
#endif
#ifndef  STABILIZE_YAW_IMAX
# define STABILIZE_YAW_IMAX		8		// degrees * 100
#endif


//////////////////////////////////////////////////////////////////////////////
// Stabilize Rate Control
//
#ifndef RATE_ROLL_P
# define RATE_ROLL_P        0.14
#endif
#ifndef RATE_ROLL_I
# define RATE_ROLL_I        0.0
#endif
#ifndef RATE_ROLL_D
# define RATE_ROLL_D        0.000 //.002
#endif
#ifndef RATE_ROLL_IMAX
# define RATE_ROLL_IMAX	 	5			// degrees
#endif

#ifndef RATE_PITCH_P
# define RATE_PITCH_P       0.14
#endif
#ifndef RATE_PITCH_I
# define RATE_PITCH_I		0.0
#endif
#ifndef RATE_PITCH_D
# define RATE_PITCH_D       0.00
#endif
#ifndef RATE_PITCH_IMAX
# define RATE_PITCH_IMAX   	5			// degrees
#endif

#ifndef RATE_YAW_P
# define RATE_YAW_P    		 .13
#endif
#ifndef RATE_YAW_I
# define RATE_YAW_I    		 0.0
#endif
#ifndef RATE_YAW_D
# define RATE_YAW_D    		 0.000
#endif
#ifndef RATE_YAW_IMAX
# define RATE_YAW_IMAX 		  50		// degrees
#endif


#ifndef STABILIZE_D
# define STABILIZE_D 		0.15
#endif

#ifndef STABILIZE_D_SCHEDULE
# define STABILIZE_D_SCHEDULE 		0.5
#endif

//////////////////////////////////////////////////////////////////////////////
// Loiter control gains
//
#ifndef LOITER_P
# define LOITER_P			.35
#endif
#ifndef LOITER_I
# define LOITER_I			0.0
#endif
#ifndef LOITER_IMAX
# define LOITER_IMAX		30		// degrees
#endif

//////////////////////////////////////////////////////////////////////////////
// Loiter Navigation control gains
//
#ifndef LOITER_RATE_P
# define LOITER_RATE_P		2.0			//
#endif
#ifndef LOITER_RATE_I
# define LOITER_RATE_I		0.2		// Wind control
#endif
#ifndef LOITER_RATE_D
# define LOITER_RATE_D		0			// try 2 or 3 for LOITER_RATE 1
#endif
#ifndef LOITER_RATE_IMAX
# define LOITER_RATE_IMAX	30			// degrees
#endif

//////////////////////////////////////////////////////////////////////////////
// WP Navigation control gains
//
#ifndef NAV_P
# define NAV_P				3.0			//
#endif
#ifndef NAV_I
# define NAV_I				0.20		// Wind control
#endif
#ifndef NAV_D
# define NAV_D				0.00		//
#endif
#ifndef NAV_IMAX
# define NAV_IMAX			30			// degrees
#endif

#ifndef AUTO_SLEW_RATE
# define AUTO_SLEW_RATE		30			// degrees
#endif


#ifndef WAYPOINT_SPEED_MAX
# define WAYPOINT_SPEED_MAX		600			// 6m/s error = 13mph
#endif

#ifndef WAYPOINT_SPEED_MIN
# define WAYPOINT_SPEED_MIN		100			// 1m/s
#endif


//////////////////////////////////////////////////////////////////////////////
// Throttle control gains
//
#ifndef AUTO_THROTTLE_HOLD
# define AUTO_THROTTLE_HOLD 1
#endif

#ifndef THROTTLE_CRUISE
# define THROTTLE_CRUISE	450		//
#endif

#ifndef ALT_HOLD_P
# define ALT_HOLD_P			0.5		//
#endif
#ifndef ALT_HOLD_I
# define ALT_HOLD_I			0.015
#endif
#ifndef ALT_HOLD_IMAX
# define ALT_HOLD_IMAX		300
#endif

// RATE control
#ifndef THROTTLE_P
# define THROTTLE_P			0.25	//
#endif
#ifndef THROTTLE_I
# define THROTTLE_I			0.0		// Don't edit
#endif
#ifndef THROTTLE_D
# define THROTTLE_D			0.02	//
#endif
#ifndef THROTTLE_IMAX
# define THROTTLE_IMAX		300
#endif


//////////////////////////////////////////////////////////////////////////////
// Crosstrack compensation
//
#ifndef CROSSTRACK_GAIN
# define CROSSTRACK_GAIN		1
#endif


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
// DEBUGGING
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
// DEBUG_LEVEL
//
#ifndef DEBUG_LEVEL
# define DEBUG_LEVEL SEVERITY_LOW
#endif

//////////////////////////////////////////////////////////////////////////////
// Dataflash logging control
//
// Logging must be disabled for 1280 build.
#if defined( __AVR_ATmega1280__ )
# if LOGGING_ENABLED == ENABLED
// If logging was enabled in APM_Config or command line, warn the user.
#  warning "Logging is not supported on ATmega1280"
#  undef LOGGING_ENABLED
# endif
# ifndef LOGGING_ENABLED
#  define LOGGING_ENABLED    DISABLED
# endif
#elif !defined(LOGGING_ENABLED)
// Logging is enabled by default for all other builds.
# define LOGGING_ENABLED		ENABLED
#endif


#ifndef LOG_ATTITUDE_FAST
# define LOG_ATTITUDE_FAST		DISABLED
#endif
#ifndef LOG_ATTITUDE_MED
# define LOG_ATTITUDE_MED 		ENABLED
#endif
#ifndef LOG_GPS
# define LOG_GPS 				ENABLED
#endif
#ifndef LOG_PM
# define LOG_PM 				ENABLED
#endif
#ifndef LOG_CTUN
# define LOG_CTUN				ENABLED
#endif
#ifndef LOG_NTUN
# define LOG_NTUN				ENABLED
#endif
#ifndef LOG_MODE
# define LOG_MODE				ENABLED
#endif
#ifndef LOG_RAW
# define LOG_RAW				DISABLED
#endif
#ifndef LOG_CMD
# define LOG_CMD				ENABLED
#endif
// current
#ifndef LOG_CUR
# define LOG_CUR				DISABLED
#endif
// quad motor PWMs
#ifndef LOG_MOTORS
# define LOG_MOTORS				DISABLED
#endif
// optical flow
#ifndef LOG_OPTFLOW
# define LOG_OPTFLOW			DISABLED
#endif
#ifndef LOG_PID
# define LOG_PID				DISABLED
#endif

// calculate the default log_bitmask
#define LOGBIT(_s)     (LOG_##_s ? MASK_LOG_##_s : 0)

#define DEFAULT_LOG_BITMASK \
			LOGBIT(ATTITUDE_FAST)	| \
			LOGBIT(ATTITUDE_MED)	| \
			LOGBIT(GPS)				| \
			LOGBIT(PM)				| \
			LOGBIT(CTUN)			| \
			LOGBIT(NTUN)			| \
			LOGBIT(MODE)			| \
			LOGBIT(RAW)				| \
			LOGBIT(CMD)				| \
			LOGBIT(CUR)				| \
			LOGBIT(MOTORS)			| \
			LOGBIT(OPTFLOW)			| \
			LOGBIT(PID)

// if we are using fast, Disable Medium
//#if LOG_ATTITUDE_FAST == ENABLED
//	#undef LOG_ATTITUDE_MED
//	#define LOG_ATTITUDE_MED 		DISABLED
//#endif

#ifndef DEBUG_PORT
# define DEBUG_PORT 0
#endif

#if DEBUG_PORT == 0
# define SendDebug_P(a) Serial.print_P(PSTR(a))
# define SendDebugln_P(a) Serial.println_P(PSTR(a))
# define SendDebug Serial.print
# define SendDebugln Serial.println
#elif DEBUG_PORT == 1
# define SendDebug_P(a) Serial1.print_P(PSTR(a))
# define SendDebugln_P(a) Serial1.println_P(PSTR(a))
# define SendDebug Serial1.print
# define SendDebugln Serial1.println
#elif DEBUG_PORT == 2
# define SendDebug_P(a) Serial2.print_P(PSTR(a))
# define SendDebugln_P(a) Serial2.println_P(PSTR(a))
# define SendDebug Serial2.print
# define SendDebugln Serial2.println
#elif DEBUG_PORT == 3
# define SendDebug_P(a) Serial3.print_P(PSTR(a))
# define SendDebugln_P(a) Serial3.println_P(PSTR(a))
# define SendDebug Serial3.print
# define SendDebugln Serial3.println
#endif

//////////////////////////////////////////////////////////////////////////////
// Navigation defaults
//
#ifndef WP_RADIUS_DEFAULT
# define WP_RADIUS_DEFAULT	1
#endif

#ifndef LOITER_RADIUS
# define LOITER_RADIUS 10		// meters for circle mode
#endif

#ifndef ALT_HOLD_HOME
# define ALT_HOLD_HOME 0		// height to return to Home, 0 = Maintain current altitude
#endif

#ifndef USE_CURRENT_ALT
# define USE_CURRENT_ALT FALSE
#endif

#ifndef CUT_MOTORS
# define CUT_MOTORS		1		// do we cut the motors with no throttle?
#endif

//////////////////////////////////////////////////////////////////////////////
// RC override
//
#ifndef ALLOW_RC_OVERRIDE
# define ALLOW_RC_OVERRIDE DISABLED
#endif

//////////////////////////////////////////////////////////////////////////////
// Developer Items
//

// use this to completely disable the CLI
#ifndef CLI_ENABLED
# define CLI_ENABLED ENABLED
#endif

// use this to disable the CLI slider switch
#ifndef CLI_SLIDER_ENABLED
# define CLI_SLIDER_ENABLED ENABLED
#endif

// delay to prevent Xbee bricking, in milliseconds
#ifndef MAVLINK_TELEMETRY_PORT_DELAY
# define MAVLINK_TELEMETRY_PORT_DELAY 2000
#endif


// experimental quaternion code
#ifndef QUATERNION_ENABLE
# define QUATERNION_ENABLE DISABLED
#endif


#ifndef RETRO_LOITER_MODE
# define RETRO_LOITER_MODE DISABLED
#endif

#endif // __ARDUCOPTER_CONFIG_H__
