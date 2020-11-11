// ServoProtocol.h was generated by ProtoGen version 3.2.a

/*
 * This file is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Oliver Walters / Currawong Engineering Pty Ltd
 */

#ifndef _SERVOPROTOCOL_H
#define _SERVOPROTOCOL_H

// Language target is C, C++ compilers: don't mangle us
#ifdef __cplusplus
extern "C" {
#endif

/*!
 * \file
 * \mainpage Servo protocol stack
 * 
 * This is the ICD for the Currawong Engineering CAN Servo. This document
 * details the Servo command and packet structure for communication with and
 * configuration of the Servo
 * 
 * The protocol API enumeration is incremented anytime the protocol is changed
 * in a way that affects compatibility with earlier versions of the protocol.
 * The protocol enumeration for this version is: 21
 * 
 * The protocol version is 2.11
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>	// C string manipulation function header

//! \return the protocol API enumeration
#define getServoApi() 21

//! \return the protocol version string
#define getServoVersion() "2.11"

// Translation provided externally. The macro takes a `const char *` and returns a `const char *`
#ifndef translateServo
    #define translateServo(x) x
#endif

typedef enum
{
    SERVO_MODE_NORMAL,   //!< The servo is in normal operational mode
    SERVO_MODE_CALIBRATING,//!< The servo is in calibration mode (factory setting only)
    SERVO_MODE_TEST,     //!< The servo is in test mode (factory setting only)
    SERVO_MODE_NUM_MODES 
} ServoModes;

//! \return the label of a 'ServoModes' enum entry, based on its value
const char* ServoModes_EnumLabel(int value);

typedef enum
{
    SERVO_TIMEOUT_ACTION_HOLD,   //!< The servo will hold the current position
    SERVO_TIMEOUT_ACTION_DISABLE,//!< The servo will be disabled
    SERVO_TIMEOUT_ACTION_NEUTRAL,//!< The servo will move to its programmed neutral position
    SERVO_NUM_TIMEOUT_MODES      
} ServoTimeoutModes;

/*!
 * Command multiple servo positions with a single command
 */
typedef enum
{
    PKT_SERVO_MULTI_COMMAND_1 = 0x00,//!< Send a position command to servos 1,2,3,4
    PKT_SERVO_MULTI_COMMAND_2,       //!< Send a position command to servos 5,6,7,8
    PKT_SERVO_MULTI_COMMAND_3,       //!< Send a position command to servos 9,10,11,12
    PKT_SERVO_MULTI_COMMAND_4,       //!< Send a position command to servos 13,14,15,16
    PKT_SERVO_MULTI_COMMAND_5,       //!< Send a position command to servos 17,18,19,20
    PKT_SERVO_MULTI_COMMAND_6,       //!< Send a position command to servos 21,22,23,24
    PKT_SERVO_MULTI_COMMAND_7,       //!< Send a position command to servos 25,26,27,28
    PKT_SERVO_MULTI_COMMAND_8,       //!< Send a position command to servos 29,30,31,32
    PKT_SERVO_MULTI_COMMAND_9,       //!< Send a position command to servos 33,34,35,36
    PKT_SERVO_MULTI_COMMAND_10,      //!< Send a position command to servos 37,38,39,40
    PKT_SERVO_MULTI_COMMAND_11,      //!< Send a position command to servos 41,42,43,44
    PKT_SERVO_MULTI_COMMAND_12,      //!< Send a position command to servos 45,46,47,48
    PKT_SERVO_MULTI_COMMAND_13,      //!< Send a position command to servos 49,50,51,52
    PKT_SERVO_MULTI_COMMAND_14,      //!< Send a position command to servos 53,54,55,56
    PKT_SERVO_MULTI_COMMAND_15,      //!< Send a position command to servos 57,58,59,60
    PKT_SERVO_MULTI_COMMAND_16       //!< Send a position command to servos 61,62,63,64
} ServoMultiCommandPackets;

//! \return the label of a 'ServoMultiCommandPackets' enum entry, based on its value
const char* ServoMultiCommandPackets_EnumLabel(int value);

/*!
 * Servo CAN packet identifiers
 */
typedef enum
{
    PKT_SERVO_POSITION_COMMAND = 0x10,   //!< Send a position command to an individual servo
    PKT_SERVO_NEUTRAL_COMMAND = 0x15,    //!< Move the servo to the neutral position
    PKT_SERVO_DISABLE = 0x20,            //!< Disable a single servo, or all servos with a broadcast message
    PKT_SERVO_ENABLE,                    //!< Enable a single servo, or all servos with a broadcast message
    PKT_SERVO_SYSTEM_COMMAND = 0x50,     //!< Servo system command
    PKT_SERVO_SET_TITLE,                 //!< Set the title of this servo
    PKT_SERVO_STATUS_A = 0x60,           //!< Servo *STATUS_A* packet contains status and position information
    PKT_SERVO_STATUS_B,                  //!< Servo *STATUS_B* packet contains current, temperature and other information
    PKT_SERVO_STATUS_C,                  //!< Reserved for future use
    PKT_SERVO_STATUS_D,                  //!< Reserved for future use
    PKT_SERVO_ACCELEROMETER,             //!< Raw accelerometer data
    PKT_SERVO_ADDRESS = 0x70,            //!< Servo address information
    PKT_SERVO_TITLE,                     //!< Servo title information
    PKT_SERVO_FIRMWARE,                  //!< Servo firmware information
    PKT_SERVO_SYSTEM_INFO,               //!< Servo system info (uptime, etc)
    PKT_SERVO_TELEMETRY_CONFIG,          //!< Telemetry settings
    PKT_SERVO_SETTINGS_INFO,             //!< Non-volatile settings configuration information
    PKT_SERVO_FACTORY,                   //!< Factory data
    PKT_SERVO_TELLTALE_A = 0x7A,         //!< Servo telltale information
    PKT_SERVO_LIMITS = 0x80,             //!< Servo limits (current, strength, temperature)
    PKT_SERVO_CURRENT_LIMITS,            //!< Current limit control system settings
    PKT_SERVO_POTENTIOMETER,             //!< Potentiometer configuration
    PKT_SERVO_BACKLASH,                  
    PKT_SERVO_BIN_DATA,                  //!< Servo telltale binning data
    PKT_SERVO_WEAR_LEVEL_A,              //!< Wear estimate information packet 1 of 2
    PKT_SERVO_WEAR_LEVEL_B,              //!< Wear estimate information packet 2 of 2
    PKT_SERVO_LOOKUP_TABLE,              //!< Input/output mapping table information
    PKT_SERVO_LOOKUP_ELEMENT,            //!< Input/output mapping table element information
    PKT_SERVO_CONFIG,                    //!< General servo configuration
    PKT_SERVO_DELTA_CONFIG,              //!< Position error configuration
    PKT_SERVO_CALIBRATION,               //!< Servo calibration data
    PKT_SERVO_MOTION_CONTROL = 0x8C,     //!< Servo motion control settings
    PKT_SERVO_LIMIT_VALUES,              //!< Servo limit value settings
    PKT_SERVO_DEBUG_DELTA = 0x90,        //!< Position debug information
    PKT_SERVO_DEBUG_CTRL_LOOP,           //!< Control loop debug information
    PKT_SERVO_DEBUG_MOTOR,               //!< Motor information
    PKT_SERVO_DEBUG_MOTION_CTRL,         //!< Motion control debug info
    PKT_SERVO_CTRL_LOOP_SETTINGS = 0xA0, //!< Position control loop settings
    PKT_SERVO_TELLTALE_SETTINGS = 0x101, //!< Telltale settings packet
    PKT_SERVO_USER_SETTINGS = 0x105,     //!< User settings (title, ID etc)
    PKT_SERVO_SYSTEM_SETTINGS = 0x1FF    //!< System settings (serial number, etc)
} ServoPackets;

//! \return the label of a 'ServoPackets' enum entry, based on its value
const char* ServoPackets_EnumLabel(int value);

/*!
 * These commands are sent to the servo using the SERVO_SYSTEM_COMMAND packet
 */
typedef enum
{
    CMD_SERVO_CONFIGURE_LOOKUP_TABLE = 0x00, //!< Configure I/O map parameters
    CMD_SERVO_SET_LOOKUP_TABLE_ELEMENT,      //!< Configure an individual I/O element
    CMD_SERVO_GET_LOOKUP_TABLE_ELEMENT,      //!< Request an individual I/O element
    CMD_SERVO_SET_CONFIG = 0x10,             //!< Set the servo configuration bits
    CMD_SERVO_SET_CURRENT_LIMIT = 0x20,      //!< Set the servo current limit
    CMD_SERVO_SET_TEMPERATURE_LIMIT,         //!< Set the over-temperature warning level
    CMD_SERVO_SET_RATE_LIMIT,                //!< Set the servo rate limit
    CMD_SERVO_SET_STRENGTH,                  //!< Set the servo 'strength' (maximum motor duty cycle)
    CMD_SERVO_SET_ILIMIT_KP,                 //!< Set the servo current-limit proportional gain value
    CMD_SERVO_SET_ILIMIT_KI,                 //!< Set the servo current-limit integral gain value
    CMD_SERVO_SET_MIN_PWM_LIMIT = 0x30,      //!< Set the minimum clipping value for the servo PWM signal
    CMD_SERVO_SET_MAX_PWM_LIMIT,             //!< Set the maximum clipping value for the servo PWM signal
    CMD_SERVO_SET_NEUTRAL_POSITION,          //!< Set the neutral position for the servo
    CMD_SERVO_SET_TELEMETRY_PERIOD = 0x40,   //!< Set the servo telemetry period
    CMD_SERVO_SET_SILENCE_PERIOD,            //!< Set the servo silence period
    CMD_SERVO_SET_TELEMETRY_PACKETS,         //!< Configure telemetry packets
    CMD_SERVO_REQUEST_HF_DATA = 0x43,        //!< Request high-frequency telemetry data
    CMD_SERVO_SET_CMD_TIMEOUT,               //!< Set the command timeout for the servo
    CMD_SERVO_SET_NODE_ID = 0x50,            //!< Configure the CAN servo node ID
    CMD_SERVO_SET_USER_ID_A,                 //!< Set User ID A
    CMD_SERVO_SET_USER_ID_B,                 //!< Set User ID B
    CMD_SERVO_CALIBRATE_POT = 0x60,          //!< Initiate servo potentiometer calibration procedure
    CMD_SERVO_START_TEST_MODE,               
    CMD_SERVO_STOP_TEST_MODE,                
    CMD_SERVO_START_BACKLASH_TEST,           
    CMD_SERVO_RESET_HALL_COUNTS,             
    CMD_SERVO_SET_MIDDLE_POS = 0x70,         
    CMD_SERVO_RESET_TELLTALES = 0x80,        //!< Reset servo telltale data
    CMD_SERVO_CLEAR_BIN_DATA,                
    CMD_SERVO_ERASE_EEPROM = 0x90,           
    CMD_SERVO_SET_COMMISSIONING_FLAG = 0x95, //!< Set servo commissioning flag
    CMD_SERVO_RESET_DEFAULT_SETTINGS = 0xA0, //!< Reset servo configuration settings to default values
    CMD_SERVO_SET_PROFILE_TASK = 0xF0,       //!< Set the task to profile
    CMD_SERVO_UNLOCK_SETTINGS = 0xF5,        //!< Unlock servo settings
    CMD_SERVO_LOCK_SETTINGS,                 //!< Lock servo settings
    CMD_SERVO_ENTER_BOOTLOADER = 0xFB,       //!< Enter bootloader mode
    CMD_SERVO_RESET,                         //!< Reset servo
    CMD_SERVO_SET_SERIAL_NUMBER = 0xFF       //!< Set the serial number
} ServoCommands;

//! \return the label of a 'ServoCommands' enum entry, based on its value
const char* ServoCommands_EnumLabel(int value);


// The prototypes below provide an interface to the packets.
// They are not auto-generated functions, but must be hand-written

//! \return the packet data pointer from the packet
uint8_t* getServoPacketData(void* pkt);

//! \return the packet data pointer from the packet, const
const uint8_t* getServoPacketDataConst(const void* pkt);

//! Complete a packet after the data have been encoded
void finishServoPacket(void* pkt, int size, uint32_t packetID);

//! \return the size of a packet from the packet header
int getServoPacketSize(const void* pkt);

//! \return the ID of a packet from the packet header
uint32_t getServoPacketID(const void* pkt);

#ifdef __cplusplus
}
#endif
#endif // _SERVOPROTOCOL_H
