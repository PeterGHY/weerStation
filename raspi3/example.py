#!/usr/bin/env python2

import RFM69
from RFM69registers import *
import datetime
import time

test = RFM69.RFM69(RF69_433MHZ, 1, 1, True)

print "class initialized"
print "reading all registers"
results = test.readAllRegs()
for result in results:
    print result
print "Performing rcCalibration"
test.rcCalibration()
print "setting high power"
test.setHighPower(True)
print "Checking temperature"
print test.readTemperature(-20)

#print "setting encryption"
#test.encrypt("1234567891011121")
#test.encrypt("sampleEncryptKey")

nodeId=1

print "sending blah to 1"
if test.sendWithRetry(nodeId, "bla1", 3, 20):
    print "ack recieved"

print "reading"
exit
while True:
    test.receiveBegin()
    while not test.receiveDone():
        time.sleep(.1)
#    if test.receiveDone():
    print "len : %d" % len(test.DATA)
    #print "%s from %s RSSI:%s" % ("".join([chr(letter) for letter in test.DATA]), test.SENDERID, test.RSSI)
    print "Recv from : %s RSSI:%s" % (test.SENDERID, test.RSSI)
    array=("".join([chr(letter) for letter in test.DATA]))
    print "array :: %s" % (array)
    print "data 1 : %s Pressure : %s humidity : %s temp1 : %s temp2 : %s vBatt %s" \
	% (array[:19],array[19:27],array[27:33],array[33:39],array[39:43],array[43:49])

#    a=""
#    for c in array[19:27]:
#        print chr(int(c))
#    print a

    if test.ACKRequested():
        test.sendACK()

    if test.sendWithRetry(nodeId, "bla2", 3, 20):
        print "ack recieved"

print "shutting down"
test.shutdown()

