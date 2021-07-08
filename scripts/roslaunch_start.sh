#!/bin/bash
# DO NOT DELETE : USED FOR INIT.D SERVICE
source ~/.bashrc
source /opt/ros/melodic/setup.sh
source $UTCOUPE_WORKSPACE/ros_ws/devel/setup.sh

/usr/bin/python /opt/ros/melodic/bin/roslaunch definitions general.launch robot:=$1 &
sleep 2

