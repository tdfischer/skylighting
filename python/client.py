#!/usr/bin/env python
import serial
import sys
import struct
from xbee import XBee

ser = serial.Serial(sys.argv[1], 9600)

#clear buffer
ser.flushInput()
ser.flushOutput()

xbee = XBee(ser)

TYPE_FIND_SERVICE = 0
TYPE_FIND_METHOD = 1
TYPE_METHOD_CALL = 2

def callMethod_old(service, method, **kwargs):
  pkt = ""
  pkt += struct.pack("!BB", TYPE_METHOD_CALL, len(service))
  pkt += service
  pkt += struct.pack("!B", len(method))
  pkt += method
  pkt += struct.pack("!B", len(kwargs))
  for k,v in kwargs.iteritems():
    pkt += struct.pack("!B", len(k))
    pkt += v
    pkt += struct.pack("!B", len(v))
    pkt += v
  return pkt

_services = {
  'net.phrobo.graviton.introspection': 0
}

_methods= {
  'net.phrobo.graviton.introspection': {
    'ping': 0
  }
}

def readResponse(ser):
  buf = ser.read(1)
  pktType = struct.unpack("<B", buf)[0]
  if pktType == 0:
    print "None!"
    return None
  if pktType == 1:
    print "Int!"
    return struct.unpack("<h", ser.read(2))[0]
  if pktType == 2:
    print "Str!"
    buf = ""
    c = struct.unpack("<B", ser.read(1))[0]
    while c != 0:
      print "Read", c
      buf += chr(c)
      c = struct.unpack("<B", ser.read(1))[0]
    return buf
  print "Unknown type", pktType

def findService(ser, service):
  pkt = ""
  pkt += struct.pack("<BB", 255, TYPE_FIND_SERVICE)
  pkt += service
  pkt += struct.pack("<B", 0)
  ser.write(pkt)
  return readResponse(ser)

def findMethod(ser, serviceIdx, method):
  pkt = ""
  pkt += struct.pack("!BBb", 255, TYPE_FIND_METHOD, serviceIdx)
  pkt += method
  pkt += struct.pack("!B", 0)
  ser.write(pkt)
  return readResponse(ser)

def callMethod(ser, serviceIdx, methodIdx, **kwargs):
  pkt = ""
  pkt += struct.pack("!BBBB", 255, TYPE_METHOD_CALL, serviceIdx, methodIdx)
  #pkt += struct.pack("!B", len(kwargs))
  for k,v in kwargs.iteritems():
    pkt += struct.pack("!B", len(k))
    pkt += v
    pkt += struct.pack("!B", len(v))
    pkt += v
  ser.write(pkt)
  return readResponse(ser)

print 'finding service'
serviceIdx = findService(ser, "net.phrobo.brilliance.linear")
print 'found at', serviceIdx
print 'finding method'
methodIdx = findMethod(ser, serviceIdx, "houselights")
print 'found at', methodIdx
print 'Calling callMethod'
data = callMethod(ser, serviceIdx, methodIdx)
print 'Got result', data
