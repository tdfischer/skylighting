#!/usr/bin/env python

from xbee import ZigBee
import serial
import sys

xbee = ZigBee(serial.Serial(sys.argv[1], 9600))

xbee.remote_at(command='ND', dest_addr_long=b'\x00\x00\x00\x00\x00\x00\xFF\xFF', options=b'\x08', frame_id='A')

print xbee.wait_read_frame()
