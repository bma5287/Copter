// -*- tab-width: 4; Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*-

/// @file	AP_MotorsCoax.h
/// @brief	Motor and Servo control class for Co-axial helicopters with two motors and two flaps
#pragma once

#include <AP_Common/AP_Common.h>
#include <AP_Math/AP_Math.h>        // ArduPilot Mega Vector/Matrix math Library
#include <RC_Channel/RC_Channel.h>     // RC Channel Library
#include "AP_MotorsMulticopter.h"

// feedback direction
#define AP_MOTORS_COAX_POSITIVE      1
#define AP_MOTORS_COAX_NEGATIVE     -1

#define NUM_ACTUATORS 4

#define AP_MOTORS_SINGLE_SPEED_DIGITAL_SERVOS 250 // update rate for digital servos
#define AP_MOTORS_SINGLE_SPEED_ANALOG_SERVOS 125  // update rate for analog servos

#define AP_MOTORS_COAX_SERVO_INPUT_RANGE    4500    // roll or pitch input of -4500 will cause servos to their minimum (i.e. radio_min), +4500 will move them to their maximum (i.e. radio_max)

/// @class      AP_MotorsSingle
class AP_MotorsCoax : public AP_MotorsMulticopter {
public:

    /// Constructor
    AP_MotorsCoax(RC_Channel& servo1, RC_Channel& servo2, RC_Channel& servo3, RC_Channel& servo4, uint16_t loop_rate, uint16_t speed_hz = AP_MOTORS_SPEED_DEFAULT) :
        AP_MotorsMulticopter(loop_rate, speed_hz),
        _servo1(servo1),
        _servo2(servo2),
        _servo3(servo3),
        _servo4(servo4)
    {
        AP_Param::setup_object_defaults(this, var_info);
    };

    // init
    virtual void        Init();

    // set update rate to motors - a value in hertz
    void                set_update_rate( uint16_t speed_hz );

    // enable - starts allowing signals to be sent to motors
    virtual void        enable();

    // output_test - spin a motor at the pwm value specified
    //  motor_seq is the motor's sequence number from 1 to the number of motors on the frame
    //  pwm value is an actual pwm value that will be output, normally in the range of 1000 ~ 2000
    virtual void        output_test(uint8_t motor_seq, int16_t pwm);

    // output_min - sends minimum values out to the motors
    virtual void        output_min();

    // get_motor_mask - returns a bitmask of which outputs are being used for motors or servos (1 means being used)
    //  this can be used to ensure other pwm outputs (i.e. for servos) do not conflict
    virtual uint16_t    get_motor_mask();

    // var_info for holding Parameter information
    static const struct AP_Param::GroupInfo var_info[];

protected:
    // output - sends commands to the motors
    void                output_armed_stabilizing();

    // calc_yaw_radio_output - calculate final radio output for yaw channel
    int16_t             calc_pivot_radio_output(float yaw_input, int16_t servo_min, int16_t servo_trim, int16_t servo_max);        // calculate radio output for yaw servo, typically in range of 1100-1900

    // We shouldn't need roll, pitch, and yaw reversing with servo reversing.
    AP_Int8             _roll_reverse;  // Reverse roll output
    AP_Int8             _pitch_reverse; // Reverse pitch output
    AP_Int8             _yaw_reverse;   // Reverse yaw output
    AP_Int16            _servo_speed;   // servo speed
    // Allow the use of a 4 servo output to make it easy to test coax and single using same airframe
    RC_Channel&         _servo1;
    RC_Channel&         _servo2;
    RC_Channel&         _servo3;
    RC_Channel&         _servo4;

    AP_Int8             _servo_1_reverse;    // Roll servo signal reversing
    AP_Int16            _servo_1_trim;       // Trim or center position of roll servo
    AP_Int16            _servo_1_min;        // Minimum angle limit of roll servo
    AP_Int16            _servo_1_max;        // Maximum angle limit of roll servo
    AP_Int8             _servo_2_reverse;   // Pitch servo signal reversing
    AP_Int16            _servo_2_trim;      // Trim or center position of pitch servo
    AP_Int16            _servo_2_min;       // Minimum angle limit of pitch servo
    AP_Int16            _servo_2_max;       // Maximum angle limit of pitch servo
    AP_Int8             _servo_3_reverse;   // Pitch servo signal reversing
    AP_Int16            _servo_3_trim;      // Trim or center position of pitch servo
    AP_Int16            _servo_3_min;       // Minimum angle limit of pitch servo
    AP_Int16            _servo_3_max;       // Maximum angle limit of pitch servo
    AP_Int8             _servo_4_reverse;   // Pitch servo signal reversing
    AP_Int16            _servo_4_trim;      // Trim or center position of pitch servo
    AP_Int16            _servo_4_min;       // Minimum angle limit of pitch servo
    AP_Int16            _servo_4_max;       // Maximum angle limit of pitch servo

    float               _actuator_out[NUM_ACTUATORS]; // combined roll, pitch, yaw and throttle outputs to motors in 0~1 range
    float               _thrust_yt_ccw;
    float               _thrust_yt_cw;
};
