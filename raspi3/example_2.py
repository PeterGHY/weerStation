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
print "sending blah to 1"
if test.sendWithRetry(1, "bla", 3, 20):
    print "ack recieved"
print "reading"
while True:
    test.receiveBegin()
    while not test.receiveDone():
        time.sleep(.1)
    print "len : %d" % len(test.DATA)
    #print "%s from %s RSSI:%s" % ("".join([chr(letter) for letter in test.DATA]), test.SENDERID, test.RSSI)
    print "RSSI:%s" % (test.RSSI)
    array=("".join([chr(letter) for letter in test.DATA]))
    print "data 1 : %s data 2 : %s temp : %s vBatt %s" % (array[:19],array[19:23],array[23:27],array[27:33] )
#    print "dec   value : ", ', '.join(chr(int(i)) for i in array[20:27])
#    a=""
#    for c in array[19:27]:
#        print chr(int(c))
#    print a
    if test.ACKRequested():
        test.sendACK()

    if test.sendWithRetry(1, "bla", 3, 20):
        print "ack recieved"
print "shutting down"
test.shutdown()
