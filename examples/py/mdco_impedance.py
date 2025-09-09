import pyCandle as pc
import time

# Uncomment this for debugging the CANdlelib stack
# pc.logVerbosity(pc.Verbosity_E.VERBOSITY_3)

# Initialize CANdle on the USB bus (SPI bus not supported yet)
candle = pc.attachCandle(pc.CANdleDatarate_E.CAN_DATARATE_1M, pc.busTypes_t.USB)

# Create virual MD representation
mdco = pc.MDCO(15, candle)

maxAcceleration=10
maxDecceleration=10
maxTorque=1000
ratedTorque=1000
maxCurrent=1000
ratedCurrent=1000
maxSpeed=50
kp=10
kd=1
TorqueForceFeedback=10

# Initialize it to see if it connects
mdco.setProfileParameters(maxAcceleration,maxDecceleration,maxCurrent,ratedCurrent,maxSpeed,maxTorque,ratedTorque,kp,kd,TorqueForceFeedback)
err = mdco.enableDriver(pc.CanOpenMotionMode_t.Impedance)
print(f"MD initialized with following status: {err}")

if err == pc.MDCO_Error_t.OK:
    time.sleep(5)

mdco.disableDriver()