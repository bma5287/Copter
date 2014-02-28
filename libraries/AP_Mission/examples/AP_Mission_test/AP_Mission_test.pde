/*
 *  Example of AP_Mission Library.
 *  DIYDrones.com
 */

#include <AP_Common.h>
#include <AP_Progmem.h>
#include <AP_Math.h>            // ArduPilot Mega Vector/Matrix math Library
#include <AP_Curve.h>           // Curve used to linearlise throttle pwm to thrust
#include <AP_Param.h>
#include <AP_HAL.h>
#include <AP_HAL_AVR.h>
#include <GCS_MAVLink.h>
#include <AP_Notify.h>
#include <AP_Vehicle.h>
#include <DataFlash.h>
#include <AP_Mission.h>
#include <AP_GPS.h>             // ArduPilot GPS library
#include <AP_GPS_Glitch.h>      // GPS glitch protection library
#include <AP_ADC.h>             // ArduPilot Mega Analog to Digital Converter Library
#include <AP_ADC_AnalogSource.h>
#include <AP_Baro.h>            // ArduPilot Mega Barometer Library
#include <Filter.h>
#include <AP_Compass.h>         // ArduPilot Mega Magnetometer Library
#include <AP_Declination.h>
#include <AP_InertialSensor.h>  // ArduPilot Mega Inertial Sensor (accel & gyro) Library
#include <AP_AHRS.h>
#include <AP_Airspeed.h>
#include <AP_Buffer.h>          // ArduPilot general purpose FIFO buffer
#include <GCS_MAVLink.h>

const AP_HAL::HAL& hal = AP_HAL_BOARD_DRIVER;

// INS and Baro declaration
#if CONFIG_HAL_BOARD == HAL_BOARD_APM2

AP_InertialSensor_MPU6000 ins;
AP_Baro_MS5611 baro(&AP_Baro_MS5611::spi);

#else

AP_ADC_ADS7844 adc;
AP_InertialSensor_Oilpan ins(&adc);
AP_Baro_BMP085 baro;
#endif

// GPS declaration
GPS *gps;
AP_GPS_Auto auto_gps(&gps);
GPS_Glitch gps_glitch(gps);

AP_Compass_HMC5843 compass;
AP_AHRS_DCM ahrs(ins, baro, gps);

// global constants that control how many verify calls must be made for a command before it completes
static uint8_t verify_nav_cmd_iterations_to_complete = 3;
static uint8_t verify_do_cmd_iterations_to_complete = 1;
static uint8_t num_nav_cmd_runs = 0;
static uint8_t num_do_cmd_runs = 0;

// start_cmd - function that is called when new command is started
//      should return true if command is successfully started
bool start_cmd(const AP_Mission::Mission_Command& cmd)
{
    // reset tracking of number of iterations of this command (we simulate all nav commands taking 3 iterations to complete, all do command 1 iteration)
    if (AP_Mission::is_nav_cmd(cmd)) {
        num_nav_cmd_runs = 0;
        hal.console->printf_P(PSTR("started cmd #%d id:%d Nav\n"),(int)cmd.index,(int)cmd.id);
    }else{
        num_do_cmd_runs = 0;
        hal.console->printf_P(PSTR("started cmd #%d id:%d Do\n"),(int)cmd.index,(int)cmd.id);
    }

    return true;
}

// verify_mcd - function that is called repeatedly to ensure a command is progressing
//      should return true once command is completed
bool verify_cmd(const AP_Mission::Mission_Command& cmd)
{
    if (AP_Mission::is_nav_cmd(cmd)) {
        num_nav_cmd_runs++;
        if (num_nav_cmd_runs < verify_nav_cmd_iterations_to_complete) {
            hal.console->printf_P(PSTR("verified cmd #%d id:%d Nav iteration:%d\n"),(int)cmd.index,(int)cmd.id,(int)num_nav_cmd_runs);
            return false;
        }else{
            hal.console->printf_P(PSTR("verified cmd #%d id:%d Nav complete!\n"),(int)cmd.index,(int)cmd.id);
            return true;
        }
    }else{
        num_do_cmd_runs++;
        if (num_do_cmd_runs < verify_do_cmd_iterations_to_complete) {
            hal.console->printf_P(PSTR("verified cmd #%d id:%d Do iteration:%d\n"),(int)cmd.index,(int)cmd.id,(int)num_do_cmd_runs);
            return false;
        }else{
            hal.console->printf_P(PSTR("verified cmd #%d id:%d Do complete!\n"),(int)cmd.index,(int)cmd.id);
            return true;
        }
    }
}

// mission_complete - function that is called once the mission completes
void mission_complete(void)
{
    hal.console->printf_P(PSTR("\nMission Complete!\n"));
}

// declaration
AP_Mission mission(ahrs, &start_cmd, &verify_cmd, &mission_complete);

// setup
void setup(void)
{
    hal.console->println("AP_Mission library test\n");

    // display basic info about command sizes
    hal.console->printf_P(PSTR("Max Num Commands: %d\n"),(int)AP_MISSION_MAX_COMMANDS);
    hal.console->printf_P(PSTR("Command size: %d bytes\n"),(int)AP_MISSION_EEPROM_COMMAND_SIZE);
    hal.console->printf_P(PSTR("Command start in Eeprom: %x\n"),(int)AP_MISSION_EEPROM_START_BYTE);
}

// loop
void loop(void)
{
    // uncomment line below to run one of the mission tests
    run_mission_test();

    // uncomment line below to run the mission pause/resume test
    //run_resume_test();

    // wait forever
    while(true) {
        hal.scheduler->delay(1000);
    }
}

// run_mission_test - tests the stop and resume feature
void run_mission_test()
{
    // uncomment one of the init_xxx() commands below to run the test

    init_mission();                   // run simple mission with many nav commands and one do-jump
    //init_mission_no_nav_commands();   // mission should start the first do command but then complete
    //init_mission_endless_loop();      // mission should ignore the jump that causes the endless loop and complete

    // mission with a do-jump to the previous command which is a "do" command
    //      ideally we would execute this previous "do" command the number of times specified in the do-jump command but this is tricky so we ignore the do-jump
    //      mission should run the "do" command once and then complete
    //init_mission_jump_to_nonnav();

    // mission which starts with do comamnds
    //      first command to execute should be the first do command followed by the first nav command
    //      second do command should execute after 1st do command completes
    //      third do command (which is after 1st nav command) should start after 1st nav command completes
    //init_mission_starts_with_do_commands();

    // init_mission_ends_with_do_commands - initialise a mission which ends with do comamnds
    //      a single do command just after nav command will be started but not verified because mission will complete
    //      final do command will not be started
    //init_mission_ends_with_do_commands();

    // init_mission_ends_with_jump_command - initialise a mission which ends with a jump comamnd
    //      mission should complete after the do-jump is executed the appropriate number of times
    //init_mission_ends_with_jump_command();

    // print current mission
    print_mission();

    // start mission
    hal.console->printf_P(PSTR("\nRunning missions\n"));
    mission.start();

    // update mission forever
    while(true) {
        mission.update();
    }
}

// run_resume_test - tests the stop and resume feature
//      when mission is resumed, active commands should be started again
void run_resume_test()
{
    AP_Mission::Mission_Command cmd;

    // create a mission
    // Command #0 : take-off to 10m
    cmd.id = MAV_CMD_NAV_TAKEOFF;
    cmd.content.location.options = 0;
    cmd.content.location.p1 = 0;
    cmd.content.location.alt = 10;
    cmd.content.location.lat = 0;
    cmd.content.location.lng = 0;
    if (!mission.add_cmd(cmd)) {
        hal.console->printf_P(PSTR("failed to add command\n"));
    }

    // Command #1 : first waypoint
    cmd.id = MAV_CMD_NAV_WAYPOINT;
    cmd.content.location.options = 0;
    cmd.content.location.p1 = 0;
    cmd.content.location.alt = 11;
    cmd.content.location.lat = 12345678;
    cmd.content.location.lng = 23456789;
    if (!mission.add_cmd(cmd)) {
        hal.console->printf_P(PSTR("failed to add command\n"));
    }

    // Command #2 : second waypoint
    cmd.id = MAV_CMD_NAV_WAYPOINT;
    cmd.content.location.p1 = 0;
    cmd.content.location.lat = 1234567890;
    cmd.content.location.lng = -1234567890;
    cmd.content.location.alt = 22;
    if (!mission.add_cmd(cmd)) {
        hal.console->printf_P(PSTR("failed to add command\n"));
    }

    // Command #3 : do command
    cmd.id = MAV_CMD_DO_SET_ROI;
    cmd.content.location.options = 0;
    cmd.content.location.p1 = 0;
    cmd.content.location.alt = 11;
    cmd.content.location.lat = 12345678;
    cmd.content.location.lng = 23456789;
    if (!mission.add_cmd(cmd)) {
        hal.console->printf_P(PSTR("failed to add command\n"));
    }

    // Command #4 : RTL
    cmd.id = MAV_CMD_NAV_RETURN_TO_LAUNCH;
    cmd.content.location.p1 = 0;
    cmd.content.location.lat = 0;
    cmd.content.location.lng = 0;
    cmd.content.location.alt = 0;
    if (!mission.add_cmd(cmd)) {
        hal.console->printf_P(PSTR("failed to add command\n"));
    }

    // print current mission
    print_mission();

    // start mission
    hal.console->printf_P(PSTR("\nRunning missions\n"));
    mission.start();

    // update the mission for X iterations
    // set condition to "i<5" to catch mission as cmd #1 (Nav) is running - you should see it restart cmd #1
    // set condition to "i<7" to catch mission just after cmd #1 (Nav) has completed - you should see it start cmd #2
    // set condition to "i<11" to catch mission just after cmd #2 (Nav) has completed - you should see it start cmd #3 (Do) and cmd #4 (Nav)
    for(uint8_t i=0; i<11; i++) {
        mission.update();
    }

    // simulate user pausing the mission
    hal.console->printf_P(PSTR("Stopping mission\n"));
    mission.stop();

    // update the mission for 5 seconds (nothing should happen)
    uint32_t start_time = hal.scheduler->millis();
    while(hal.scheduler->millis() - start_time < 5000) {
        mission.update();
    }

    // simulate user resuming mission
    hal.console->printf_P(PSTR("Resume mission\n"));
    mission.resume();

    // update the mission forever
    while(true) {
        mission.update();
    }
}

// init_mission - initialise the mission to hold something
void init_mission()
{
    AP_Mission::Mission_Command cmd;

    // clear mission
    mission.clear();

    // Command #0 : home
    cmd.id = MAV_CMD_NAV_WAYPOINT;
    cmd.content.location.options = 0;
    cmd.content.location.p1 = 0;
    cmd.content.location.alt = 0;
    cmd.content.location.lat = 12345678;
    cmd.content.location.lng = 23456789;
    if (!mission.add_cmd(cmd)) {
        hal.console->printf_P(PSTR("failed to add command\n"));
    }

    // Command #1 : take-off to 10m
    cmd.id = MAV_CMD_NAV_TAKEOFF;
    cmd.content.location.options = 0;
    cmd.content.location.p1 = 0;
    cmd.content.location.alt = 10;
    cmd.content.location.lat = 0;
    cmd.content.location.lng = 0;
    if (!mission.add_cmd(cmd)) {
        hal.console->printf_P(PSTR("failed to add command\n"));
    }

    // Command #2 : first waypoint
    cmd.id = MAV_CMD_NAV_WAYPOINT;
    cmd.content.location.options = 0;
    cmd.content.location.p1 = 0;
    cmd.content.location.alt = 11;
    cmd.content.location.lat = 12345678;
    cmd.content.location.lng = 23456789;
    if (!mission.add_cmd(cmd)) {
        hal.console->printf_P(PSTR("failed to add command\n"));
    }

    // Command #3 : second waypoint
    cmd.id = MAV_CMD_NAV_WAYPOINT;
    cmd.content.location.p1 = 0;
    cmd.content.location.lat = 1234567890;
    cmd.content.location.lng = -1234567890;
    cmd.content.location.alt = 22;
    if (!mission.add_cmd(cmd)) {
        hal.console->printf_P(PSTR("failed to add command\n"));
    }

    // Command #4 : do-jump to first waypoint 3 times
    cmd.id = MAV_CMD_DO_JUMP;
    cmd.content.jump.target = 2;
    cmd.content.jump.num_times = 1;
    if (!mission.add_cmd(cmd)) {
        hal.console->printf_P(PSTR("failed to add command\n"));
    }

    // Command #5 : RTL
    cmd.id = MAV_CMD_NAV_RETURN_TO_LAUNCH;
    cmd.content.location.p1 = 0;
    cmd.content.location.lat = 0;
    cmd.content.location.lng = 0;
    cmd.content.location.alt = 0;
    if (!mission.add_cmd(cmd)) {
        hal.console->printf_P(PSTR("failed to add command\n"));
    }
}

// init_mission_no_nav_commands - initialise a mission with no navigation commands
//      mission should ignore the jump that causes the endless loop and complete
void init_mission_no_nav_commands()
{
    AP_Mission::Mission_Command cmd;

    // clear mission
    mission.clear();

    // Command #0 : home
    cmd.id = MAV_CMD_NAV_WAYPOINT;
    cmd.content.location.options = 0;
    cmd.content.location.p1 = 0;
    cmd.content.location.alt = 0;
    cmd.content.location.lat = 12345678;
    cmd.content.location.lng = 23456789;
    if (!mission.add_cmd(cmd)) {
        hal.console->printf_P(PSTR("failed to add command\n"));
    }

    // Command #1 : "do" command
    cmd.id = MAV_CMD_DO_SET_ROI;
    cmd.content.location.options = 0;
    cmd.content.location.p1 = 0;
    cmd.content.location.alt = 11;
    cmd.content.location.lat = 12345678;
    cmd.content.location.lng = 23456789;
    if (!mission.add_cmd(cmd)) {
        hal.console->printf_P(PSTR("failed to add command\n"));
    }

    // Command #2 : "do" command
    cmd.id = MAV_CMD_DO_CHANGE_SPEED;
    cmd.content.location.options = 0;
    cmd.content.location.p1 = 0;
    cmd.content.location.alt = 0;
    cmd.content.location.lat = 0;
    cmd.content.location.lng = 0;
    if (!mission.add_cmd(cmd)) {
        hal.console->printf_P(PSTR("failed to add command\n"));
    }

    // Command #3 : "do" command
    cmd.id = MAV_CMD_DO_SET_SERVO;
    if (!mission.add_cmd(cmd)) {
        hal.console->printf_P(PSTR("failed to add command\n"));
    }

    // Command #4 : do-jump to first command 3 times
    cmd.id = MAV_CMD_DO_JUMP;
    cmd.content.jump.target = 1;
    cmd.content.jump.num_times = 1;
    if (!mission.add_cmd(cmd)) {
        hal.console->printf_P(PSTR("failed to add command\n"));
    }
}

// init_mission_endless_loop - initialise a mission with a do-jump that causes an endless loop
//      mission should start the first do command but then complete
void init_mission_endless_loop()
{
    AP_Mission::Mission_Command cmd;

    // clear mission
    mission.clear();

    // Command #0 : home
    cmd.id = MAV_CMD_NAV_WAYPOINT;
    cmd.content.location.options = 0;
    cmd.content.location.p1 = 0;
    cmd.content.location.alt = 0;
    cmd.content.location.lat = 12345678;
    cmd.content.location.lng = 23456789;
    if (!mission.add_cmd(cmd)) {
        hal.console->printf_P(PSTR("failed to add command\n"));
    }

    // Command #1 : do-jump command to itself
    cmd.id = MAV_CMD_DO_JUMP;
    cmd.content.jump.target = 1;
    cmd.content.jump.num_times = 2;
    if (!mission.add_cmd(cmd)) {
        hal.console->printf_P(PSTR("failed to add command\n"));
    }

    // Command #2 : take-off to 10m
    cmd.id = MAV_CMD_NAV_TAKEOFF;
    cmd.content.location.options = 0;
    cmd.content.location.p1 = 0;
    cmd.content.location.alt = 10;
    cmd.content.location.lat = 0;
    cmd.content.location.lng = 0;
    if (!mission.add_cmd(cmd)) {
        hal.console->printf_P(PSTR("failed to add command\n"));
    }

    // Command #3 : waypoint
    cmd.id = MAV_CMD_NAV_WAYPOINT;
    cmd.content.location.options = 0;
    cmd.content.location.p1 = 0;
    cmd.content.location.alt = 11;
    cmd.content.location.lat = 12345678;
    cmd.content.location.lng = 23456789;
    if (!mission.add_cmd(cmd)) {
        hal.console->printf_P(PSTR("failed to add command\n"));
    }
}

// init_mission_jump_to_nonnav - initialise a mission with a do-jump to the previous command which is a "do" command
//      ideally we would execute this previous "do" command the number of times specified in the do-jump command but this is tricky so we ignore the do-jump
//      mission should run the "do" command once and then complete
void init_mission_jump_to_nonnav()
{
    AP_Mission::Mission_Command cmd;

    // clear mission
    mission.clear();

    // Command #0 : home
    cmd.id = MAV_CMD_NAV_WAYPOINT;
    cmd.content.location.options = 0;
    cmd.content.location.p1 = 0;
    cmd.content.location.alt = 0;
    cmd.content.location.lat = 12345678;
    cmd.content.location.lng = 23456789;
    if (!mission.add_cmd(cmd)) {
        hal.console->printf_P(PSTR("failed to add command\n"));
    }

    // Command #1 : take-off to 10m
    cmd.id = MAV_CMD_NAV_TAKEOFF;
    cmd.content.location.options = 0;
    cmd.content.location.p1 = 0;
    cmd.content.location.alt = 10;
    cmd.content.location.lat = 0;
    cmd.content.location.lng = 0;
    if (!mission.add_cmd(cmd)) {
        hal.console->printf_P(PSTR("failed to add command\n"));
    }

    // Command #2 : do-roi command
    cmd.id = MAV_CMD_DO_SET_ROI;
    cmd.content.location.options = 0;
    cmd.content.location.p1 = 0;
    cmd.content.location.alt = 11;
    cmd.content.location.lat = 12345678;
    cmd.content.location.lng = 23456789;
    if (!mission.add_cmd(cmd)) {
        hal.console->printf_P(PSTR("failed to add command\n"));
    }

    // Command #3 : do-jump command to #2
    cmd.id = MAV_CMD_DO_JUMP;
    cmd.content.jump.target = 2;
    cmd.content.jump.num_times = 2;
    if (!mission.add_cmd(cmd)) {
        hal.console->printf_P(PSTR("failed to add command\n"));
    }

    // Command #4 : waypoint
    cmd.id = MAV_CMD_NAV_WAYPOINT;
    cmd.content.location.options = 0;
    cmd.content.location.p1 = 0;
    cmd.content.location.alt = 22;
    cmd.content.location.lat = 12345678;
    cmd.content.location.lng = 23456789;
    if (!mission.add_cmd(cmd)) {
        hal.console->printf_P(PSTR("failed to add command\n"));
    }
}

// init_mission_starts_with_do_commands - initialise a mission which starts with do comamnds
//      first command to execute should be the first do command followed by the first nav command
//      second do command should execute after 1st do command completes
//      third do command (which is after 1st nav command) should start after 1st nav command completes
void init_mission_starts_with_do_commands()
{
    AP_Mission::Mission_Command cmd;

    // clear mission
    mission.clear();

    // Command #0 : home
    cmd.id = MAV_CMD_NAV_WAYPOINT;
    cmd.content.location.options = 0;
    cmd.content.location.p1 = 0;
    cmd.content.location.alt = 0;
    cmd.content.location.lat = 12345678;
    cmd.content.location.lng = 23456789;
    if (!mission.add_cmd(cmd)) {
        hal.console->printf_P(PSTR("failed to add command\n"));
    }

    // Command #1 : First "do" command
    cmd.id = MAV_CMD_DO_SET_ROI;
    cmd.content.location.options = 0;
    cmd.content.location.p1 = 0;
    cmd.content.location.alt = 11;
    cmd.content.location.lat = 12345678;
    cmd.content.location.lng = 23456789;
    if (!mission.add_cmd(cmd)) {
        hal.console->printf_P(PSTR("failed to add command\n"));
    }

    // Command #2 : Second "do" command
    cmd.id = MAV_CMD_DO_CHANGE_SPEED;
    cmd.content.location.options = 0;
    cmd.content.location.p1 = 0;
    cmd.content.location.alt = 0;
    cmd.content.location.lat = 0;
    cmd.content.location.lng = 0;
    if (!mission.add_cmd(cmd)) {
        hal.console->printf_P(PSTR("failed to add command\n"));
    }

    // Command #3 : take-off to 10m
    cmd.id = MAV_CMD_NAV_TAKEOFF;
    cmd.content.location.options = 0;
    cmd.content.location.p1 = 0;
    cmd.content.location.alt = 10;
    cmd.content.location.lat = 0;
    cmd.content.location.lng = 0;
    if (!mission.add_cmd(cmd)) {
        hal.console->printf_P(PSTR("failed to add command\n"));
    }

    // Command #4 : Third "do" command
    cmd.id = MAV_CMD_DO_SET_ROI;
    cmd.content.location.options = 0;
    cmd.content.location.p1 = 0;
    cmd.content.location.alt = 22;
    cmd.content.location.lat = 12345678;
    cmd.content.location.lng = 23456789;
    if (!mission.add_cmd(cmd)) {
        hal.console->printf_P(PSTR("failed to add command\n"));
    }

    // Command #5 : waypoint
    cmd.id = MAV_CMD_NAV_WAYPOINT;
    cmd.content.location.options = 0;
    cmd.content.location.p1 = 0;
    cmd.content.location.alt = 33;
    cmd.content.location.lat = 12345678;
    cmd.content.location.lng = 23456789;
    if (!mission.add_cmd(cmd)) {
        hal.console->printf_P(PSTR("failed to add command\n"));
    }
}

// init_mission_ends_with_do_commands - initialise a mission which ends with do comamnds
//      a single do command just after nav command will be started but not verified because mission will complete
//      final do command will not be started
void init_mission_ends_with_do_commands()
{
    AP_Mission::Mission_Command cmd;

    // clear mission
    mission.clear();

    // Command #0 : home
    cmd.id = MAV_CMD_NAV_WAYPOINT;
    cmd.content.location.options = 0;
    cmd.content.location.p1 = 0;
    cmd.content.location.alt = 0;
    cmd.content.location.lat = 12345678;
    cmd.content.location.lng = 23456789;
    if (!mission.add_cmd(cmd)) {
        hal.console->printf_P(PSTR("failed to add command\n"));
    }

    // Command #1 : take-off to 10m
    cmd.id = MAV_CMD_NAV_TAKEOFF;
    cmd.content.location.options = 0;
    cmd.content.location.p1 = 0;
    cmd.content.location.alt = 10;
    cmd.content.location.lat = 0;
    cmd.content.location.lng = 0;
    if (!mission.add_cmd(cmd)) {
        hal.console->printf_P(PSTR("failed to add command\n"));
    }

    // Command #2 : "do" command
    cmd.id = MAV_CMD_DO_SET_ROI;
    cmd.content.location.options = 0;
    cmd.content.location.p1 = 0;
    cmd.content.location.alt = 22;
    cmd.content.location.lat = 12345678;
    cmd.content.location.lng = 23456789;
    if (!mission.add_cmd(cmd)) {
        hal.console->printf_P(PSTR("failed to add command\n"));
    }

    // Command #3 : waypoint
    cmd.id = MAV_CMD_NAV_WAYPOINT;
    cmd.content.location.options = 0;
    cmd.content.location.p1 = 0;
    cmd.content.location.alt = 33;
    cmd.content.location.lat = 12345678;
    cmd.content.location.lng = 23456789;
    if (!mission.add_cmd(cmd)) {
        hal.console->printf_P(PSTR("failed to add command\n"));
    }

    // Command #4 : "do" command after last nav command (but not at end of mission)
    cmd.id = MAV_CMD_DO_CHANGE_SPEED;
    cmd.content.location.options = 0;
    cmd.content.location.p1 = 0;
    cmd.content.location.alt = 0;
    cmd.content.location.lat = 0;
    cmd.content.location.lng = 0;
    if (!mission.add_cmd(cmd)) {
        hal.console->printf_P(PSTR("failed to add command\n"));
    }

    // Command #5 : "do" command at end of mission
    cmd.id = MAV_CMD_DO_SET_ROI;
    cmd.content.location.options = 0;
    cmd.content.location.p1 = 0;
    cmd.content.location.alt = 22;
    cmd.content.location.lat = 12345678;
    cmd.content.location.lng = 23456789;
    if (!mission.add_cmd(cmd)) {
        hal.console->printf_P(PSTR("failed to add command\n"));
    }
}

// init_mission_ends_with_jump_command - initialise a mission which ends with a jump comamnd
//      mission should complete after the do-jump is executed the appropriate number of times
void init_mission_ends_with_jump_command()
{
    AP_Mission::Mission_Command cmd;

    // clear mission
    mission.clear();

    // Command #0 : home
    cmd.id = MAV_CMD_NAV_WAYPOINT;
    cmd.content.location.options = 0;
    cmd.content.location.p1 = 0;
    cmd.content.location.alt = 0;
    cmd.content.location.lat = 12345678;
    cmd.content.location.lng = 23456789;
    if (!mission.add_cmd(cmd)) {
        hal.console->printf_P(PSTR("failed to add command\n"));
    }

    // Command #1 : take-off to 10m
    cmd.id = MAV_CMD_NAV_TAKEOFF;
    cmd.content.location.options = 0;
    cmd.content.location.p1 = 0;
    cmd.content.location.alt = 10;
    cmd.content.location.lat = 0;
    cmd.content.location.lng = 0;
    if (!mission.add_cmd(cmd)) {
        hal.console->printf_P(PSTR("failed to add command\n"));
    }

    // Command #2 : "do" command
    cmd.id = MAV_CMD_DO_SET_ROI;
    cmd.content.location.options = 0;
    cmd.content.location.p1 = 0;
    cmd.content.location.alt = 22;
    cmd.content.location.lat = 12345678;
    cmd.content.location.lng = 23456789;
    if (!mission.add_cmd(cmd)) {
        hal.console->printf_P(PSTR("failed to add command\n"));
    }

    // Command #3 : waypoint
    cmd.id = MAV_CMD_NAV_WAYPOINT;
    cmd.content.location.options = 0;
    cmd.content.location.p1 = 0;
    cmd.content.location.alt = 33;
    cmd.content.location.lat = 12345678;
    cmd.content.location.lng = 23456789;
    if (!mission.add_cmd(cmd)) {
        hal.console->printf_P(PSTR("failed to add command\n"));
    }

    // Command #4 : "do" command after last nav command (but not at end of mission)
    cmd.id = MAV_CMD_DO_CHANGE_SPEED;
    cmd.content.location.options = 0;
    cmd.content.location.p1 = 0;
    cmd.content.location.alt = 0;
    cmd.content.location.lat = 0;
    cmd.content.location.lng = 0;
    if (!mission.add_cmd(cmd)) {
        hal.console->printf_P(PSTR("failed to add command\n"));
    }

    // Command #5 : "do" command at end of mission
    cmd.id = MAV_CMD_DO_SET_ROI;
    cmd.content.location.options = 0;
    cmd.content.location.p1 = 0;
    cmd.content.location.alt = 22;
    cmd.content.location.lat = 12345678;
    cmd.content.location.lng = 23456789;
    if (!mission.add_cmd(cmd)) {
        hal.console->printf_P(PSTR("failed to add command\n"));
    }

    // Command #6 : do-jump command to #2 two times
    cmd.id = MAV_CMD_DO_JUMP;
    cmd.content.jump.target = 3;
    cmd.content.jump.num_times = 2;
    if (!mission.add_cmd(cmd)) {
        hal.console->printf_P(PSTR("failed to add command\n"));
    }
}

// print_mission - print out the entire mission to the console
void print_mission()
{
    AP_Mission::Mission_Command cmd;

    // check for empty mission
    if (mission.num_commands() == 0) {
        hal.console->printf_P(PSTR("No Mission!\n"));
        return;
    }

    hal.console->printf_P(PSTR("Mission: %d commands\n"),(int)mission.num_commands());

    // print each command
    for(uint16_t i=0; i<mission.num_commands(); i++) {
        // get next command from eeprom
        mission.read_cmd_from_storage(i,cmd);

        // print command position in list and mavlink id
        hal.console->printf_P(PSTR("Cmd#%d mav-id:%d "), (int)cmd.index, (int)cmd.id);

        // print whether nav or do command
        if (AP_Mission::is_nav_cmd(cmd)) {
            hal.console->printf_P(PSTR("Nav "));
        }else{
            hal.console->printf_P(PSTR("Do "));
        }

        // print command contents
        if (cmd.id == MAV_CMD_DO_JUMP) {
            hal.console->printf_P(PSTR("jump-to:%d num_times:%d\n"), (int)cmd.content.jump.target, (int)cmd.content.jump.num_times);
        }else{
            hal.console->printf_P(PSTR("p1:%d lat:%ld lng:%ld alt:%ld\n"),(int)cmd.content.location.p1, (long)cmd.content.location.lat, (long)cmd.content.location.lng, (long)cmd.content.location.alt);
        }
    }
    hal.console->printf_P(PSTR("--------\n"));
}

AP_HAL_MAIN();
