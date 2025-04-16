import pyCandle as pc
import time

candle = pc.attachCandle(pc.CANdleBaudrate_E.CAN_BAUD_1M, pc.busTypes_t.USB)

md = pc.MD(100, candle)

err = md.init()

print(err)

if err == pc.MD_Error_t.OK:
    pc.writeRegisterString(md, "motorName", "abc efgh")
    name = pc.readRegisterString(md, "motorName")[0]
    print(f"Drive with name: {name}")

    md.zero()
    md.setMotionMode(pc.MotionMode_t.IMPEDANCE)
    md.enable()
    for i in range(20):
        t = i * 0.05
        md.setTargetPosition(t)
        pos, err = md.getPosition()
        name = pc.readRegisterString(md, "motorName")[0]
        print(f"Drive with name: {name}")
        canID = pc.readRegisterU32(md, "canID")[0]
        print(f"Drive with ID: {canID}")
        band = pc.readRegisterU16(md, "motorTorqueBandwidth")[0]
        print(f"With bandwidth: {band} Hz")
        torque = pc.readRegisterFloat(md, "motorTorque")[0]
        print(f"Exerting torque: {round(torque,2)} Nm")
        print(f"Position: {round(pos,2)}, Target position: {round(t,2)} Error: {err}")
        time.sleep(0.1)
    md.disable()
