/// -*- tab-width: 4; Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*-

#include <AP_HAL/AP_HAL.h>

#if HAL_CPU_CLASS >= HAL_CPU_CLASS_150

#include "AP_NavEKF2.h"
#include "AP_NavEKF2_core.h"
#include <AP_AHRS/AP_AHRS.h>
#include <AP_Vehicle/AP_Vehicle.h>

#include <stdio.h>

extern const AP_HAL::HAL& hal;


/* Monitor GPS data to see if quality is good enough to initialise the EKF
   Monitor magnetometer innovations to to see if the heading is good enough to use GPS
   Return true if all criteria pass for 10 seconds

   We also record the failure reason so that prearm_failure_reason()
   can give a good report to the user on why arming is failing
*/
bool NavEKF2_core::calcGpsGoodToAlign(void)
{
    // fail if reported speed accuracy greater than threshold
    bool gpsSpdAccFail = (gpsSpdAccuracy > 1.0f) && (frontend._gpsCheck & MASK_GPS_SPD_ERR);

    // Report check result as a text string and bitmask
    if (gpsSpdAccFail) {
        hal.util->snprintf(prearm_fail_string,
                           sizeof(prearm_fail_string),
                           "GPS speed error %.1f", (double)gpsSpdAccuracy);
        gpsCheckStatus.bad_sAcc = true;
    } else {
        gpsCheckStatus.bad_sAcc = false;
    }

    // fail if not enough sats
    bool numSatsFail = (_ahrs->get_gps().num_sats() < 6) && (frontend._gpsCheck & MASK_GPS_NSATS);

    // Report check result as a text string and bitmask
    if (numSatsFail) {
        hal.util->snprintf(prearm_fail_string, sizeof(prearm_fail_string),
                           "GPS numsats %u (needs 6)", _ahrs->get_gps().num_sats());
        gpsCheckStatus.bad_sats = true;
    } else {
        gpsCheckStatus.bad_sats = false;
    }

    // fail if satellite geometry is poor
    bool hdopFail = (_ahrs->get_gps().get_hdop() > 250)  && (frontend._gpsCheck & MASK_GPS_HDOP);

    // Report check result as a text string and bitmask
    if (hdopFail) {
        hal.util->snprintf(prearm_fail_string, sizeof(prearm_fail_string),
                           "GPS HDOP %.1f (needs 2.5)", (double)(0.01f * _ahrs->get_gps().get_hdop()));
        gpsCheckStatus.bad_hdop = true;
    } else {
        gpsCheckStatus.bad_hdop = false;
    }

    // fail if horiziontal position accuracy not sufficient
    float hAcc = 0.0f;
    bool hAccFail;
    if (_ahrs->get_gps().horizontal_accuracy(hAcc)) {
        hAccFail = (hAcc > 5.0f)  && (frontend._gpsCheck & MASK_GPS_POS_ERR);
    } else {
        hAccFail =  false;
    }

    // Report check result as a text string and bitmask
    if (hAccFail) {
        hal.util->snprintf(prearm_fail_string,
                           sizeof(prearm_fail_string),
                           "GPS horiz error %.1f", (double)hAcc);
        gpsCheckStatus.bad_hAcc = true;
    } else {
        gpsCheckStatus.bad_hAcc = false;
    }

    // If we have good magnetometer consistency and bad innovations for longer than 5 seconds then we reset heading and field states
    // This enables us to handle large changes to the external magnetic field environment that occur before arming
    if ((magTestRatio.x <= 1.0f && magTestRatio.y <= 1.0f) || !consistentMagData) {
        magYawResetTimer_ms = imuSampleTime_ms;
    }
    if (imuSampleTime_ms - magYawResetTimer_ms > 5000) {
        // reset heading and field states
        Vector3f eulerAngles;
        getEulerAngles(eulerAngles);
        stateStruct.quat = calcQuatAndFieldStates(eulerAngles.x, eulerAngles.y);
        // reset timer to ensure that bad magnetometer data cannot cause a heading reset more often than every 5 seconds
        magYawResetTimer_ms = imuSampleTime_ms;
    }

    // fail if magnetometer innovations are outside limits indicating bad yaw
    // with bad yaw we are unable to use GPS
    bool yawFail;
    if ((magTestRatio.x > 1.0f || magTestRatio.y > 1.0f) && (frontend._gpsCheck & MASK_GPS_YAW_ERR)) {
        yawFail = true;
    } else {
        yawFail = false;
    }

    // Report check result as a text string and bitmask
    if (yawFail) {
        hal.util->snprintf(prearm_fail_string,
                           sizeof(prearm_fail_string),
                           "Mag yaw error x=%.1f y=%.1f",
                           (double)magTestRatio.x,
                           (double)magTestRatio.y);
        gpsCheckStatus.bad_yaw = true;
    } else {
        gpsCheckStatus.bad_yaw = false;
    }

    // Check for significant change in GPS position if disarmed which indicates bad GPS
    // This check can only be used when the vehicle is stationary
    const struct Location &gpsloc = _ahrs->get_gps().location(); // Current location
    const float posFiltTimeConst = 10.0f; // time constant used to decay position drift
    // calculate time lapsesd since last update and limit to prevent numerical errors
    float deltaTime = constrain_float(float(imuDataDelayed.time_ms - lastPreAlignGpsCheckTime_ms)*0.001f,0.01f,posFiltTimeConst);
    lastPreAlignGpsCheckTime_ms = imuDataDelayed.time_ms;
    // Sum distance moved
    gpsDriftNE += location_diff(gpsloc_prev, gpsloc).length();
    gpsloc_prev = gpsloc;
    // Decay distance moved exponentially to zero
    gpsDriftNE *= (1.0f - deltaTime/posFiltTimeConst);
    // Clamp the fiter state to prevent excessive persistence of large transients
    gpsDriftNE = min(gpsDriftNE,10.0f);
    // Fail if more than 3 metres drift after filtering whilst on-ground
    // This corresponds to a maximum acceptable average drift rate of 0.3 m/s or single glitch event of 3m
    bool gpsDriftFail = (gpsDriftNE > 3.0f) && onGround && (frontend._gpsCheck & MASK_GPS_POS_DRIFT);

    // Report check result as a text string and bitmask
    if (gpsDriftFail) {
        hal.util->snprintf(prearm_fail_string,
                           sizeof(prearm_fail_string),
                           "GPS drift %.1fm (needs 3.0)", (double)gpsDriftNE);
        gpsCheckStatus.bad_horiz_drift = true;
    } else {
        gpsCheckStatus.bad_horiz_drift = false;
    }

    // Check that the vertical GPS vertical velocity is reasonable after noise filtering
    bool gpsVertVelFail;
    if (_ahrs->get_gps().have_vertical_velocity() && onGround) {
        // check that the average vertical GPS velocity is close to zero
        gpsVertVelFilt = 0.1f * gpsDataNew.vel.z + 0.9f * gpsVertVelFilt;
        gpsVertVelFilt = constrain_float(gpsVertVelFilt,-10.0f,10.0f);
        gpsVertVelFail = (fabsf(gpsVertVelFilt) > 0.3f) && (frontend._gpsCheck & MASK_GPS_VERT_SPD);
    } else if ((frontend._fusionModeGPS == 0) && !_ahrs->get_gps().have_vertical_velocity()) {
        // If the EKF settings require vertical GPS velocity and the receiver is not outputting it, then fail
        gpsVertVelFail = true;
    } else {
        gpsVertVelFail = false;
    }

    // Report check result as a text string and bitmask
    if (gpsVertVelFail) {
        hal.util->snprintf(prearm_fail_string,
                           sizeof(prearm_fail_string),
                           "GPS vertical speed %.2fm/s (needs 0.30)", (double)fabsf(gpsVertVelFilt));
        gpsCheckStatus.bad_vert_vel = true;
    } else {
        gpsCheckStatus.bad_vert_vel = false;
    }

    // Check that the horizontal GPS vertical velocity is reasonable after noise filtering
    // This check can only be used if the vehicle is stationary
    bool gpsHorizVelFail;
    if (onGround) {
        gpsHorizVelFilt = 0.1f * pythagorous2(gpsDataDelayed.vel.x,gpsDataDelayed.vel.y) + 0.9f * gpsHorizVelFilt;
        gpsHorizVelFilt = constrain_float(gpsHorizVelFilt,-10.0f,10.0f);
        gpsHorizVelFail = (fabsf(gpsHorizVelFilt) > 0.3f) && (frontend._gpsCheck & MASK_GPS_HORIZ_SPD);
    } else {
        gpsHorizVelFail = false;
    }

    // Report check result as a text string and bitmask
    if (gpsHorizVelFail) {
        hal.util->snprintf(prearm_fail_string,
                           sizeof(prearm_fail_string),
                           "GPS horizontal speed %.2fm/s (needs 0.30)", (double)gpsDriftNE);
        gpsCheckStatus.bad_horiz_vel = true;
    } else {
        gpsCheckStatus.bad_horiz_vel = false;
    }

    // record time of fail - assume  fail first time called
    if (gpsSpdAccFail || numSatsFail || hdopFail || hAccFail || yawFail || gpsDriftFail || gpsVertVelFail || gpsHorizVelFail || lastGpsVelFail_ms == 0) {
        lastGpsVelFail_ms = imuSampleTime_ms;
    }

    // continuous period without fail required to return a healthy status
    if (imuSampleTime_ms - lastGpsVelFail_ms > 10000) {
        return true;
    } else {
        hal.util->snprintf(prearm_fail_string, sizeof(prearm_fail_string), "EKF checking GPS");
        return false;
    }
}

// update inflight calculaton that determines if GPS data is good enough for reliable navigation
void NavEKF2_core::calcGpsGoodForFlight(void)
{
    // use a simple criteria based on the GPS receivers claimed speed accuracy and the EKF innovation consistency checks

    // set up varaibles and constants used by filter that is applied to GPS speed accuracy
    const float alpha1 = 0.2f; // coefficient for first stage LPF applied to raw speed accuracy data
    const float tau = 10.0f; // time constant (sec) of peak hold decay
    if (lastGpsCheckTime_ms == 0) {
        lastGpsCheckTime_ms =  imuSampleTime_ms;
    }
    float dtLPF = (imuSampleTime_ms - lastGpsCheckTime_ms) * 1e-3f;
    lastGpsCheckTime_ms = imuSampleTime_ms;
    float alpha2 = constrain_float(dtLPF/tau,0.0f,1.0f);

    // get the receivers reported speed accuracy
    float gpsSpdAccRaw;
    if (!_ahrs->get_gps().speed_accuracy(gpsSpdAccRaw)) {
        gpsSpdAccRaw = 0.0f;
    }

    // filter the raw speed accuracy using a LPF
    sAccFilterState1 = constrain_float((alpha1 * gpsSpdAccRaw + (1.0f - alpha1) * sAccFilterState1),0.0f,10.0f);

    // apply a peak hold filter to the LPF output
    sAccFilterState2 = max(sAccFilterState1,((1.0f - alpha2) * sAccFilterState2));

    // Apply a threshold test with hysteresis to the filtered GPS speed accuracy data
    if (sAccFilterState2 > 1.5f ) {
        gpsSpdAccPass = false;
    } else if(sAccFilterState2 < 1.0f) {
        gpsSpdAccPass = true;
    }

    // Apply a threshold test with hysteresis to the normalised position and velocity innovations
    // Require a fail for one second and a pass for 10 seconds to transition
    if (lastInnovFailTime_ms == 0) {
        lastInnovFailTime_ms = imuSampleTime_ms;
        lastInnovPassTime_ms = imuSampleTime_ms;
    }
    if (velTestRatio < 1.0f && posTestRatio < 1.0f) {
        lastInnovPassTime_ms = imuSampleTime_ms;
    } else if (velTestRatio > 0.7f || posTestRatio > 0.7f) {
        lastInnovFailTime_ms = imuSampleTime_ms;
    }
    if ((imuSampleTime_ms - lastInnovPassTime_ms) > 1000) {
        ekfInnovationsPass = false;
    } else if ((imuSampleTime_ms - lastInnovFailTime_ms) > 10000) {
        ekfInnovationsPass = true;
    }

    // both GPS speed accuracy and EKF innovations must pass
    gpsAccuracyGood = gpsSpdAccPass && ekfInnovationsPass;
}

// Detect if we are in flight or on ground
void NavEKF2_core::detectFlight()
{
    /*
        If we are a fly forward type vehicle (eg plane), then in-air status can be determined through a combination of speed and height criteria.
        Because of the differing certainty requirements of algorithms that need the in-flight / on-ground status we use two booleans where
        onGround indicates a high certainty we are not flying and inFlight indicates a high certainty we are flying. It is possible for
        both onGround and inFlight to be false if the status is uncertain, but they cannot both be true.

        If we are a plane as indicated by the assume_zero_sideslip() status, then different logic is used

        TODO - this logic should be moved out of the EKF and into the flight vehicle code.
    */

    if (assume_zero_sideslip()) {
        // To be confident we are in the air we use a criteria which combines arm status, ground speed, airspeed and height change
        float gndSpdSq = sq(gpsDataDelayed.vel.x) + sq(gpsDataDelayed.vel.y);
        bool highGndSpd = false;
        bool highAirSpd = false;
        bool largeHgtChange = false;

        // trigger at 8 m/s airspeed
        if (_ahrs->airspeed_sensor_enabled()) {
            const AP_Airspeed *airspeed = _ahrs->get_airspeed();
            if (airspeed->get_airspeed() * airspeed->get_EAS2TAS() > 10.0f) {
                highAirSpd = true;
            }
        }

        // trigger at 10 m/s GPS velocity, but not if GPS is reporting bad velocity errors
        if (gndSpdSq > 100.0f && gpsSpdAccuracy < 1.0f) {
            highGndSpd = true;
        }

        // trigger if more than 10m away from initial height
        if (fabsf(baroDataDelayed.hgt) > 10.0f) {
            largeHgtChange = true;
        }

        // Determine to a high certainty we are flying
        if (motorsArmed && highGndSpd && (highAirSpd || largeHgtChange)) {
            onGround = false;
            inFlight = true;
        }

        // if is possible we are in flight, set the time this condition was last detected
        if (motorsArmed && (highGndSpd || highAirSpd || largeHgtChange)) {
            airborneDetectTime_ms = imuSampleTime_ms;
            onGround = false;
        }

        // Determine to a high certainty we are not flying
        // after 5 seconds of not detecting a possible flight condition or we are disarmed, we transition to on-ground mode
        if(!motorsArmed || ((imuSampleTime_ms - airborneDetectTime_ms) > 5000)) {
            onGround = true;
            inFlight = false;
        }
    } else {
        // Non fly forward vehicle, so can only use height and motor arm status

        // If the motors are armed then we could be flying and if they are not armed then we are definitely not flying
        if (motorsArmed) {
            onGround = false;
        } else {
            inFlight = false;
            onGround = true;
        }

        // If height has increased since exiting on-ground, then we definitely are flying
        if (!onGround && ((stateStruct.position.z - posDownAtTakeoff) < -1.5f)) {
            inFlight = true;
        }

        // If rangefinder has increased since exiting on-ground, then we definitely are flying
        if (!onGround && ((rngMea - rngAtStartOfFlight) > 0.5f)) {
            inFlight = true;
        }

    }

    // store current on-ground  and in-air status for next time
    prevOnGround = onGround;
    prevInFlight = inFlight;

    // Store vehicle height and range prior to takeoff for use in post takeoff checks
    if (onGround && prevOnGround) {
        // store vertical position at start of flight to use as a reference for ground relative checks
        posDownAtTakeoff = stateStruct.position.z;
        // store the range finder measurement which will be used as a reference to detect when we have taken off
        rngAtStartOfFlight = rngMea;
    }

}


// determine if a takeoff is expected so that we can compensate for expected barometer errors due to ground effect
bool NavEKF2_core::getTakeoffExpected()
{
    if (expectGndEffectTakeoff && imuSampleTime_ms - takeoffExpectedSet_ms > frontend.gndEffectTimeout_ms) {
        expectGndEffectTakeoff = false;
    }

    return expectGndEffectTakeoff;
}

// called by vehicle code to specify that a takeoff is happening
// causes the EKF to compensate for expected barometer errors due to ground effect
void NavEKF2_core::setTakeoffExpected(bool val)
{
    takeoffExpectedSet_ms = imuSampleTime_ms;
    expectGndEffectTakeoff = val;
}


// determine if a touchdown is expected so that we can compensate for expected barometer errors due to ground effect
bool NavEKF2_core::getTouchdownExpected()
{
    if (expectGndEffectTouchdown && imuSampleTime_ms - touchdownExpectedSet_ms > frontend.gndEffectTimeout_ms) {
        expectGndEffectTouchdown = false;
    }

    return expectGndEffectTouchdown;
}

// called by vehicle code to specify that a touchdown is expected to happen
// causes the EKF to compensate for expected barometer errors due to ground effect
void NavEKF2_core::setTouchdownExpected(bool val)
{
    touchdownExpectedSet_ms = imuSampleTime_ms;
    expectGndEffectTouchdown = val;
}

// Detect takeoff for optical flow navigation
void NavEKF2_core::detectOptFlowTakeoff(void)
{
    if (!onGround && !takeOffDetected && (imuSampleTime_ms - timeAtArming_ms) > 1000) {
        // we are no longer confidently on the ground so check the range finder and gyro for signs of takeoff
        const AP_InertialSensor &ins = _ahrs->get_ins();
        Vector3f angRateVec;
        Vector3f gyroBias;
        getGyroBias(gyroBias);
        bool dual_ins = ins.get_gyro_health(0) && ins.get_gyro_health(1);
        if (dual_ins) {
            angRateVec = (ins.get_gyro(0) + ins.get_gyro(1)) * 0.5f - gyroBias;
        } else {
            angRateVec = ins.get_gyro() - gyroBias;
        }

        takeOffDetected = (takeOffDetected || (angRateVec.length() > 0.1f) || (rngMea > (rngAtStartOfFlight + 0.1f)));
    } else if (onGround) {
        // we are confidently on the ground so set the takeoff detected status to false
        takeOffDetected = false;
    }
}


#endif // HAL_CPU_CLASS
