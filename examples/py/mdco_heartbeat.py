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
maxCurrent=500
ratedCurrent=1000
maxSpeed=200
kp=10
kd=1
TorqueForceFeedback=0

# Initialize it to see if it connects
err = mdco.setProfileParameters(maxAcceleration,maxDeceleration,maxCurrent,ratedCurrent,maxSpeed,maxTorque,ratedTorque,kp,kd,TorqueForceFeedback)

# save the parameters to the motor controller
mdco.openSave()

# display the error code
print(f"MD initialized with following status: {err}")

# test the heartbeat
mdco.testHeartbeat()


