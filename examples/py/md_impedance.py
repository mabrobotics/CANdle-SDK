import time

import pyCandle as pc

# Uncomment this for debugging the CANdlelib stack
# pc.logVerbosity(pc.Verbosity_E.VERBOSITY_3)

# Initialize CANdle on the USB bus (SPI bus not supported yet)
candle = pc.attachCandle(pc.CANdleDatarate_E.CAN_DATARATE_1M, pc.busTypes_t.USB)

# Look for MDs on the bus, stop at the first one found (only one is used)
ids = pc.discoverMDs(candle, 10, 0x7FF, 1)
if not ids:
    raise SystemExit("No MDs found!")

# Create virtual MD representation
md = pc.MD(ids[0], candle)

# Initialize it to see if it connects
err = md.init()
print(f"MD initialized with following status: {err}")
if err != pc.MD_Error_t.OK:
    raise SystemExit("Could not initialize MD!")

# Read the current position, the movement is planned relative to it
start_pos, err = md.getPosition()
if err != pc.MD_Error_t.OK:
    raise SystemExit("Could not read MD position!")
print(f"Start position: {round(start_pos, 3)} rad")

# Set motion mode of the MD
md.setMotionMode(pc.MotionMode_t.IMPEDANCE)

# Hold the current position first, so the drive does not jump to a stale target when enabled
md.setTargetPosition(start_pos)

# Enable motor power
md.enable()

DISTANCE = 5.0  # rad, relative to the start position
STEPS = 100  # one step every 20 ms -> 2 s movement

# This loop is crucial as MD requires constant polling by any of its functions
# in order to keep the motor alive.
# The frequency of required polling can be adjusted via watchdog register in the MD.
for i in range(1, STEPS + 1):
    target = start_pos + DISTANCE * i / STEPS
    # Set desired position
    md.setTargetPosition(target)
    if i % 2 == 0:
        # Receive achieved position
        pos, err = md.getPosition()

        # Check exerted torque via a register
        torque = pc.readRegisterFloat(md, "motorTorque")[0]
        print(f"Exerting torque: {round(torque, 2)} Nm")
        print(f"Position: {round(pos, 2)}, Target position: {round(target, 2)} Error: {err}")

    # Sleep less than wdg timer
    time.sleep(0.02)

# Disable motor power
md.disable()
