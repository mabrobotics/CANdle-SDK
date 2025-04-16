import pyCandle as pc
import time

candle = pc.attachCandle(pc.CANdleBaudrate_E.CAN_BAUD_1M, pc.busTypes_t.USB)

md = pc.MD(100, candle)

err = md.init()

print(err)

if err == pc.MD_Error_t.OK:
    md.zero()
    md.setMotionMode(pc.MotionMode_t.IMPEDANCE)
    md.enable()
    for i in range(100):
        t = i * 0.2
        md.setTargetPosition(t)
        pos, err = md.getPosition()
        print(f"Position: {round(pos,2)}, Target position: {round(t,2)} Error: {err}")
        time.sleep(0.1)
    md.disable()
