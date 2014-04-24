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
 #include "APM_Config.h" // <== THIS INCLUDE, DO NOT EDIT IT. EVER.
#endif


//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
// HARDWARE CONFIGURATION AND CONNECTIONS
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

#ifdef CONFIG_APM_HARDWARE
#error CONFIG_APM_HARDWARE option is deprecated! use CONFIG_HAL_BOARD instead
#endif

#ifndef CONFIG_HAL_BOARD
#error CONFIG_HAL_BOARD must be defined to build ArduCopter
#endif

#ifdef __AVR_ATmega1280__
#error ATmega1280 is not supported
#endif

//////////////////////////////////////////////////////////////////////////////
// FLIGHT CONTROLLER HARDWARE DEFAULT SETTINGS
//
#if CONFIG_HAL_BOARD == HAL_BOARD_APM2
 # define CONFIG_IMU_TYPE   CONFIG_IMU_MPU6000
 # define CONFIG_SONAR_SOURCE SONAR_SOURCE_ANALOG_PIN
 # define MAGNETOMETER ENABLED
 # ifdef APM2_BETA_HARDWARE
  #  define CONFIG_BARO     AP_BARO_BMP085
 # else // APM2 Production Hardware (default)
  #  define CONFIG_BARO          AP_BARO_MS5611
  #  define CONFIG_MS5611_SERIAL AP_BARO_MS5611_SPI
 # endif
#elif CONFIG_HAL_BOARD == HAL_BOARD_AVR_SITL
 # define CONFIG_IMU_TYPE   CONFIG_IMU_SITL
 # define CONFIG_SONAR_SOURCE SONAR_SOURCE_ANALOG_PIN
 # define MAGNETOMETER ENABLED
 # define OPTFLOW DISABLED
#elif CONFIG_HAL_BOARD == HAL_BOARD_PX4
 # define CONFIG_IMU_TYPE   CONFIG_IMU_PX4
 # define CONFIG_BARO       AP_BARO_PX4
 # define CONFIG_SONAR_SOURCE SONAR_SOURCE_ANALOG_PIN
 # define MAGNETOMETER ENABLED
 # define OPTFLOW DISABLED
#elif CONFIG_HAL_BOARD == HAL_BOARD_FLYMAPLE
 # define CONFIG_IMU_TYPE CONFIG_IMU_FLYMAPLE
 # define CONFIG_BARO AP_BARO_BMP085
 # define CONFIG_COMPASS  AP_COMPASS_HMC5843
 # define CONFIG_ADC        DISABLED
 # define MAGNETOMETER ENABLED
 # define CONFIG_SONAR_SOURCE SONAR_SOURCE_ANALOG_PIN
 # define OPTFLOW DISABLED
#elif CONFIG_HAL_BOARD == HAL_BOARD_LINUX
 # define CONFIG_IMU_TYPE CONFIG_IMU_L3G4200D
 # define CONFIG_BARO AP_BARO_BMP085
 # define CONFIG_COMPASS  AP_COMPASS_HMC5843
 # define CONFIG_ADC        DISABLED
 # define MAGNETOMETER ENABLED
 # define CONFIG_SONAR_SOURCE SONAR_SOURCE_ANALOG_PIN
 # define OPTFLOW DISABLED
#elif CONFIG_HAL_BOARD == HAL_BOARD_VRBRAIN
 # define CONFIG_IMU_TYPE   CONFIG_IMU_VRBRAIN
 # define CONFIG_BARO       AP_BARO_VRBRAIN
 # define CONFIG_SONAR_SOURCE SONAR_SOURCE_ANALOG_PIN
 # define MAGNETOMETER ENABLED
 # define OPTFLOW DISABLED
#endif

#if HAL_CPU_CLASS < HAL_CPU_CLASS_75 || CONFIG_HAL_BOARD == HAL_BOARD_AVR_SITL
 // low power CPUs (APM1, APM2 and SITL)
 # define MAIN_LOOP_RATE    100
 # define MAIN_LOOP_SECONDS 0.01
 # define MAIN_LOOP_MICROS  10000
#else
 // high power CPUs (Flymaple, PX4, Pixhawk, VRBrain)
 # define MAIN_LOOP_RATE    400
 # define MAIN_LOOP_SECONDS 0.0025
 # define MAIN_LOOP_MICROS  2500
#endif

//////////////////////////////////////////////////////////////////////////////
// FRAME_CONFIG
//
#ifndef FRAME_CONFIG
 # define FRAME_CONFIG   QUAD_FRAME
#endif

/////////////////////////////////////////////////////////////////////////////////
// TradHeli defaults
#if FRAME_CONFIG == HELI_FRAME
  # define RC_FAST_SPEED                        125
  # define WP_YAW_BEHAVIOR_DEFAULT              WP_YAW_BEHAVIOR_LOOK_AHEAD
  # define RATE_ROLL_D                          0
  # define RATE_PITCH_D                         0
  # define MPU6K_FILTER                         10
  # define HELI_STAB_COLLECTIVE_MIN_DEFAULT     0
  # define HELI_STAB_COLLECTIVE_MAX_DEFAULT     1000
  # define THR_MIN_DEFAULT                      0
  # define AUTOTUNE_ENABLED                     DISABLED
#endif

/////////////////////////////////////////////////////////////////////////////////
// Y6 defaults
#if FRAME_CONFIG == Y6_FRAME
  # define RATE_ROLL_P                  0.1f
  # define RATE_ROLL_D                  0.006f
  # define RATE_PITCH_P                 0.1f
  # define RATE_PITCH_D                 0.006f
  # define RATE_YAW_P                   0.150f
  # define RATE_YAW_I                   0.015f
#endif


// optical flow doesn't work in SITL yet
#ifdef DESKTOP_BUILD
# define OPTFLOW DISABLED
#endif


//////////////////////////////////////////////////////////////////////////////
// IMU Selection
//
#ifndef CONFIG_IMU_TYPE
 # define CONFIG_IMU_TYPE CONFIG_IMU_OILPAN
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
// default RC speed in Hz
#ifndef RC_FAST_SPEED
   #   define RC_FAST_SPEED 490
#endif

////////////////////////////////////////////////////////
// LED and IO Pins
//
#if CONFIG_HAL_BOARD == HAL_BOARD_APM1
#elif CONFIG_HAL_BOARD == HAL_BOARD_APM2
#elif CONFIG_HAL_BOARD == HAL_BOARD_AVR_SITL
#elif CONFIG_HAL_BOARD == HAL_BOARD_PX4
#elif CONFIG_HAL_BOARD == HAL_BOARD_FLYMAPLE
#elif CONFIG_HAL_BOARD == HAL_BOARD_LINUX
 # define LED_ON           LOW
 # define LED_OFF          HIGH
#elif CONFIG_HAL_BOARD == HAL_BOARD_VRBRAIN
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
  #  define CONFIG_SONAR_SOURCE_ANALOG_PIN 0
 # endif
#else
 # warning Invalid value for CONFIG_SONAR_SOURCE, disabling sonar
 # define CONFIG_SONAR DISABLED
#endif

#ifndef CONFIG_SONAR
 # define CONFIG_SONAR ENABLED
#endif

#ifndef SONAR_ALT_HEALTH_MAX
 # define SONAR_ALT_HEALTH_MAX 3            // number of good reads that indicates a healthy sonar
#endif

#ifndef SONAR_RELIABLE_DISTANCE_PCT
 # define SONAR_RELIABLE_DISTANCE_PCT 0.60f // we trust the sonar out to 60% of it's maximum range
#endif

#ifndef SONAR_GAIN_DEFAULT
 # define SONAR_GAIN_DEFAULT 0.8            // gain for controlling how quickly sonar range adjusts target altitude (lower means slower reaction)
#endif

#ifndef THR_SURFACE_TRACKING_VELZ_MAX
 # define THR_SURFACE_TRACKING_VELZ_MAX 150 // max vertical speed change while surface tracking with sonar
#endif

//////////////////////////////////////////////////////////////////////////////
// Channel 7 and 8 default options
//

#ifndef CH7_OPTION
 # define CH7_OPTION            AUX_SWITCH_DO_NOTHING
#endif

#ifndef CH8_OPTION
 # define CH8_OPTION            AUX_SWITCH_DO_NOTHING
#endif

//////////////////////////////////////////////////////////////////////////////
// HIL_MODE                                 OPTIONAL

#ifndef HIL_MODE
 #define HIL_MODE        HIL_MODE_DISABLED
#endif

#if HIL_MODE != HIL_MODE_DISABLED       // we are in HIL mode

 #undef CONFIG_SONAR
 #define CONFIG_SONAR DISABLED

#endif


#ifndef MAV_SYSTEM_ID
 # define MAV_SYSTEM_ID          1
#endif


//////////////////////////////////////////////////////////////////////////////
// Serial port speeds.
//
#ifndef SERIAL0_BAUD
 # define SERIAL0_BAUD                   115200
#endif
#ifndef SERIAL1_BAUD
 # define SERIAL1_BAUD                    57600
#endif
#ifndef SERIAL2_BAUD
 # define SERIAL2_BAUD                    57600
#endif


//////////////////////////////////////////////////////////////////////////////
// Battery monitoring
//
#ifndef FS_BATT_VOLTAGE_DEFAULT
 # define FS_BATT_VOLTAGE_DEFAULT       10.5f       // default battery voltage below which failsafe will be triggered
#endif

#ifndef FS_BATT_MAH_DEFAULT
 # define FS_BATT_MAH_DEFAULT             0         // default battery capacity (in mah) below which failsafe will be triggered
#endif

#ifndef BOARD_VOLTAGE_MIN
 # define BOARD_VOLTAGE_MIN             4.3f        // min board voltage in volts for pre-arm checks
#endif

#ifndef BOARD_VOLTAGE_MAX
 # define BOARD_VOLTAGE_MAX             5.8f        // max board voltage in volts for pre-arm checks
#endif

// GPS failsafe
#ifndef FAILSAFE_GPS_TIMEOUT_MS
 # define FAILSAFE_GPS_TIMEOUT_MS       5000    // gps failsafe triggers after 5 seconds with no GPS
#endif
#ifndef GPS_HDOP_GOOD_DEFAULT
 # define GPS_HDOP_GOOD_DEFAULT         200     // minimum hdop that represents a good position.  used during pre-arm checks if fence is enabled
#endif

// GCS failsafe
#ifndef FS_GCS
 # define FS_GCS                        DISABLED
#endif
#ifndef FS_GCS_TIMEOUT_MS
 # define FS_GCS_TIMEOUT_MS             5000    // gcs failsafe triggers after 5 seconds with no GCS heartbeat
#endif
// possible values for FS_GCS parameter
#define FS_GCS_DISABLED                     0
#define FS_GCS_ENABLED_ALWAYS_RTL           1
#define FS_GCS_ENABLED_CONTINUE_MISSION     2

// pre-arm check max velocity
#ifndef PREARM_MAX_VELOCITY_CMS
 # define PREARM_MAX_VELOCITY_CMS           50.0f   // vehicle must be travelling under 50cm/s before arming
#endif

//////////////////////////////////////////////////////////////////////////////
//  MAGNETOMETER
#ifndef MAGNETOMETER
 # define MAGNETOMETER                   ENABLED
#endif

// expected magnetic field strength.  pre-arm checks will fail if 50% higher or lower than this value
#if CONFIG_HAL_BOARD == HAL_BOARD_APM1 || CONFIG_HAL_BOARD == HAL_BOARD_APM2
 #ifndef COMPASS_MAGFIELD_EXPECTED
  # define COMPASS_MAGFIELD_EXPECTED     330        // pre arm will fail if mag field > 544 or < 115
 #endif
#else // PX4, SITL
 #ifndef COMPASS_MAGFIELD_EXPECTED
  #define COMPASS_MAGFIELD_EXPECTED      530        // pre arm will fail if mag field > 874 or < 185
 #endif
#endif

// max compass offset length (i.e. sqrt(offs_x^2+offs_y^2+offs_Z^2))
#ifndef CONFIG_ARCH_BOARD_PX4FMU_V1
 #ifndef COMPASS_OFFSETS_MAX
  # define COMPASS_OFFSETS_MAX          600         // PX4 onboard compass has high offsets
 #endif
#else   // APM1, APM2, SITL, FLYMAPLE, etc
 #ifndef COMPASS_OFFSETS_MAX
  # define COMPASS_OFFSETS_MAX          500
 #endif
#endif

//////////////////////////////////////////////////////////////////////////////
//  OPTICAL_FLOW
#ifndef OPTFLOW                         // sets global enabled/disabled flag for optflow (as seen in CLI)
 # define OPTFLOW                       DISABLED
#endif
// optical flow based loiter PI values
#ifndef OPTFLOW_ROLL_P
 #define OPTFLOW_ROLL_P 2.5f
#endif
#ifndef OPTFLOW_ROLL_I
 #define OPTFLOW_ROLL_I 0.5f
#endif
#ifndef OPTFLOW_ROLL_D
 #define OPTFLOW_ROLL_D 0.12f
#endif
#ifndef OPTFLOW_PITCH_P
 #define OPTFLOW_PITCH_P 2.5f
#endif
#ifndef OPTFLOW_PITCH_I
 #define OPTFLOW_PITCH_I 0.5f
#endif
#ifndef OPTFLOW_PITCH_D
 #define OPTFLOW_PITCH_D 0.12f
#endif
#ifndef OPTFLOW_IMAX
 #define OPTFLOW_IMAX 100
#endif

//////////////////////////////////////////////////////////////////////////////
//  Auto Tuning
#ifndef AUTOTUNE_ENABLED
 # define AUTOTUNE_ENABLED  ENABLED
#endif

//////////////////////////////////////////////////////////////////////////////
//  Crop Sprayer
#ifndef SPRAYER
 # define SPRAYER  DISABLED
#endif

//////////////////////////////////////////////////////////////////////////////
//	EPM cargo gripper
#ifndef EPM_ENABLED
 # define EPM_ENABLED DISABLED
#endif

//////////////////////////////////////////////////////////////////////////////
// Parachute release
#ifndef PARACHUTE
 # define PARACHUTE DISABLED
#endif

//////////////////////////////////////////////////////////////////////////////
// RADIO CONFIGURATION
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
// FLIGHT_MODE
//

#if !defined(FLIGHT_MODE_1)
 # define FLIGHT_MODE_1                  STABILIZE
#endif
#if !defined(FLIGHT_MODE_2)
 # define FLIGHT_MODE_2                  STABILIZE
#endif
#if !defined(FLIGHT_MODE_3)
 # define FLIGHT_MODE_3                  STABILIZE
#endif
#if !defined(FLIGHT_MODE_4)
 # define FLIGHT_MODE_4                  STABILIZE
#endif
#if !defined(FLIGHT_MODE_5)
 # define FLIGHT_MODE_5                  STABILIZE
#endif
#if !defined(FLIGHT_MODE_6)
 # define FLIGHT_MODE_6                  STABILIZE
#endif


//////////////////////////////////////////////////////////////////////////////
// Throttle Failsafe
//
#ifndef FS_THR_VALUE_DEFAULT
 # define FS_THR_VALUE_DEFAULT             975
#endif

#ifndef LAND_SPEED
 # define LAND_SPEED    50          // the descent speed for the final stage of landing in cm/s
#endif
#ifndef LAND_START_ALT
 # define LAND_START_ALT 1000         // altitude in cm where land controller switches to slow rate of descent
#endif
#ifndef LAND_DETECTOR_TRIGGER
 # define LAND_DETECTOR_TRIGGER 50    // number of 50hz iterations with near zero climb rate and low throttle that triggers landing complete.
#endif
#ifndef LAND_REQUIRE_MIN_THROTTLE_TO_DISARM // require pilot to reduce throttle to minimum before vehicle will disarm
 # define LAND_REQUIRE_MIN_THROTTLE_TO_DISARM ENABLED
#endif

//////////////////////////////////////////////////////////////////////////////
// CAMERA TRIGGER AND CONTROL
//
#ifndef CAMERA
 # define CAMERA        ENABLED
#endif

//////////////////////////////////////////////////////////////////////////////
// MOUNT (ANTENNA OR CAMERA)
//
#ifndef MOUNT
 # define MOUNT         ENABLED
#endif

#ifndef MOUNT2
 # define MOUNT2         DISABLED
#endif


//////////////////////////////////////////////////////////////////////////////
// Flight mode definitions
//

// Acro Mode
#ifndef ACRO_RP_P
 # define ACRO_RP_P                 4.5f
#endif

#ifndef ACRO_YAW_P
 # define ACRO_YAW_P                4.5f
#endif

#ifndef ACRO_LEVEL_MAX_ANGLE
 # define ACRO_LEVEL_MAX_ANGLE      3000
#endif

#ifndef ACRO_BALANCE_ROLL
 #define ACRO_BALANCE_ROLL          1.0f
#endif

#ifndef ACRO_BALANCE_PITCH
 #define ACRO_BALANCE_PITCH         1.0f
#endif

// Stabilize (angle controller) gains
#ifndef STABILIZE_ROLL_P
 # define STABILIZE_ROLL_P          4.5f
#endif

#ifndef STABILIZE_PITCH_P
 # define STABILIZE_PITCH_P         4.5f
#endif

#ifndef  STABILIZE_YAW_P
 # define STABILIZE_YAW_P           4.5f
#endif

// RTL Mode
#ifndef RTL_ALT_FINAL
 # define RTL_ALT_FINAL             0       // the altitude the vehicle will move to as the final stage of Returning to Launch.  Set to zero to land.
#endif

#ifndef RTL_ALT
 # define RTL_ALT 				    1500    // default alt to return to home in cm, 0 = Maintain current altitude
#endif

#ifndef RTL_LOITER_TIME
 # define RTL_LOITER_TIME           5000    // Time (in milliseconds) to loiter above home before begining final descent
#endif

// AUTO Mode
#ifndef WP_YAW_BEHAVIOR_DEFAULT
 # define WP_YAW_BEHAVIOR_DEFAULT   WP_YAW_BEHAVIOR_LOOK_AT_NEXT_WP_EXCEPT_RTL
#endif

#ifndef AUTO_YAW_SLEW_RATE
 # define AUTO_YAW_SLEW_RATE    60              // degrees/sec
#endif

#ifndef YAW_LOOK_AHEAD_MIN_SPEED
 # define YAW_LOOK_AHEAD_MIN_SPEED  100             // minimum ground speed in cm/s required before copter is aimed at ground course
#endif

// Super Simple mode
#ifndef SUPER_SIMPLE_RADIUS
 # define SUPER_SIMPLE_RADIUS       1000
#endif

//////////////////////////////////////////////////////////////////////////////
// Stabilize Rate Control
//
#ifndef ROLL_PITCH_INPUT_MAX
 # define ROLL_PITCH_INPUT_MAX      4500            // roll, pitch input range
#endif
#ifndef DEFAULT_ANGLE_MAX
 # define DEFAULT_ANGLE_MAX         4500            // ANGLE_MAX parameters default value
#endif
#ifndef ANGLE_RATE_MAX
 # define ANGLE_RATE_MAX            18000           // default maximum rotation rate in roll/pitch axis requested by angle controller used in stabilize, loiter, rtl, auto flight modes
#endif

//////////////////////////////////////////////////////////////////////////////
// Rate controller gains
//
#ifndef RATE_ROLL_P
 # define RATE_ROLL_P        		0.150f
#endif
#ifndef RATE_ROLL_I
 # define RATE_ROLL_I        		0.100f
#endif
#ifndef RATE_ROLL_D
 # define RATE_ROLL_D        		0.004f
#endif
#ifndef RATE_ROLL_IMAX
 # define RATE_ROLL_IMAX         	500
#endif

#ifndef RATE_PITCH_P
 # define RATE_PITCH_P       		0.150f
#endif
#ifndef RATE_PITCH_I
 # define RATE_PITCH_I       		0.100f
#endif
#ifndef RATE_PITCH_D
 # define RATE_PITCH_D       		0.004f
#endif
#ifndef RATE_PITCH_IMAX
 # define RATE_PITCH_IMAX        	500
#endif

#ifndef RATE_YAW_P
 # define RATE_YAW_P              	0.200f
#endif
#ifndef RATE_YAW_I
 # define RATE_YAW_I              	0.020f
#endif
#ifndef RATE_YAW_D
 # define RATE_YAW_D              	0.000f
#endif
#ifndef RATE_YAW_IMAX
 # define RATE_YAW_IMAX            	800
#endif

//////////////////////////////////////////////////////////////////////////////
// Loiter position control gains
//
#ifndef LOITER_POS_P
 # define LOITER_POS_P             	1.0f
#endif

//////////////////////////////////////////////////////////////////////////////
// Loiter rate control gains
//
#ifndef LOITER_RATE_P
 # define LOITER_RATE_P          	1.0f
#endif
#ifndef LOITER_RATE_I
 # define LOITER_RATE_I          	0.5f
#endif
#ifndef LOITER_RATE_D
 # define LOITER_RATE_D          	0.0f
#endif
#ifndef LOITER_RATE_IMAX
 # define LOITER_RATE_IMAX       	400             // maximum acceleration from I term build-up in cm/s/s
#endif

//////////////////////////////////////////////////////////////////////////////
// Hybrid parameter defaults
//
#ifndef HYBRID_ENABLED
 # define HYBRID_ENABLED                ENABLED // hybrid flight mode enabled by default
#endif
#ifndef HYBRID_BRAKE_RATE_DEFAULT
 # define HYBRID_BRAKE_RATE_DEFAULT     8       // default HYBRID_BRAKE_RATE param value.  Rotation rate during braking in deg/sec
#endif
#ifndef HYBRID_BRAKE_ANGLE_DEFAULT
 # define HYBRID_BRAKE_ANGLE_DEFAULT    3000    // default HYBRID_BRAKE_ANGLE param value.  Max lean angle during braking in centi-degrees
#endif

//////////////////////////////////////////////////////////////////////////////
// Throttle control gains
//
#ifndef THROTTLE_CRUISE
 # define THROTTLE_CRUISE       450             // default estimate of throttle required for vehicle to maintain a hover
#endif

#ifndef THR_MID_DEFAULT
 # define THR_MID_DEFAULT       500             // Throttle output (0 ~ 1000) when throttle stick is in mid position
#endif

#ifndef THR_MIN_DEFAULT
 # define THR_MIN_DEFAULT       130             // minimum throttle sent to the motors when armed and pilot throttle above zero
#endif
#ifndef THR_MAX_DEFAULT
 # define THR_MAX_DEFAULT       1000            // maximum throttle sent to the motors
#endif

#ifndef THROTTLE_IN_DEADBAND
# define THROTTLE_IN_DEADBAND    100            // the throttle input channel's deadband in PWM
#endif

#ifndef ALT_HOLD_P
 # define ALT_HOLD_P            1.0f
#endif

// RATE control
#ifndef THROTTLE_RATE_P
 # define THROTTLE_RATE_P       6.0f
#endif

// default maximum vertical velocity the pilot may request
#ifndef PILOT_VELZ_MAX
 # define PILOT_VELZ_MAX    250     // maximum vertical velocity in cm/s
#endif

// max distance in cm above or below current location that will be used for the alt target when transitioning to alt-hold mode
#ifndef ALT_HOLD_INIT_MAX_OVERSHOOT
 # define ALT_HOLD_INIT_MAX_OVERSHOOT 200
#endif
// the acceleration used to define the distance-velocity curve
#ifndef ALT_HOLD_ACCEL_MAX
 # define ALT_HOLD_ACCEL_MAX 250    // if you change this you must also update the duplicate declaration in AC_WPNav.h
#endif

// Throttle Accel control
#ifndef THROTTLE_ACCEL_P
 # define THROTTLE_ACCEL_P  0.75f
#endif
#ifndef THROTTLE_ACCEL_I
 # define THROTTLE_ACCEL_I  1.50f
#endif
#ifndef THROTTLE_ACCEL_D
 # define THROTTLE_ACCEL_D 0.0f
#endif
#ifndef THROTTLE_ACCEL_IMAX
 # define THROTTLE_ACCEL_IMAX 500
#endif


//////////////////////////////////////////////////////////////////////////////
// Dataflash logging control
//
#ifndef LOGGING_ENABLED
 # define LOGGING_ENABLED                ENABLED
#endif

#if CONFIG_HAL_BOARD == HAL_BOARD_APM1 || CONFIG_HAL_BOARD == HAL_BOARD_APM2 || CONFIG_HAL_BOARD == HAL_BOARD_AVR_SITL
 // APM1 & APM2 default logging
 # define DEFAULT_LOG_BITMASK \
    MASK_LOG_ATTITUDE_MED | \
    MASK_LOG_GPS | \
    MASK_LOG_PM | \
    MASK_LOG_CTUN | \
    MASK_LOG_NTUN | \
    MASK_LOG_RCIN | \
    MASK_LOG_CMD | \
    MASK_LOG_CURRENT
#else
 // PX4, Pixhawk, FlyMaple default logging
 # define DEFAULT_LOG_BITMASK \
    MASK_LOG_ATTITUDE_MED | \
    MASK_LOG_GPS | \
    MASK_LOG_PM | \
    MASK_LOG_CTUN | \
    MASK_LOG_NTUN | \
    MASK_LOG_RCIN | \
    MASK_LOG_IMU | \
    MASK_LOG_CMD | \
    MASK_LOG_CURRENT | \
    MASK_LOG_RCOUT | \
    MASK_LOG_COMPASS | \
    MASK_LOG_CAMERA
#endif

//////////////////////////////////////////////////////////////////////////////
// AP_Limits Defaults
//

// Enable/disable AP_Limits
#ifndef AC_FENCE
 #define AC_FENCE ENABLED
#endif

#ifndef AC_RALLY
 #define AC_RALLY ENABLED
#endif

//////////////////////////////////////////////////////////////////////////////
// Developer Items
//

// use this to completely disable the CLI
#ifndef CLI_ENABLED
  #  define CLI_ENABLED           ENABLED
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

#endif // __ARDUCOPTER_CONFIG_H__
