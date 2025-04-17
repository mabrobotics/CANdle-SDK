import pyCandle as pc
import time

# Uncomment this for debugging the CANdlelib stack
# pc.logVerbosity(pc.Verbosity_E.VERBOSITY_3)

# Initialize CANdle on the USB bus (SPI bus not supported yet)
candle = pc.attachCandle(pc.CANdleBaudrate_E.CAN_BAUD_1M, pc.busTypes_t.USB)

# Create virual MD representation
md = pc.MD(100, candle)

# Initialize it to see if it connects
err = md.init()

print(f"MD initialized with following status: {err}")

if err == pc.MD_Error_t.OK:
    # Zero out the drive position
    md.zero()

    # Set motion mode of the MD
    md.setMotionMode(pc.MotionMode_t.IMPEDANCE)

    # Enable motor power
    md.enable()

    # This loop is crucial as MD requires constant polling by any of its functions
    # in order to keep the motor alive.
    # The frequency of required polling can be adjusted via watchdog register in the MD.
    for i in range(100):
        t = i * 0.05
        # Set desired position
        md.setTargetPosition(t)
        if (i % 2 == 0):
            # Recieve achieved position
            pos, err = md.getPosition()

            # Check exerted torque via a register
            torque = pc.readRegisterFloat(md, "motorTorque")[0]
            print(f"Exerting torque: {round(torque,2)} Nm")
            print(
                f"Position: {round(pos,2)}, Target position: {round(t,2)} Error: {err}")

        # Sleep less than wdg timer
        time.sleep(0.02)

    # Disable motor power
    md.disable()
