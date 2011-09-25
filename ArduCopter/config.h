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

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
// HARDWARE CONFIGURATION AND CONNECTIONS
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// FRAME_CONFIG
//
#ifndef FRAME_CONFIG
# define FRAME_CONFIG		QUAD_FRAME
#endif
#ifndef FRAME_ORIENTATION
# define FRAME_ORIENTATION		PLUS_FRAME
#endif


//////////////////////////////////////////////////////////////////////////////
// Sonar
//

#ifndef SONAR_PORT
# define SONAR_PORT		AP_RANGEFINDER_PITOT_TUBE
#endif

#ifndef SONAR_TYPE
# define SONAR_TYPE		MAX_SONAR_XL
#endif

//////////////////////////////////////////////////////////////////////////////
// Acrobatics
//

#ifndef CH7_OPTION
# define CH7_OPTION		DO_SET_HOVER
#endif


//////////////////////////////////////////////////////////////////////////////
// AIRSPEED_SENSOR
// AIRSPEED_RATIO
//
#ifndef AIRSPEED_SENSOR
# define AIRSPEED_SENSOR		DISABLED
#endif
#ifndef AIRSPEED_RATIO
# define AIRSPEED_RATIO			1.9936		// Note - this varies from the value in ArduPilot due to the difference in ADC resolution
#endif


//////////////////////////////////////////////////////////////////////////////
// HIL_PROTOCOL                             OPTIONAL
// HIL_MODE                                 OPTIONAL
// HIL_PORT                                 OPTIONAL

#ifndef HIL_MODE
#define HIL_MODE	HIL_MODE_DISABLED
#endif

#ifndef HIL_PROTOCOL
#define HIL_PROTOCOL	HIL_PROTOCOL_MAVLINK
#endif

#ifndef HIL_PORT
#define HIL_PORT 0
#endif

#if HIL_MODE != HIL_MODE_DISABLED	// we are in HIL mode

 # undef GPS_PROTOCOL
 # define GPS_PROTOCOL GPS_PROTOCOL_NONE

#endif


// If we are in XPlane, diasble the mag
#if HIL_MODE != HIL_MODE_DISABLED // we are in HIL mode

 // check xplane settings
 #if HIL_PROTOCOL == HIL_PROTOCOL_XPLANE

  // MAGNETOMETER not supported by XPLANE
  # undef MAGNETOMETER
  # define MAGNETOMETER			DISABLED

  # if HIL_MODE != HIL_MODE_ATTITUDE
  #  error HIL_PROTOCOL_XPLANE requires HIL_MODE_ATTITUDE
  # endif

 #endif
#endif


//////////////////////////////////////////////////////////////////////////////
// GPS_PROTOCOL
//
// Note that this test must follow the HIL_PROTOCOL block as the HIL
// setup may override the GPS configuration.
//
#ifndef GPS_PROTOCOL
# define GPS_PROTOCOL 		GPS_PROTOCOL_AUTO
#endif


//////////////////////////////////////////////////////////////////////////////
// GCS_PROTOCOL
// GCS_PORT
//
#ifndef GCS_PROTOCOL
# define GCS_PROTOCOL			GCS_PROTOCOL_MAVLINK
#endif

//Chris: Commenting out assignment of GCS to port 3 because it kills MAVLink on Port 0
#ifndef GCS_PORT
# define GCS_PORT			3
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


#ifndef PIEZO
# define PIEZO				ENABLED				//Enables Piezo Code and beeps once on Startup to verify operation
#endif
#ifndef PIEZO_LOW_VOLTAGE
# define PIEZO_LOW_VOLTAGE	ENABLED				//Enables Tone on reaching low battery or current alert
#endif
#ifndef PIEZO_ARMING
# define PIEZO_ARMING		ENABLED				//Two tones on ARM, 1 Tone on disarm
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
# define MAGNETOMETER			DISABLED
#endif
#ifndef MAG_ORIENTATION
# define MAG_ORIENTATION		AP_COMPASS_COMPONENTS_DOWN_PINS_FORWARD
#endif


//////////////////////////////////////////////////////////////////////////////
//  OPTICAL_FLOW
#if defined( __AVR_ATmega2560__ )  // determines if optical flow code is included
  //#define OPTFLOW_ENABLED
#endif

//#define OPTFLOW_ENABLED

#ifndef OPTFLOW					// sets global enabled/disabled flag for optflow (as seen in CLI)
# define OPTFLOW				DISABLED
#endif
#ifndef OPTFLOW_ORIENTATION
# define OPTFLOW_ORIENTATION 	AP_OPTICALFLOW_ADNS3080_PINS_FORWARD
#endif
#ifndef OPTFLOW_FOV
# define OPTFLOW_FOV 			AP_OPTICALFLOW_ADNS3080_08_FOV
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
# define TOP_BOTTOM_RATIO	0.92
#endif



//////////////////////////////////////////////////////////////////////////////
// Attitude Control
//

// SIMPLE Mode
#ifndef SIMPLE_YAW
# define SIMPLE_YAW 		YAW_HOLD
#endif

#ifndef SIMPLE_RP
# define SIMPLE_RP 			ROLL_PITCH_STABLE
#endif

#ifndef SIMPLE_THR
# define SIMPLE_THR 		THROTTLE_MANUAL
#endif

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
# define RTL_YAW 			YAW_AUTO
#endif

#ifndef RTL_RP
# define RTL_RP 			ROLL_PITCH_AUTO
#endif

#ifndef RTL_THR
# define RTL_THR			THROTTLE_HOLD
#endif




//////////////////////////////////////////////////////////////////////////////
// Attitude Control
//
#ifndef STABILIZE_ROLL_P
# define STABILIZE_ROLL_P 		4.2
#endif
#ifndef STABILIZE_ROLL_I
# define STABILIZE_ROLL_I 		0.001
#endif
#ifndef STABILIZE_ROLL_IMAX
# define STABILIZE_ROLL_IMAX 	0		// degrees
#endif

#ifndef STABILIZE_PITCH_P
# define STABILIZE_PITCH_P		4.2
#endif
#ifndef STABILIZE_PITCH_I
# define STABILIZE_PITCH_I		0.001
#endif
#ifndef STABILIZE_PITCH_IMAX
# define STABILIZE_PITCH_IMAX	0		// degrees
#endif

//////////////////////////////////////////////////////////////////////////////
// Rate Control
//
#ifndef RATE_ROLL_P
# define RATE_ROLL_P         0.14
#endif
#ifndef RATE_ROLL_I
# define RATE_ROLL_I         0 //0.18
#endif
#ifndef RATE_ROLL_IMAX
# define RATE_ROLL_IMAX	 	15			// degrees
#endif

#ifndef RATE_PITCH_P
# define RATE_PITCH_P       0.14
#endif
#ifndef RATE_PITCH_I
# define RATE_PITCH_I		0 //0.18
#endif
#ifndef RATE_PITCH_IMAX
# define RATE_PITCH_IMAX   	15			// degrees
#endif

//////////////////////////////////////////////////////////////////////////////
// YAW Control
//
#ifndef  STABILIZE_YAW_P
# define STABILIZE_YAW_P		7			// increase for more aggressive Yaw Hold, decrease if it's bouncy
#endif
#ifndef  STABILIZE_YAW_I
# define STABILIZE_YAW_I		0.01		// set to .0001 to try and get over user's steady state error caused by poor balance
#endif
#ifndef  STABILIZE_YAW_IMAX
# define STABILIZE_YAW_IMAX		8			// degrees * 100
#endif

#ifndef RATE_YAW_P
# define RATE_YAW_P     .13			// used to control response in turning
#endif
#ifndef RATE_YAW_I
# define RATE_YAW_I     0.0
#endif
#ifndef RATE_YAW_IMAX
# define RATE_YAW_IMAX   50
#endif


//////////////////////////////////////////////////////////////////////////////
// Autopilot control limits
//
// how much to we pitch towards the target
#ifndef PITCH_MAX
# define PITCH_MAX				22			// degrees
#endif


//////////////////////////////////////////////////////////////////////////////
// Navigation control gains
//
#ifndef LOITER_P
# define LOITER_P			.4		//
#endif
#ifndef LOITER_I
# define LOITER_I			0.01	//
#endif
#ifndef LOITER_IMAX
# define LOITER_IMAX		12		// degrees°
#endif

#ifndef NAV_P
# define NAV_P				2.0			// for 4.5 ms error = 13.5 pitch
#endif
#ifndef NAV_I
# define NAV_I				0.15		// this
#endif
#ifndef NAV_IMAX
# define NAV_IMAX			20			// degrees
#endif

#ifndef WAYPOINT_SPEED_MAX
# define WAYPOINT_SPEED_MAX			400			// for 6m/s error = 13mph
#endif


//////////////////////////////////////////////////////////////////////////////
// Throttle control gains
//


#ifndef THROTTLE_CRUISE
# define THROTTLE_CRUISE	350			//
#endif

#ifndef THROTTLE_P
# define THROTTLE_P		0.6			//
#endif
#ifndef THROTTLE_I
# define THROTTLE_I		0.02		// with 4m error, 8 PWM gain/s
#endif
#ifndef THROTTLE_IMAX
# define THROTTLE_IMAX		300
#endif


//////////////////////////////////////////////////////////////////////////////
// Crosstrack compensation
//
#ifndef XTRACK_ENTRY_ANGLE
# define XTRACK_ENTRY_ANGLE   30 // deg
#endif

#ifndef XTRACK_P
# define XTRACK_P		2			// trying a lower val
#endif
#ifndef XTRACK_I
# define XTRACK_I		0.00		//with 4m error, 12.5s windup
#endif
#ifndef XTRACK_D
# define XTRACK_D		0.00		// upped with filter
#endif
#ifndef XTRACK_IMAX
# define XTRACK_IMAX		10
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

#ifndef LOGGING_ENABLED
# define LOGGING_ENABLED               ENABLED
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
// guess!
#ifndef LOG_OPTFLOW
# define LOG_OPTFLOW				DISABLED
#endif

// calculate the default log_bitmask
#define LOGBIT(_s)     (LOG_##_s ? MASK_LOG_##_s : 0)

#define DEFAULT_LOG_BITMASK \
               LOGBIT(ATTITUDE_FAST)   | \
               LOGBIT(ATTITUDE_MED)    | \
               LOGBIT(GPS)                             | \
               LOGBIT(PM)                              | \
               LOGBIT(CTUN)                    | \
               LOGBIT(NTUN)                    | \
               LOGBIT(MODE)                    | \
               LOGBIT(RAW)                             | \
               LOGBIT(CMD)                             | \
               LOGBIT(CUR)						| \
               LOGBIT(MOTORS)					| \
               LOGBIT(OPTFLOW)

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
# define WP_RADIUS_DEFAULT		3
#endif

#ifndef LOITER_RADIUS
# define LOITER_RADIUS 10
#endif

#ifndef ALT_HOLD_HOME
# define ALT_HOLD_HOME -1
#endif

#ifndef USE_CURRENT_ALT
# define USE_CURRENT_ALT FALSE
#endif


#ifndef AUTO_RESET_LOITER
# define AUTO_RESET_LOITER	1	// enables Loiter to reset it's current location based on stick input.
#endif
#ifndef CUT_MOTORS
# define CUT_MOTORS		1		// do we cut the motors with no throttle?
#endif

#ifndef BROKEN_SLIDER
# define BROKEN_SLIDER		0		// 1 = yes (use Yaw to enter CLI mode)
#endif

#ifndef MOTOR_LEDS
# define MOTOR_LEDS		1		// 0 = off, 1 = on
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

// delay to prevent Xbee bricking, in milliseconds
#ifndef MAVLINK_TELEMETRY_PORT_DELAY
# define MAVLINK_TELEMETRY_PORT_DELAY 2000
#endif
