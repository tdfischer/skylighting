#!/usr/bin/env python

from xbee import XBee
import serial
import sys

ser = serial.Serial(sys.argv[1], 9600)

xbee = XBee(ser)
xbee.tx("ohio");

while True:
  try:
    response = xbee.wait_read_frame()
    print response
  except KeyboardInterrupt:
    break

ser.close()
