# -*- coding: utf-8 -*-
#
#     ||          ____  _ __
#  +------+      / __ )(_) /_______________ _____  ___
#  | 0xBC |     / __  / / __/ ___/ ___/ __ `/_  / / _ \
#  +------+    / /_/ / / /_/ /__/ /  / /_/ / / /_/  __/
#   ||  ||    /_____/_/\__/\___/_/   \__,_/ /___/\___/
#
#  Copyright (C) 2014 Bitcraze AB
#
#  Crazyflie Nano Quadcopter Client
#
#  This program is free software; you can redistribute it and/or
#  modify it under the terms of the GNU General Public License
#  as published by the Free Software Foundation; either version 2
#  of the License, or (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
#  MA  02110-1301, USA.
"""
Simple example that connects to one crazyflie (check the address on the last
line and update it to your crazyflie address), sets the anchors postition and
send a sequence of setpoints, one every 5 secondes.
This exemple is intended to work with the Loco Positioning System in TWR TOA
mode. It aims at documenting how to set the Crazyflie in position control mode
and how to send setpoints.
"""
import random
import time
from threading import Thread

import cflib.crtp
from cflib.crazyflie import Crazyflie

# Change anchor position and sequence according to your setup
#anchors = [(2, 0.99, 2.4),
#          (0, 0, 2.40)]
#           (4.67, 2.54, 1.80),
#           (0.59, 2.27, 0.20),
#           (4.70, 3.38, 0.20),
#           (4.70, 1.14, 0.20)]

#             x    y    z  YAW
sequence = [(0,    0,   1,  0),
            (0,    0,   1,  0),
            (0,    0,   1,  0),
            (0,    0,   1,  0),
            (0,    0,   1,  0),
            (0,    0,   1,  0),
            (0,    0,   1,  0),
            (0,    0,   0.3,  0)]


class AutonomousSequence:
    """
    Simple logging example class that logs the Stabilizer from a supplied
    link uri and disconnects after 5s.
    """

    def __init__(self, link_uri):
        """ Initialize and run the example with the specified link_uri """

        # Create a Crazyflie object without specifying any cache dirs
        self._cf = Crazyflie()

        # Connect some callbacks from the Crazyflie API
        self._cf.connected.add_callback(self._connected)
        self._cf.disconnected.add_callback(self._disconnected)
        self._cf.connection_failed.add_callback(self._connection_failed)
        self._cf.connection_lost.add_callback(self._connection_lost)

        print('Connecting to %s' % link_uri)

        # Try to connect to the Crazyflie
        self._cf.open_link(link_uri)

        # Variable used to keep main loop occupied until disconnect
        self.is_connected = True

        self._param_check_list = []
        self._param_groups = []

        random.seed()

    def _connected(self, link_uri):
        """ This callback is called form the Crazyflie API when a Crazyflie
        has been connected and the TOCs have been downloaded."""
        print('Connected to %s' % link_uri)

        # Start a separate thread to do the motor test.
        # Do not hijack the calling thread!
        Thread(target=self._run_sequence).start()

    def _connection_failed(self, link_uri, msg):
        """Callback when connection initial connection fails (i.e no Crazyflie
        at the specified address)"""
        print('Connection to %s failed: %s' % (link_uri, msg))
        self.is_connected = False

    def _connection_lost(self, link_uri, msg):
        """Callback when disconnected after a connection has been made (i.e
        Crazyflie moves out of range)"""
        print('Connection to %s lost: %s' % (link_uri, msg))

    def _disconnected(self, link_uri):
        """Callback when the Crazyflie is disconnected (called in all cases)"""
        print('Disconnected from %s' % link_uri)
        self.is_connected = False

    def _run_sequence(self):
        # Setting up the anchors position
        '''for i in range(len(anchors)):
            self._cf.param.set_value('anchorpos.anchor{}x'.format(i),
                                     '{}'.format(anchors[i][0]))
            self._cf.param.set_value('anchorpos.anchor{}y'.format(i),
                                     '{}'.format(anchors[i][1]))
            self._cf.param.set_value('anchorpos.anchor{}z'.format(i),
                                     '{}'.format(anchors[i][2]))'''

        self._cf.param.set_value('anchorpos.enable', '1')

        self._cf.param.set_value('flightmode.posSet', '1')

        #I commented out the followng three lines as anchor position and is already set and crazyflie is started from its iniital position
        #self._cf.param.set_value('kalman.resetEstimation', '1')
        #time.sleep(0.1)
        #self._cf.param.set_value('kalman.resetEstimation', '0')

        # TODO: replace the 5 second sleep by detecting filter convergence
        time.sleep(5)

        for position in sequence:
            print('Setting position {}'.format(position))
            for i in range(50):
                self._cf.commander.send_setpoint(position[1], position[0],
                                                 position[3],
                                                 int(position[2] * 1000))
                time.sleep(0.1)

        self._cf.commander.send_setpoint(0, 0, 0, 0)
        # Make sure that the last packet leaves before the link is closed
        # since the message queue is not flushed before closing
        time.sleep(0.2)
        self._cf.close_link()


if __name__ == '__main__':
    cflib.crtp.init_drivers(enable_debug_driver=False)

    # le = AutonomousSequence("radio://0/80/2M/E7E7E7E701")
    le = AutonomousSequence('radio://0/80/250K')
