# fly ArduCopter in SIL

import util, pexpect, sys, time, math, shutil, os
from common import *

# get location of scripts
testdir=os.path.dirname(os.path.realpath(__file__))

sys.path.insert(0, util.reltopdir('../pymavlink'))
import mavutil, mavwp

HOME_LOCATION='-35.362938,149.165085,584,270'

homeloc = None
num_wp = 0

def arm_motors(mavproxy, mav):
    '''arm motors'''
    print("Arming motors")
    mavproxy.send('switch 6\n') # stabilize mode
    mav.wait_mode('STABILIZE')
    mavproxy.send('rc 3 1000\n')
    mavproxy.send('rc 4 2000\n')
    mavproxy.expect('APM: ARMING MOTORS')
    mavproxy.send('rc 4 1500\n')
    print("MOTORS ARMED OK")
    return True

def disarm_motors(mavproxy, mav):
    '''disarm motors'''
    print("Disarming motors")
    mavproxy.send('switch 6\n') # stabilize mode
    mavproxy.send('rc 3 1000\n')
    mavproxy.send('rc 4 1000\n')
    mavproxy.expect('APM: DISARMING MOTORS')
    mavproxy.send('rc 4 1500\n')
    print("MOTORS DISARMED OK")
    return True


def takeoff(mavproxy, mav):
    '''takeoff get to 30m altitude'''
    mavproxy.send('switch 6\n') # stabilize mode
    mav.wait_mode('STABILIZE')
    mavproxy.send('rc 3 1500\n')
    wait_altitude(mav, 30, 40)
    print("TAKEOFF COMPLETE")
    return True


def loiter(mavproxy, mav, maxaltchange=10, holdtime=10, timeout=60):
    '''hold loiter position'''
    mavproxy.send('switch 5\n') # loiter mode
    mav.wait_mode('LOITER')
    m = mav.recv_match(type='VFR_HUD', blocking=True)
    start_altitude = m.alt
    tstart = time.time()
    tholdstart = time.time()
    print("Holding loiter at %u meters for %u seconds" % (start_altitude, holdtime))
    while time.time() < tstart + timeout:
        m = mav.recv_match(type='VFR_HUD', blocking=True)
        print("Altitude %u" % m.alt)
        if math.fabs(m.alt - start_altitude) > maxaltchange:
            tholdstart = time.time()
        if time.time() - tholdstart > holdtime:
            print("Loiter OK for %u seconds" % holdtime)
            return True
    print("Loiter FAILED")
    return False


def fly_square(mavproxy, mav, side=50, timeout=120):
    '''fly a square, flying N then E'''
    mavproxy.send('switch 6\n')
    mav.wait_mode('STABILIZE')
    tstart = time.time()
    failed = False

    print("Save WP 1")
    save_wp(mavproxy, mav)

    print("turn")
    mavproxy.send('rc 3 1430\n')
    mavproxy.send('rc 4 1610\n')
    if not wait_heading(mav, 0):
        return False
    mavproxy.send('rc 4 1500\n')

    print("Going north %u meters" % side)
    mavproxy.send('rc 2 1390\n')
    if not wait_distance(mav, side):
        failed = True
    mavproxy.send('rc 2 1500\n')

    print("Save WP 2")
    save_wp(mavproxy, mav)

    print("Going east %u meters" % side)
    mavproxy.send('rc 1 1610\n')
    if not wait_distance(mav, side):
        failed = True
    mavproxy.send('rc 1 1500\n')

    print("Save WP 3")
    save_wp(mavproxy, mav)

    print("Going south %u meters" % side)
    mavproxy.send('rc 2 1610\n')
    if not wait_distance(mav, side):
        failed = True
    mavproxy.send('rc 2 1500\n')
    mav.recv_match(condition='RC_CHANNELS_RAW.chan7_raw==1000', blocking=True)

    print("Save WP 4")
    save_wp(mavproxy, mav)

    print("Going west %u meters" % side)
    mavproxy.send('rc 1 1390\n')
    if not wait_distance(mav, side):
        failed = True
    mavproxy.send('rc 1 1500\n')

    print("Save WP 5")
    save_wp(mavproxy, mav)

    return not failed




def land(mavproxy, mav, timeout=60):
    '''land the quad'''
    print("STARTING LANDING")
    mavproxy.send('switch 6\n')
    mav.wait_mode('STABILIZE')

    # start by dropping throttle till we have lost 5m
    mavproxy.send('rc 3 1380\n')
    m = mav.recv_match(type='VFR_HUD', blocking=True)
    wait_altitude(mav, 0, m.alt-5)

    # now let it settle gently
    mavproxy.send('rc 3 1400\n')
    tstart = time.time()

    if wait_altitude(mav, -5, 0):
        print("LANDING OK")
        return True
    else:
        print("LANDING FAILED")
        return False

def circle(mavproxy, mav, maxaltchange=10, holdtime=90, timeout=35):
    '''fly circle'''
    print("FLY CIRCLE")
    mavproxy.send('switch 1\n') # CIRCLE mode
    mavproxy.expect('CIRCLE>')
    mavproxy.send('status\n')
    mavproxy.expect('>')
    m = mav.recv_match(type='VFR_HUD', blocking=True)
    start_altitude = m.alt
    tstart = time.time()
    tholdstart = time.time()
    print("Circle at %u meters for %u seconds" % (start_altitude, holdtime))
    while time.time() < tstart + timeout:
        m = mav.recv_match(type='VFR_HUD', blocking=True)
        print("heading %u" % m.heading)

    print("CIRCLE OK for %u seconds" % holdtime)
    return True


def fly_mission(mavproxy, mav, height_accuracy=-1, target_altitude=None):
    '''fly a mission from a file'''
    print("Fly a mission")
    global homeloc
    global num_wp
    mavproxy.send('switch 4\n') # auto mode
    mavproxy.expect('AUTO>')

    wait_altitude(mav, 30, 40)
    if wait_waypoint(mav, 1, num_wp):
        print("MISSION COMPLETE")
        return True
    else:
        return False

    #if not wait_distance(mav, 30, timeout=120):
    #    return False
    #if not wait_location(mav, homeloc, timeout=600, target_altitude=target_altitude, height_accuracy=height_accuracy):
    #    return False

def load_mission(mavproxy, mav, filename):
    '''load a mission from a file'''
    global num_wp
    mavproxy.send('wp load %s\n' % filename)
    mavproxy.expect('flight plan received')
    mavproxy.send('wp list\n')
    mavproxy.expect('Requesting [0-9]+ waypoints')

    wploader = mavwp.MAVWPLoader()
    wploader.load(filename)
    num_wp = wploader.count()
    print("loaded mission")
    for i in range(num_wp):
        print (dir(wploader.wp(i)))

def setup_rc(mavproxy):
    '''setup RC override control'''
    for chan in range(1,9):
        mavproxy.send('rc %u 1500\n' % chan)
    # zero throttle
    mavproxy.send('rc 3 1000\n')


def fly_ArduCopter(viewerip=None):
    '''fly ArduCopter in SIL

    you can pass viewerip as an IP address to optionally send fg and
    mavproxy packets too for local viewing of the flight in real time
    '''
    global expect_list, homeloc

    sil = util.start_SIL('ArduCopter', wipe=True)
    mavproxy = util.start_MAVProxy_SIL('ArduCopter')
    mavproxy.expect('Please Run Setup')

    # we need to restart it after eeprom erase
    util.pexpect_close(mavproxy)
    util.pexpect_close(sil)
    sil = util.start_SIL('ArduCopter')
    mavproxy = util.start_MAVProxy_SIL('ArduCopter', options='--fgout=127.0.0.1:5502 --fgin=127.0.0.1:5501 --out=127.0.0.1:19550 --quadcopter')
    mavproxy.expect('Received [0-9]+ parameters')

    # setup test parameters
    mavproxy.send("param load %s/ArduCopter.parm\n" % testdir)
    mavproxy.expect('Loaded [0-9]+ parameters')

    # reboot with new parameters
    util.pexpect_close(mavproxy)
    util.pexpect_close(sil)
    sil = util.start_SIL('ArduCopter')
    options = '--fgout=127.0.0.1:5502 --fgin=127.0.0.1:5501 --out=127.0.0.1:19550 --quadcopter --streamrate=1'
    if viewerip:
        options += ' --out=%s:14550' % viewerip
    mavproxy = util.start_MAVProxy_SIL('ArduCopter', options=options)
    mavproxy.expect('Logging to (\S+)')
    logfile = mavproxy.match.group(1)
    print("LOGFILE %s" % logfile)

    buildlog = util.reltopdir("../buildlogs/ArduCopter-test.mavlog")
    print("buildlog=%s" % buildlog)
    if os.path.exists(buildlog):
        os.unlink(buildlog)
    os.link(logfile, buildlog)

    mavproxy.expect("Ready to FLY")
    mavproxy.expect('Received [0-9]+ parameters')

    util.expect_setup_callback(mavproxy, expect_callback)

    # start hil_quad.py
    cmd = util.reltopdir('../HILTest/hil_quad.py') + ' --fgrate=200 --home=%s' % HOME_LOCATION
    if viewerip:
        cmd += ' --fgout=192.168.2.15:9123'
    hquad = pexpect.spawn(cmd, logfile=sys.stdout, timeout=10)
    util.pexpect_autoclose(hquad)
    hquad.expect('Starting at')

    expect_list.extend([hquad, sil, mavproxy])

    # get a mavlink connection going
    try:
        mav = mavutil.mavlink_connection('127.0.0.1:19550', robust_parsing=True)
    except Exception, msg:
        print("Failed to start mavlink connection on 127.0.0.1:19550" % msg)
        raise
    mav.message_hooks.append(message_hook)


    failed = False
    e = 'None'
    try:
        mav.wait_heartbeat()
        mav.recv_match(type='GPS_RAW', blocking=True)
        setup_rc(mavproxy)
        homeloc = current_location(mav)
        if not arm_motors(mavproxy, mav):
            failed = True

        if not takeoff(mavproxy, mav):
            failed = True

        if not fly_square(mavproxy, mav):
            failed = True

        if not loiter(mavproxy, mav):
            failed = True

        #Fly a circle for 60 seconds
        if not circle(mavproxy, mav):
            failed = True

        # fly the stores mission
        if not fly_mission(mavproxy, mav,height_accuracy = 0.5, target_altitude=10):
            failed = True

        #fly_mission(mavproxy, mav, os.path.join(testdir, "mission_ttt.txt"), height_accuracy=0.2)

        if not load_mission(mavproxy, mav, os.path.join(testdir, "mission_ttt.txt")):
            failed = True

        if not fly_mission(mavproxy, mav,height_accuracy = 0.5, target_altitude=10):
            failed = True

        if not land(mavproxy, mav):
            failed = True

        if not disarm_motors(mavproxy, mav):
            failed = True
    except pexpect.TIMEOUT, e:
        failed = True

    util.pexpect_close(mavproxy)
    util.pexpect_close(sil)
    util.pexpect_close(hquad)

    if os.path.exists('ArduCopter-valgrind.log'):
        os.chmod('ArduCopter-valgrind.log', 0644)
        shutil.copy("ArduCopter-valgrind.log", util.reltopdir("../buildlogs/ArduCopter-valgrind.log"))

    if failed:
        print("FAILED: %s" % e)
        return False
    return True
