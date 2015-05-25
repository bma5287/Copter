#!/bin/bash

# home location lat, lon, alt, heading
LOCATION="CMAC"
TRACKER_LOCATION="CMAC_PILOTSBOX"
VEHICLE=""
BUILD_TARGET="sitl"
FRAME=""
NUM_PROCS=1
SPEEDUP="1"

# check the instance number to allow for multiple copies of the sim running at once
INSTANCE=0
USE_VALGRIND=0
USE_GDB=0
USE_GDB_STOPPED=0
DEBUG_BUILD=0
USE_MAVLINK_GIMBAL=0
CLEAN_BUILD=0
START_ANTENNA_TRACKER=0
WIPE_EEPROM=0
REVERSE_THROTTLE=0
NO_REBUILD=0
START_HIL=0
TRACKER_ARGS=""
EXTERNAL_SIM=0
MODEL=""
BREAKPOINT=""

usage()
{
cat <<EOF
Usage: sim_vehicle.sh [options] [mavproxy_options]
Options:
    -v VEHICLE       vehicle type (ArduPlane, ArduCopter or APMrover2)
                     vehicle type defaults to working directory
    -I INSTANCE      instance of simulator (default 0)
    -V               enable valgrind for memory access checking (very slow!)
    -G               use gdb for debugging ardupilot
    -g               use gdb for debugging ardupilot, but don't auto-start
    -D               build with debugging
    -B               add a breakpoint at given location in debugger
    -T               start an antenna tracker instance
    -A               pass arguments to antenna tracker
    -t               set antenna tracker start location
    -L               select start location from Tools/autotest/locations.txt
    -l               set the custom start location from -L
    -c               do a make clean before building
    -N               don't rebuild before starting ardupilot
    -w               wipe EEPROM and reload parameters
    -R               reverse throttle in plane
    -M               enable MAVLink gimbal
    -f FRAME         set aircraft frame type
                     for copters can choose +, X, quad or octa
                     for planes can choose elevon or vtail
    -j NUM_PROC      number of processors to use during build (default 1)
    -H               start HIL
    -e               use external simulator
    -S SPEEDUP       set simulation speedup (1 for wall clock time)

mavproxy_options:
    --map            start with a map
    --console        start with a status console
    --out DEST       start MAVLink output to DEST

Note: 
    eeprom.bin in the starting directory contains the parameters for your 
    simulated vehicle. Always start from the same directory. It is recommended that 
    you start in the main vehicle directory for the vehicle you are simulating, 
    for example, start in the ArduPlane directory to simulate ArduPlane
EOF
}


# parse options. Thanks to http://wiki.bash-hackers.org/howto/getopts_tutorial
while getopts ":I:VgGcj:TA:t:L:l:v:hwf:RNHeMS:DB:" opt; do
  case $opt in
    v)
      VEHICLE=$OPTARG
      ;;
    I)
      INSTANCE=$OPTARG
      ;;
    V)
      USE_VALGRIND=1
      ;;
    N)
      NO_REBUILD=1
      ;;
    H)
      START_HIL=1
      NO_REBUILD=1
      ;;
    T)
      START_ANTENNA_TRACKER=1
      ;;
    A)
      TRACKER_ARGS="$OPTARG"
      ;;
    R)
      REVERSE_THROTTLE=1
      ;;
    G)
      USE_GDB=1
      ;;
    D)
      DEBUG_BUILD=1
      ;;
    B)
      BREAKPOINT="$OPTARG"
      ;;
    M)
      USE_MAVLINK_GIMBAL=1
      ;;
    g)
      USE_GDB=1
      USE_GDB_STOPPED=1
      ;;
    L)
      LOCATION="$OPTARG"
      ;;
    l)
      CUSTOM_LOCATION="$OPTARG"
      ;;
    f)
      FRAME="$OPTARG"
      ;;
    S)
      SPEEDUP="$OPTARG"
      ;;
    t)
      TRACKER_LOCATION="$OPTARG"
      ;;
    c)
      CLEAN_BUILD=1
      ;;
    j)
      NUM_PROCS=$OPTARG
      ;;
    w)
      WIPE_EEPROM=1
      ;;
    e)
      EXTERNAL_SIM=1
      ;;
    h)
      usage
      exit 0
      ;;
    \?)
      # allow other args to pass on to mavproxy
      break
      ;;
    :)
      echo "Option -$OPTARG requires an argument." >&2
      usage
      exit 1
  esac
done
shift $((OPTIND-1))

# kill existing copy if this is the '0' instance only
kill_tasks() 
{
    [ "$INSTANCE" -eq "0" ] && {
        killall -q JSBSim lt-JSBSim ArduPlane.elf ArduCopter.elf APMrover2.elf AntennaTracker.elf
        pkill -f runsim.py
        pkill -f sim_tracker.py
    }
}

if [ $START_HIL == 0 ]; then
kill_tasks
fi

trap kill_tasks SIGINT

# setup ports for this instance
MAVLINK_PORT="tcp:127.0.0.1:"$((5760+10*$INSTANCE))
SIMIN_PORT="127.0.0.1:"$((5502+10*$INSTANCE))
SIMOUT_PORT="127.0.0.1:"$((5501+10*$INSTANCE))
FG_PORT="127.0.0.1:"$((5503+10*$INSTANCE))

set -x

[ -z "$VEHICLE" ] && {
    VEHICLE=$(basename $PWD)
}

[ -z "$FRAME" -a "$VEHICLE" = "APMrover2" ] && {
    FRAME="rover"
}

[ -z "$FRAME" -a "$VEHICLE" = "ArduPlane" ] && {
    FRAME="jsbsim"
}
[ -z "$FRAME" -a "$VEHICLE" = "ArduCopter" ] && {
    FRAME="quad"
}

EXTRA_PARM=""
EXTRA_SIM=""

[ "$SPEEDUP" != "1" ] && {
    EXTRA_SIM="$EXTRA_SIM --speedup=$SPEEDUP"
}

check_jsbsim_version()
{
    jsbsim_version=$(JSBSim --version)
    if [[ $jsbsim_version != *"ArduPilot"* ]]
    then
        cat <<EOF
=========================================================
You need the latest ArduPilot version of JSBSim installed
and in your \$PATH

Please get it from git://github.com/tridge/jsbsim.git
See 
  http://dev.ardupilot.com/wiki/simulation-2/sitl-simulator-software-in-the-loop/setting-up-sitl-on-linux/ 
for more details
=========================================================
EOF
        exit 1
    fi
}


# modify build target based on copter frame type
case $FRAME in
    +|quad)
	BUILD_TARGET="sitl"
        EXTRA_SIM="$EXTRA_SIM --frame=quad"
        MODEL="+"
	;;
    X)
	BUILD_TARGET="sitl"
        EXTRA_PARM="param set FRAME 1;"
        EXTRA_SIM="$EXTRA_SIM --frame=X"
        MODEL="X"
	;;
    octa*)
	BUILD_TARGET="sitl-octa"
        EXTRA_SIM="$EXTRA_SIM --frame=octa"
        MODEL="$FRAME"
	;;
    heli)
	BUILD_TARGET="sitl-heli"
        EXTRA_SIM="$EXTRA_SIM --frame=heli"
        MODEL="heli"
	;;
    IrisRos)
	BUILD_TARGET="sitl"
        EXTRA_SIM="$EXTRA_SIM --frame=IrisRos"
	;;
    CRRCSim-heli)
	BUILD_TARGET="sitl-heli"
        EXTRA_SIM="$EXTRA_SIM --frame=CRRCSim-heli"
        MODEL="$FRAME"
	;;
    CRRCSim|last_letter*)
	BUILD_TARGET="sitl"
        EXTRA_SIM="$EXTRA_SIM --frame=$FRAME"
        MODEL="$FRAME"
	;;
    jsbsim*)
	BUILD_TARGET="sitl"
        EXTRA_SIM="$EXTRA_SIM --frame=$FRAME"
        MODEL="$FRAME"
        check_jsbsim_version
	;;
    rover|rover-skid)
        EXTRA_SIM="$EXTRA_SIM --frame=$FRAME"
        MODEL="$FRAME"
	;;
    "")
        ;;
    *)
        echo "Unknown frame type $FRAME"
        usage
        exit 1
        ;;
esac

if [ $DEBUG_BUILD == 1 ]; then
    BUILD_TARGET="$BUILD_TARGET-debug"
fi

autotest=$(dirname $(readlink -e $0))
if [ $NO_REBUILD == 0 ]; then
pushd $autotest/../../$VEHICLE || {
    echo "Failed to change to vehicle directory for $VEHICLE"
    usage
    exit 1
}
VEHICLEDIR=$(pwd)
if [ $CLEAN_BUILD == 1 ]; then
    make clean
fi
make $BUILD_TARGET -j$NUM_PROCS || {
    make clean
    make $BUILD_TARGET -j$NUM_PROCS
}
popd
fi

# get the location information
if [ -z $CUSTOM_LOCATION ]; then
    SIMHOME=$(cat $autotest/locations.txt | grep -i "^$LOCATION=" | cut -d= -f2)
else
    SIMHOME=$CUSTOM_LOCATION
    LOCATION="Custom_Location"
fi

[ -z "$SIMHOME" ] && {
    echo "Unknown location $LOCATION"
    usage
    exit 1
}
echo "Starting up at $LOCATION : $SIMHOME"

TRACKER_HOME=$(cat $autotest/locations.txt | grep -i "^$TRACKER_LOCATION=" | cut -d= -f2)
[ -z "$TRACKER_HOME" ] && {
    echo "Unknown tracker location $TRACKER_LOCATION"
    usage
    exit 1
}


if [ $START_ANTENNA_TRACKER == 1 ]; then
    pushd $autotest/../../AntennaTracker
    if [ $CLEAN_BUILD == 1 ]; then
        make clean
    fi
    make sitl -j$NUM_PROCS || {
        make clean
        make sitl -j$NUM_PROCS
    }
    TRACKER_INSTANCE=1
    TRACKIN_PORT="127.0.0.1:"$((5502+10*$TRACKER_INSTANCE))
    TRACKOUT_PORT="127.0.0.1:"$((5501+10*$TRACKER_INSTANCE))
    TRACKER_UARTA="tcp:127.0.0.1:"$((5760+10*$TRACKER_INSTANCE))
    cmd="nice /tmp/AntennaTracker.build/AntennaTracker.elf -I1"
    $autotest/run_in_terminal_window.sh "AntennaTracker" $cmd || exit 1
    $autotest/run_in_terminal_window.sh "pysim(Tracker)" nice $autotest/pysim/sim_tracker.py --home=$TRACKER_HOME --simin=$TRACKIN_PORT --simout=$TRACKOUT_PORT $TRACKER_ARGS || exit 1
    popd
fi

cmd="$VEHICLEDIR/$VEHICLE.elf -S -I$INSTANCE --home $SIMHOME"
if [ $WIPE_EEPROM == 1 ]; then
    cmd="$cmd -w"
fi

case $VEHICLE in
    ArduPlane)
        PARMS="ArduPlane.parm"
        RUNSIM=""
        cmd="$cmd --model $MODEL --speedup=$SPEEDUP"
        ;;
    ArduCopter)
        RUNSIM=""
        cmd="$cmd --model $MODEL --speedup=$SPEEDUP"
        PARMS="copter_params.parm"
        ;;
    APMrover2)
        RUNSIM=""
        cmd="$cmd --model $MODEL --speedup=$SPEEDUP"
        PARMS="Rover.parm"
        ;;
    *)
        echo "Unknown vehicle simulation type $VEHICLE - please specify vehicle using -v VEHICLE_TYPE"
        usage
        exit 1
        ;;
esac

if [ $USE_MAVLINK_GIMBAL == 1 ]; then
    echo "Using MAVLink gimbal"
    cmd="$cmd --gimbal"
fi

if [ $START_HIL == 0 ]; then
if [ $USE_VALGRIND == 1 ]; then
    echo "Using valgrind"
    $autotest/run_in_terminal_window.sh "ardupilot (valgrind)" valgrind $cmd || exit 1
elif [ $USE_GDB == 1 ]; then
    echo "Using gdb"
    tfile=$(mktemp)
    [ $USE_GDB_STOPPED == 0 ] && {
        if [ -n "$BREAKPOINT" ]; then
            echo "b $BREAKPOINT" >> $tfile
        fi
        echo r >> $tfile
    }
    $autotest/run_in_terminal_window.sh "ardupilot (gdb)" gdb -x $tfile --args $cmd || exit 1
else
    $autotest/run_in_terminal_window.sh "ardupilot" $cmd || exit 1
fi
fi

trap kill_tasks SIGINT

echo "RUNSIM: $RUNSIM"

if [ -n "$RUNSIM" -o "$EXTERNAL_SIM" == 1 ]; then
    sleep 2
    rm -f $tfile
    if [ $EXTERNAL_SIM == 0 ]; then
        $autotest/run_in_terminal_window.sh "Simulator" $RUNSIM || {
            echo "Failed to start simulator: $RUNSIM"
            exit 1
        }
        sleep 2
    else
        echo "Using external ROS simulator"
        RUNSIM="$autotest/ROS/runsim.py --simin=$SIMIN_PORT --simout=$SIMOUT_PORT --fgout=$FG_PORT $EXTRA_SIM"
        $autotest/run_in_terminal_window.sh "ROS Simulator" $RUNSIM || {
            echo "Failed to start simulator: $RUNSIM"
            exit 1
        }
        sleep 2
    fi
else
    echo "not running external simulator"
fi

# mavproxy.py --master tcp:127.0.0.1:5760 --sitl 127.0.0.1:5501 --out 127.0.0.1:14550 --out 127.0.0.1:14551 
options=""
if [ $START_HIL == 0 ]; then
options="--master $MAVLINK_PORT --sitl $SIMOUT_PORT"
fi

# If running inside of a vagrant guest, then we probably want to forward our mavlink out to the containing host OS
if [ $USER == "vagrant" ]; then
options="$options --out 10.0.2.2:14550"
fi
options="$options --out 127.0.0.1:14550 --out 127.0.0.1:14551"
extra_cmd1=""
if [ $WIPE_EEPROM == 1 ]; then
    extra_cmd="param forceload $autotest/$PARMS; $EXTRA_PARM; param fetch"
fi
if [ $START_ANTENNA_TRACKER == 1 ]; then
    options="$options --load-module=tracker"
    extra_cmd="$extra_cmd module load map; tracker set port $TRACKER_UARTA; tracker start;"
fi
if [ $START_HIL == 1 ]; then
    options="$options --load-module=HIL"
fi
if [ $USE_MAVLINK_GIMBAL == 1 ]; then
    options="$options --load-module=gimbal"
fi
mavproxy.py $options --cmd="$extra_cmd" $*
if [ $START_HIL == 0 ]; then
kill_tasks
fi
