import pyCandle as pc
import time

# Uncomment this for debugging the CANdlelib stack
# pc.logVerbosity(pc.Verbosity_E.VERBOSITY_3)

# Initialize CANdle
candle = pc.attachCandle(pc.CANdleDatarate_E.CAN_DATARATE_1M, pc.busTypes_t.USB, True)

# Create virtual MD representation (insert your motor id)
mdco = pc.MDCO(15, candle)

# movement parameter (insert your motor parameter)
maxAcceleration=10
maxDeceleration=10
maxTorque=2000
ratedTorque=1000
maxCurrent=1000
ratedCurrent=1000
maxSpeed=200
kp=10
kd=1
TorqueForceFeedback=0
desired_speed=50
timeoutms=5000

# Initialize it to see if it connects
err = mdco.setProfileParameters(maxAcceleration,maxDeceleration,maxCurrent,ratedCurrent,maxSpeed,maxTorque,ratedTorque,kp,kd,TorqueForceFeedback)
mdco.openSave()
print(f"MD initialized with following status: {err}")


if err == pc.MDCO_Error_t.OK:
    # Set motion mode of the MD
    mdco.enableDriver(pc.CanOpenMotionMode_t.ProfileVelocity)    
    mdco.moveSpeed(desired_speed,timeoutms)

# disable driver
mdco.disableDriver()

