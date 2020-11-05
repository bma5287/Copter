#!/usr/bin/env python

import pexpect, time, sys
from pymavlink import mavutil

def wait_heartbeat(mav, timeout=10):
    '''wait for a heartbeat'''
    start_time = time.time()
    while time.time() < start_time+timeout:
        if mav.recv_match(type='HEARTBEAT', blocking=True, timeout=0.5) is not None:
            return
    raise Exception("Failed to get heartbeat")    

def wait_mode(mav, modes, timeout=10):
    '''wait for one of a set of flight modes'''
    start_time = time.time()
    last_mode = None
    while time.time() < start_time+timeout:
        wait_heartbeat(mav, timeout=10)
        if mav.flightmode != last_mode:
            print("Flightmode %s" % mav.flightmode)
            last_mode = mav.flightmode
        if mav.flightmode in modes:
            return
    print("Failed to get mode from %s" % modes)
    sys.exit(1)

def wait_time(mav, simtime):
    '''wait for simulation time to pass'''
    imu = mav.recv_match(type='RAW_IMU', blocking=True)
    t1 = imu.time_usec*1.0e-6
    while True:
        imu = mav.recv_match(type='RAW_IMU', blocking=True)
        t2 = imu.time_usec*1.0e-6
        if t2 - t1 > simtime:
            break

cmd = '../Tools/autotest/sim_vehicle.py -D -f quadplane'
mavproxy = pexpect.spawn(cmd, logfile=sys.stdout, timeout=30)
mavproxy.expect("ArduPilot Ready")

mav = mavutil.mavlink_connection('127.0.0.1:14550')

mavproxy.send('speedup 40\n')
mavproxy.expect('using GPS')
mavproxy.expect('using GPS')
mavproxy.expect('using GPS')
mavproxy.expect('using GPS')
mavproxy.send('auto\n')
mavproxy.send('arm throttle\n')
wait_mode(mav, ['AUTO'])
mavproxy.expect("Mission: 5 WP")
