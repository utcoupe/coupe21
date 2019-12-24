#!/usr/bin/env python
#-*- coding:utf-8 *-*

import ctypes
import os
import protocol_parser
import math
import numpy as np
import rospy
import time

from asserv_real import *
from geometry_msgs.msg import Pose2D
from ard_asserv.msg import RobotSpeed
from static_map.srv import MapGetContext, MapGetContextRequest

__author__ = "Thomas Fuhrmann & milesial & Mindstan & PaulMConstant"
__date__ = 19/04/2018

# Rates 
SEND_POSE_RATE = 0.1  # in s
SEND_SPEED_RATE = 0.1  # in s
ASSERV_RATE = 0.05  # in s

class AsservSetPosModes():
    AXY = 0
    A = 1
    Y = 2
    AY = 3
    X = 4
    AX = 5
    XY = 6

class stm32Speeds(ctypes.Structure):
    _fields_= [("pwm_left", ctypes.c_int),
               ("pwm_right", ctypes.c_int),
               ("angular_speed", ctypes.c_float),
               ("linear_speed", ctypes.c_float)]

class stm32Control(ctypes.Structure):
    _fields_=[("speeds", stm32Speeds),
              ("max_acc", ctypes.c_float),
              ("max_spd", ctypes.c_float),
              ("rot_spd_ratio", ctypes.c_float),
              ("reset", ctypes.c_uint8),
              ("last_finished_id", ctypes.c_uint16),
              ("order_started", ctypes.c_uint16),
              ("status_bits", ctypes.c_int)]

class AsservSimu(AsservReal):
    #Overloaded functions : 

    # Adapt serial port to send data to 
    # the asserv library instead of arduino
    def _start_serial_com_line(self, port):
        # no serial line in simu
        pass

    def _data_receiver(self):
        # no data to receive from non-existent arduino
        pass

    def _process_received_data(self, data):
        # no data will be received from non-existent arduino
        pass

    def _send_serial_data(self, order_type, args_list):
        args_list.insert(0, str(self._order_id))
        self._order_id += 1
        self._sending_queue.put(order_type + ";" + ";".join(args_list) + ";\n")

    def _callback_timer_serial_send(self, event):
        if not self._sending_queue.empty():
            data_to_send = self._sending_queue.get()
            rospy.logdebug("Sending data : " + data_to_send)
            self._simu_protocol_parse(data_to_send)
            self._sending_queue.task_done()
    
    def __init__(self, asserv_node):
        AsservReal.__init__(self, asserv_node, 0)

        self._stm32lib = ctypes.cdll.LoadLibrary(
            os.environ['UTCOUPE_WORKSPACE']
             + '/libs/lib_stm32_asserv.so')

        self._orders_dictionary_reverse = {
            v: k for k, v in self._orders_dictionary.iteritems()}

        # ROS stuff
        self._tmr_asserv_loop = rospy.Timer(rospy.Duration(ASSERV_RATE), self._asserv_loop)
        self._tmr_speed_send = rospy.Timer(rospy.Duration(SEND_SPEED_RATE), self._callback_timer_speed_send)
        self._tmr_pose_send = rospy.Timer(rospy.Duration(SEND_POSE_RATE), self._callback_timer_pose_send)

        # Get map & robot size 
        self._map_x = 3.0
        self._map_y = 2.0
        self._robot_wheelbase = 0.2

        context = self.getMapContext()
        if context:
            self._map_x = context.terrain_shape.width
            self._map_y = context.terrain_shape.height
            self._robot_wheelbase = context.robot_shape.wheelbase

        else:
            rospy.logwarn("Can't get map context, taking default values.")

        
        self._robot_up_size = 0.23
        self._robot_down_size = 0.058
        self._sending_queue.put(self._orders_dictionary['START'] + ";0;\n")
        self._stm32lib.ControlLogicInit()
        self._stm32lib.RobotStateLogicInit()
        self._control = stm32Control.in_dll(self._stm32lib, "control")

    def _simu_protocol_parse(self, data):
        order_char = data[0]
        order_id = data[2]
        
        # Move to first parameter of the order
        semicolon_count = 0
        index = 0
        while semicolon_count != 2:
            if data[index] == ";":
                semicolon_count += 1
            index += 1
        data = data[index:]

        # C type conversion
        c_order_id = ctypes.c_int(int(order_id))
        c_data = ctypes.c_char_p(data)

        # Switch case like in parseAndExecuteOrder
        order_type = self._orders_dictionary_reverse[order_char]

        if order_type == "START":
            self._stm32lib.start()

        elif order_type == "HALT":
            self._stm32lib.halt()

        elif order_type == "PINGPING":
            rospy.logerr("PINGPING order is not implemented in simu...")

        elif order_type == "GET_CODER":
            rospy.logerr("GET_CODER order is not implemented in simu...")

        elif order_type == "GOTO":
            self._stm32lib.parseGOTO(c_data, c_order_id)
        
        elif order_type == "GOTOA":
            self._stm32lib.parseGOTOA(c_data, c_order_id)

        elif order_type == "ROT":
            self._stm32lib.parseROT(c_data, c_order_id)

        elif order_type == "ROTNMODULO":
            self._stm32lib.parseROTNMODULO(c_data, c_order_id)

        elif order_type == "PWM":
            self._stm32lib.parsePWM(c_data, c_order_id)

        elif order_type == "SPD":
            self._stm32lib.parseSPD(c_data, c_order_id)

        elif order_type == "PIDALL":
            self._stm32lib.parsePIDALL(c_data)

        elif order_type == "PIDRIGHT":
            self._stm32lib.parsePIDRIGHT(c_data)

        elif order_type == "PIDLEFT":
            self._stm32lib.parsePIDLEFT(c_data)

        elif order_type == "KILLG":
            self._stm32lib.FifoNextGoal()
            self._stm32lib.ControlPrepareNewGoal()

        elif order_type == "CLEANG":
            self._stm32lib.FifoInit()
            self._stm32lib.ControlPrepareNewGoal()

        elif order_type == "RESET_ID":
            self._stm32lib.resetID()

        elif order_type == "SET_POS":
            self._stim32lib.parseSETPOS(c_data)

        elif order_type == "GET_POS":
            rospy.logerr("GET_POS order is not implemented in simu...")

        elif order_type == "GET_SPD":
            rospy.logerr("GET_SPD order is not implemented in simu...")

        elif order_type == "GET_TARGET_SPD":
            rospy.logerr("GET_TARGET_SOD order is not implemented in simu...")

        elif order_type == "GET_POS_ID":
            rospy.logerr("GET_POS_ID order is not implemented in simu...")

        elif order_type == "SPDMAX":
            self._stm32lib.parseSPDMAX(c_data)

        elif order_type == "ACCMAX":
            self._stm32lib.parseACCMAX(c_data)

        elif order_type == "GET_LAST_ID":
            rospy.logerr("GET_LAST_ID order is not implemented in simu...")

        elif order_type == "PAUSE": #TODO getPauseBit();
            self._stm32lib.ControlSetStop(self._stm32lib.getPauseBit())

        elif order_type == "RESUME":
            self._stm32lib.ControlUnsetStop(self._stm32lib.getPauseBit())

        elif order_type == "WHOAMI":
            rospy.logerr("WHOAMI order is not implemented in simu...")

        elif order_type == "SETEMERGENCYSTOP":
            self._stm32lib.parseSETEMERGENCYSTOP(c_data)
        
        else:
            rospy.logerr("Order " + order_char + " is wrong !")

    def getMapContext(self):
        dest = "/static_map/get_context"
        try: # Handle a timeout in case one node doesn't respond
            server_wait_timeout = 2
            rospy.logdebug("Waiting for service %s for %d seconds" % (dest, server_wait_timeout))
            rospy.wait_for_service(dest, timeout=server_wait_timeout)
        except rospy.ROSException:
            return False

        rospy.loginfo("Sending service request to '{}'...".format(dest))
        service = rospy.ServiceProxy(dest, MapGetContext)
        response = service(MapGetContextRequest()) # rospy can't handle timeout, solution ?

        if response is not None:
            return response
        else:
            rospy.logerr("Service call response from '{}' is null.".format(dest))
        return False

    def start(self):
        rospy.logdebug("[ASSERV] Node has correctly started in simulation mode.")
        rospy.spin()

    def set_pos(self, x, y, a, mode=AsservSetPosModes.AXY):
        """
        Sets a new position. Can update x, y or a only.

        @param x: new x coord
        @param y: new y coord
        @param a: new angle
        @param mode: choose if you want to change any x, y, a combination.
        See AsservSetPosModes class for modes info.
        """
        if (x < 0 or x > self._map_x or
            y < 0 or y > self._map_y or
            a < -math.pi or a > math.pi ):
            rospy.logerr("Invalid position !")
            return False

        new_pose = Pose2D(self._robot_raw_position.x, 
                          self._robot_raw_position.y,
                          self._robot_raw_position.theta)

        if mode == AsservSetPosModes.A:
            new_pose.theta = a
        elif mode == AsservSetPosModes.Y:
            new_pose.y = y
        elif mode == AsservSetPosModes.AY:
            new_pose.theta = a
            new_pose.y = y
        elif mode == AsservSetPosModes.X:
            new_pose.x = x
        elif mode == AsservSetPosModes.AX:
            new_pose.theta = a
            new_pose.x = x
        elif mode == AsservSetPosModes.XY:
            new_pose.x = x
            new_pose.y = y
        elif mode == AsservSetPosModes.AXY:
            new_pose.x = x
            new_pose.y = y
            new_pose.theta = a

        self._robot_raw_position = new_pose
        self._stm32lib.RobotStateSetPos(ctypes.c_float(new_pose.x * 1000), 
                                        ctypes.c_float(new_pose.y * 1000), 
                                        ctypes.c_float(new_pose.theta * 1000))
        
        return True

    def _asserv_loop(self, event):
        """
        Main function of the asserv simu.
        Checks for new goals and updates behavior every ASSERV_RATE ms.
        """
        now = int(time.time() * 1000000)
        self._stm32lib.processCurrentGoal(ctypes.c_long(now))       
        self._update_robot_pose()
        return

    def _wallhit_stop(self, direction):
        """
        Checks if the robot is near a wall.
        If it is, stops it and returns True.

        @param direction: (current direction of the robot) 1 forward, 0 backwards
        @return: True if the robot is hitting a wall, else False
        """
        # TODO avoid robot teleporting when he comes from an angle
        # TODO fix it, sometimes it doesnt work as intended
        traj_angle = self._robot_raw_position.theta

        # Depending on the orientation of the robot, 
        # adjust the position of the map borders
        if abs(traj_angle) > math.pi:
            traj_angle -= np.sign(traj_angle) * 2 * math.pi

        # Set y border
        if traj_angle < 0 : # Facing down
            high_y_edge = self._map_y - self._robot_down_size
            low_y_wall = self._robot_up_size
        else: # Facing up
            high_y_edge = self._map_y - self._robot_up_size
            low_y_wall = self._robot_down_size

        # Set x border
        if abs(traj_angle) < math.pi/2: # Facing right
            high_x_edge = self._map_x - self._robot_up_size
            low_x_wall = self._robot_down_size
        else: # Facing left
            high_x_edge = self._map_x - self._robot_down_size
            low_x_wall = self._robot_up_size

        new_theta = None

        if self._robot_raw_position.x >= high_x_edge :
            new_theta = 0
        elif self._robot_raw_position.x <= low_x_wall :
            new_theta = -math.pi

        elif self._robot_raw_position.y >= high_y_edge :
            new_theta = math.pi/2
        elif self._robot_raw_position.y <= low_y_wall :
            new_theta = -math.pi/2
        
        if new_theta is None:
            return False

        if direction == 0 : # Semiturn if we were going backwards
            new_theta += math.pi
        new_theta %= 2 * math.pi

        self._robot_raw_position = Pose2D(self._robot_raw_position.x, 
                                    self._robot_raw_position.y, 
                                    new_theta)
        rospy.loginfo('[ASSERV] A wall has been hit !')
        self._node.goal_reached(self._current_goal.goal_id, True)
        # self._stop()

        return True

    def _update_robot_pose(self):
        """
        Uses the angular and linear speeds to update the robot Pose(x, y, theta).
        """
        new_theta = self._robot_raw_position.theta + self._control.speeds.angular_speed / 1000 * ASSERV_RATE
        new_theta %= 2 * math.pi

        new_x = self._robot_raw_position.x + self._control.speeds.linear_speed / 1000 * ASSERV_RATE * math.cos(
            new_theta)
        new_y = self._robot_raw_position.y + self._control.speeds.linear_speed / 1000 * ASSERV_RATE * math.sin(
            new_theta) 
            
        self._robot_raw_position = Pose2D(new_x, new_y, new_theta)
        self._stm32lib.RobotStateSetPos(ctypes.c_float(new_x * 1000), 
                                        ctypes.c_float(new_y * 1000), 
                                        ctypes.c_float(new_theta * 1000))

    def _callback_timer_pose_send(self, event):
        self._node.send_robot_position(self._robot_raw_position)

    def _callback_timer_speed_send(self, event):
        self._node.send_robot_speed(RobotSpeed(0, 0, self._control.speeds.linear_speed, 0, 0))