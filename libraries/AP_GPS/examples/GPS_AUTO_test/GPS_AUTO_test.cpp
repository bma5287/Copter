// -*- tab-width: 4; Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*-
//
// Test for AP_GPS_AUTO
//

#include <stdlib.h>

#include <AP_Common.h>
#include <AP_Progmem.h>
#include <AP_Param.h>
#include <AP_HAL.h>
#include <AP_HAL_AVR.h>
#include <AP_HAL_SITL.h>
#include <AP_HAL_PX4.h>
#include <AP_HAL_Linux.h>
#include <AP_HAL_Empty.h>
#include <AP_GPS.h>
#include <DataFlash.h>
#include <AP_InertialSensor.h>
#include <AP_ADC.h>
#include <GCS_MAVLink.h>
#include <AP_Baro.h>
#include <Filter.h>
#include <AP_AHRS.h>
#include <AP_Compass.h>
#include <AP_Declination.h>
#include <AP_Airspeed.h>
#include <AP_Vehicle.h>
#include <AP_ADC_AnalogSource.h>
#include <AP_Mission.h>
#include <StorageManager.h>
#include <AP_Terrain.h>
#include <AP_Math.h>
#include <AP_Notify.h>
#include <AP_BoardLED.h>
#include <AP_NavEKF.h>
#include <AP_Rally.h>
#include <AP_Scheduler.h>
#include <AP_BattMonitor.h>
#include <AP_SerialManager.h>
#include <AP_RangeFinder.h>

const AP_HAL::HAL& hal = AP_HAL_BOARD_DRIVER;

// create board led object
AP_BoardLED board_led;

// This example uses GPS system. Create it.
AP_GPS gps;

// Serial manager is needed for UART comunications
AP_SerialManager serial_manager;

#define T6 1000000
#define T7 10000000

void setup()
{
    hal.console->println("GPS AUTO library test");

    // Initialise the leds
    board_led.init();

    // Initialize the UART for GPS system
    serial_manager.init();
    gps.init(NULL, serial_manager);
}

void loop()
{
    static uint32_t last_msg_ms;

    // Update GPS state based on possible bytes received from the module.
    gps.update();

    // If new GPS data is received, output it's contents to the console
    // Here we rely on the time of the message in GPS class and the time of last message
    // saved in static variable last_msg_ms. When new message is received, the time
    // in GPS class will be updated.
    if (last_msg_ms != gps.last_message_time_ms()) {
        // Reset the time of message
        last_msg_ms = gps.last_message_time_ms();

        // Acquire location
        const Location &loc = gps.location();

        // Print the contents of message
        hal.console->print("Lat: ");
        print_latlon(hal.console, loc.lat);
        hal.console->print(" Lon: ");
        print_latlon(hal.console, loc.lng);
        hal.console->printf(" Alt: %.2fm GSP: %.2fm/s CoG: %d SAT: %d TIM: %u/%lu STATUS: %u\n",
                            loc.alt * 0.01f,
                            gps.ground_speed(),
                            (int)gps.ground_course_cd() / 100,
                            gps.num_sats(),
                            gps.time_week(),
                            (unsigned long)gps.time_week_ms(),
                            gps.status());
    }

    // Delay for 10 mS will give us 100 Hz invocation rate
    hal.scheduler->delay(10);
}

// Register above functions in HAL board level
AP_HAL_MAIN();
