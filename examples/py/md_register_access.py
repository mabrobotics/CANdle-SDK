import pyCandle as pc

# Uncomment this for debugging the CANdlelib stack
# pc.logVerbosity(pc.Verbosity_E.VERBOSITY_3)


def check(err, what):
    if err != pc.MD_Error_t.OK:
        raise SystemExit(f"Error {what}: {err}")


# Initialize CANdle on the USB bus (SPI bus not supported yet)
candle = pc.attachCandle(pc.CANdleDatarate_E.CAN_DATARATE_1M, pc.busTypes_t.USB)

# Look for MDs on the bus, stop at the first one found (only one is used)
ids = pc.discoverMDs(candle, 10, 0x7FF, 1)
if not ids:
    raise SystemExit("No MDs found!")

md = pc.MD(ids[0], candle)
check(md.init(), "initializing MD")

# Registers are accessed by name, the function suffix has to match the register type.
# Note: unlike C++, python reads/writes one register per request.

# 1. Read an integer register
pole_pairs, err = pc.readRegisterU32(md, "motorPolePairs")
check(err, "reading motorPolePairs")
print(f"Motor pole pairs: {pole_pairs}")

# 2. Read a float register
original_target_velocity, err = pc.readRegisterFloat(md, "targetVelocity")
check(err, "reading targetVelocity")
print(f"Target velocity: {original_target_velocity:.3f} rad/s")

# 3. Write a float register. The drive is not enabled, so changing the target does not move
# the motor.
check(
    pc.writeRegisterFloat(md, "targetVelocity", original_target_velocity + 1.0),
    "writing targetVelocity",
)
print(f"Target velocity set to: {original_target_velocity + 1.0:.3f} rad/s")

# 4. Read two registers
# One of the python implementation limitations is that only one register can
# be read in one message. No equivalent to readRegisters exists for python lib.
position, err = pc.readRegisterFloat(md, "mainEncoderPosition")
check(err, "reading mainEncoderPosition")
velocity, err = pc.readRegisterFloat(md, "mainEncoderVelocity")
check(err, "reading mainEncoderVelocity")
print(f"Position: {position:.4f} rad | Velocity: {velocity:.4f} rad/s")

# 5. Read three registers, targetVelocity shows the value written in step 3
targets = {}
for name in ("targetPosition", "targetVelocity", "targetTorque"):
    targets[name], err = pc.readRegisterFloat(md, name)
    check(err, f"reading {name}")
print(
    f"Targets - position: {targets['targetPosition']:.3f} | "
    f"velocity: {targets['targetVelocity']:.3f} | torque: {targets['targetTorque']:.3f}"
)

# 6. Write three registers, restoring the original target velocity
targets["targetVelocity"] = original_target_velocity
for name, value in targets.items():
    check(pc.writeRegisterFloat(md, name, value), f"writing {name}")
print(f"Targets restored, target velocity: {original_target_velocity:.3f} rad/s")
