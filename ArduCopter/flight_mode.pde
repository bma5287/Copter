/// -*- tab-width: 4; Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*-

/*
 * flight.pde - high level calls to set and update flight modes
 *      logic for individual flight modes is in control_acro.pde, control_stabilize.pde, etc
 */

// set_mode - change flight mode and perform any necessary initialisation
// optional force parameter used to force the flight mode change (used only first time mode is set)
// returns true if mode was succesfully set
// ACRO, STABILIZE, ALTHOLD, LAND, DRIFT and SPORT can always be set successfully but the return state of other flight modes should be checked and the caller should deal with failures appropriately
static bool set_mode(uint8_t mode)
{
    // boolean to record if flight mode could be set
    bool success = false;
    bool ignore_checks = !motors.armed();   // allow switching to any mode if disarmed.  We rely on the arming check to perform

    // return immediately if we are already in the desired mode
    if (mode == control_mode) {
        return true;
    }

    switch(mode) {
        case ACRO:
            #if FRAME_CONFIG == HELI_FRAME
                success = heli_acro_init(ignore_checks);
            #else
                success = acro_init(ignore_checks);
            #endif
            break;

        case STABILIZE:
            #if FRAME_CONFIG == HELI_FRAME
                success = heli_stabilize_init(ignore_checks);
            #else
                success = stabilize_init(ignore_checks);
            #endif
            break;

        case ALT_HOLD:
            success = althold_init(ignore_checks);
            break;

        case AUTO:
            success = auto_init(ignore_checks);
            break;

        case CIRCLE:
            success = circle_init(ignore_checks);
            break;

        case LOITER:
            success = loiter_init(ignore_checks);
            break;

        case GUIDED:
            success = guided_init(ignore_checks);
            break;

        case LAND:
            success = land_init(ignore_checks);
            break;

        case RTL:
            success = rtl_init(ignore_checks);
            break;

        case OF_LOITER:
            success = ofloiter_init(ignore_checks);
            break;

        case DRIFT:
            success = drift_init(ignore_checks);
            break;

        case SPORT:
            success = sport_init(ignore_checks);
            break;

        case FLIP:
            success = flip_init(ignore_checks);
            break;

#if AUTOTUNE_ENABLED == ENABLED
        case AUTOTUNE:
            success = autotune_init(ignore_checks);
            break;
#endif

        case HYBRID:
            success = hybrid_init(ignore_checks);
            break;

        default:
            success = false;
            break;
    }

    // update flight mode
    if (success) {
        // perform any cleanup required by previous flight mode
        exit_mode(control_mode, mode);
        control_mode = mode;
        Log_Write_Mode(control_mode);
    }else{
        // Log error that we failed to enter desired flight mode
        Log_Write_Error(ERROR_SUBSYSTEM_FLIGHT_MODE,mode);
    }

    // return success or failure
    return success;
}

// update_flight_mode - calls the appropriate attitude controllers based on flight mode
// called at 100hz or more
static void update_flight_mode()
{
    switch (control_mode) {
        case ACRO:
            #if FRAME_CONFIG == HELI_FRAME
                heli_acro_run();
            #else
                acro_run();
            #endif
            break;

        case STABILIZE:
            #if FRAME_CONFIG == HELI_FRAME
                heli_stabilize_run();
            #else
                stabilize_run();
            #endif
            break;

        case ALT_HOLD:
            althold_run();
            break;

        case AUTO:
            auto_run();
            break;

        case CIRCLE:
            circle_run();
            break;

        case LOITER:
            loiter_run();
            break;

        case GUIDED:
            guided_run();
            break;

        case LAND:
            land_run();
            break;

        case RTL:
            rtl_run();
            break;

        case OF_LOITER:
            ofloiter_run();
            break;

        case DRIFT:
            drift_run();
            break;

        case SPORT:
            sport_run();
            break;

        case FLIP:
            flip_run();
            break;

#if AUTOTUNE_ENABLED == ENABLED
        case AUTOTUNE:
            autotune_run();
            break;
#endif

        case HYBRID:
            hybrid_run();
            break;
    }
}

// exit_mode - high level call to organise cleanup as a flight mode is exited
static void exit_mode(uint8_t old_control_mode, uint8_t new_control_mode)
{
#if AUTOTUNE_ENABLED == ENABLED
    if (old_control_mode == AUTOTUNE) {
        autotune_stop();
    }
#endif

    // smooth throttle transition when switching from manual to automatic flight modes
    if (manual_flight_mode(old_control_mode) && !manual_flight_mode(new_control_mode) && motors.armed() && !ap.land_complete) {
        // this assumes all manual flight modes use get_pilot_desired_throttle to translate pilot input to output throttle
        set_accel_throttle_I_from_pilot_throttle(get_pilot_desired_throttle(g.rc_3.control_in));
    }
}

// returns true or false whether mode requires GPS
static bool mode_requires_GPS(uint8_t mode) {
    switch(mode) {
        case AUTO:
        case GUIDED:
        case LOITER:
        case RTL:
        case CIRCLE:
        case DRIFT:
        case HYBRID:
            return true;
        default:
            return false;
    }

    return false;
}

// manual_flight_mode - returns true if flight mode is completely manual (i.e. roll, pitch, yaw and throttle are controlled by pilot)
static bool manual_flight_mode(uint8_t mode) {
    switch(mode) {
        case ACRO:
        case STABILIZE:
        case DRIFT:
        case SPORT:
            return true;
        default:
            return false;
    }

    return false;
}

//
// print_flight_mode - prints flight mode to serial port.
//
static void
print_flight_mode(AP_HAL::BetterStream *port, uint8_t mode)
{
    switch (mode) {
    case STABILIZE:
        port->print_P(PSTR("STABILIZE"));
        break;
    case ACRO:
        port->print_P(PSTR("ACRO"));
        break;
    case ALT_HOLD:
        port->print_P(PSTR("ALT_HOLD"));
        break;
    case AUTO:
        port->print_P(PSTR("AUTO"));
        break;
    case GUIDED:
        port->print_P(PSTR("GUIDED"));
        break;
    case LOITER:
        port->print_P(PSTR("LOITER"));
        break;
    case RTL:
        port->print_P(PSTR("RTL"));
        break;
    case CIRCLE:
        port->print_P(PSTR("CIRCLE"));
        break;
    case LAND:
        port->print_P(PSTR("LAND"));
        break;
    case OF_LOITER:
        port->print_P(PSTR("OF_LOITER"));
        break;
    case DRIFT:
        port->print_P(PSTR("DRIFT"));
        break;
    case SPORT:
        port->print_P(PSTR("SPORT"));
        break;
    case FLIP:
        port->print_P(PSTR("FLIP"));
        break;
    case AUTOTUNE:
        port->print_P(PSTR("AUTOTUNE"));
        break;
    case HYBRID:
        port->print_P(PSTR("HYBRID"));
        break;
    default:
        port->printf_P(PSTR("Mode(%u)"), (unsigned)mode);
        break;
    }
}

// get_angle_targets_for_reporting() - returns 3d vector of roll, pitch and yaw target angles for logging and reporting to GCS
static void get_angle_targets_for_reporting(Vector3f& targets)
{
    targets = attitude_control.angle_ef_targets();
}
