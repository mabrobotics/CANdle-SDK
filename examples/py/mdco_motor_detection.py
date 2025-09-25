import pyCandle as pc
import time

# Uncomment this for debugging the CANdlelib stack
# pc.logVerbosity(pc.Verbosity_E.VERBOSITY_3)

# Initialize CANdle 
candle = pc.attachCandle(pc.CANdleDatarate_E.CAN_DATARATE_1M, pc.busTypes_t.USB, True)

# call discoverOpenMDs function
ids = pc.MDCO.discoverOpenMDs(candle)

# displays the list of all found IDs
if (len(ids)!=0):
    print("MDCO attach to the candle :", ids)
else:
    print ("No motor drives is attached to the candle")

# make blink each driver led in ascending order
for i in range(len(ids)):
    md = pc.MDCO(ids[i], candle);
    print("blink motor drive with ID: %d", ids[i])
    md.blinkOpenTest()
    time.sleep(5)
