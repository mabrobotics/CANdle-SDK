import pyCandle as pc
import time

candle = pc.attachCandle(pc.CANdleBaudrate_E.CAN_BAUD_1M, pc.busTypes_t.USB)
md = pc.MD(100, candle)
err = md.init()
print(f"MD initialized with following status: {err}")
if err != pc.MD_Error_t.OK:
    exit(1)

# Direct Id/Iq
pc.writeRegisterU8(md, "iqControlMode", 1)
pc.writeRegisterU8(md, "idControlMode", 1)
md.setMotionMode(pc.MotionMode_t.RAW_TORQUE)
time.sleep(2)
md.setTargetTorque(0.)
md.enable()

while True:
    velocity = md.getVelocity()[0] * 9.5492;    # rad/s to rpm

    id, err = pc.readRegisterFloat(md, "idTarget")
    iq, err = pc.readRegisterFloat(md, "iqTarget")
    
    print(f"Velocity: {round(velocity,2)}, iq: {round(iq,2)}, id: {round(id,2)}") 
md.disable()

