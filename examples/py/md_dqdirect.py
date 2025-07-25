import pyCandle as pc
import time

candle = pc.attachCandle(pc.CANdleBaudrate_E.CAN_BAUD_1M, pc.busTypes_t.USB)
md = pc.MD(100, candle)
err = md.init()
print(f"MD initialized with following status: {err}")
if err != pc.MD_Error_t.OK:
    exit(1)

# Direct Id/Iq
pc.writeRegisterU8(md, "iqControlMode", 2)
pc.writeRegisterU8(md, "idControlMode", 2)
md.setMotionMode(pc.MotionMode_t.IMPEDANCE)
md.setTargetTorque(0.5)
md.enable()

for i in range(500):
    velocity = md.getVelocity()[0];
    pc.writeRegisterFloat(md, "iqDirect", 1.25)
    pc.writeRegisterFloat(md, "iqDirect", 1.)

    id, err = pc.readRegisterFloat(md, "idTarget")
    iq, err = pc.readRegisterFloat(md, "iqTarget")
    
    print(f"Velocity: {round(velocity,2)}, iq: {round(iq,2)}, id: {round(id,2)}")
    time.sleep(0.05)
md.disable()

