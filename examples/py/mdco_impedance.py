import pyCandle as pc
import time

# Uncomment this for debugging the CANdlelib stack
# pc.logVerbosity(pc.Verbosity_E.VERBOSITY_3)

# Initialize CANdle
candle = pc.attachCandle(pc.CANdleDatarate_E.CAN_DATARATE_1M, pc.busTypes_t.USB, True)

# Create virtual MD representation
mdco = pc.MDCO(15, candle)

# movement parameter (insert your motor parameter)
maxAcceleration=10
maxDeceleration=10
maxTorque=2000
ratedTorque=1000
maxCurrent=2000
ratedCurrent=1000
maxSpeed=200
kp=10
kd=1
TorqueForceFeedback=0

# Initialize it to see if it connects
mdco.setProfileParameters(maxAcceleration,maxDeceleration,maxCurrent,ratedCurrent,maxSpeed,maxTorque,ratedTorque,kp,kd,TorqueForceFeedback)
mdco.openSave()
err = mdco.enableDriver(pc.CanOpenMotionMode_t.Impedance)
print(f"MD initialized with following status: {err}")

# Torque actual value register
index=0x6077
subindex=0x00
# Target position register
index2=0x607A
subindex2=0x00

# Display torque actual value
if err == pc.MDCO_Error_t.OK:
    # mode spring
    for i in range(500):
        value = mdco.getValueFromOpenRegister(index,subindex)
        # testing bit sign
        if value & (1 << 15):
            value -= 1 << 16
        # Display torque actual value
        print("torque actual value:",(value))
        time.sleep(0.01)
    # mode movement
    for i in range(500):
        mdco.writeOpenRegisters(index2,subindex2,-i*10,4)
        value = mdco.getValueFromOpenRegister(index2,subindex2)
        print("position actual value:",(value))
        time.sleep(0.01)
# disable driver
mdco.disableDriver()