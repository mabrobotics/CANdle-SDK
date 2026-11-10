import pyCandle as pc

# Uncomment this for debugging the CANdlelib stack
# pc.logVerbosity(pc.Verbosity_E.VERBOSITY_3)

# Status registers in the same order as "candletool md info" prints them
STATUS_REGISTERS = (
    "mainEncoderStatus",
    "auxEncoderStatus",
    "calibrationStatus",
    "bridgeStatus",
    "hardwareStatus",
    "communicationStatus",
    "motionStatus",
    "miscStatus",
    "configStatus",
)

# Initialize CANdle on the USB bus (SPI bus not supported yet)
candle = pc.attachCandle(pc.CANdleDatarate_E.CAN_DATARATE_1M, pc.busTypes_t.USB)

# Look for MDs on the bus, stop at the first one found (only one is used)
ids = pc.discoverMDs(candle, 10, 0x7FF, 1)
if not ids:
    raise SystemExit("No MDs found!")

md = pc.MD(ids[0], candle)
if md.init() != pc.MD_Error_t.OK:
    raise SystemExit("Could not initialize MD!")

# Aux encoder status is meaningful only when aux encoder is configured
aux_encoder, err = pc.readRegisterU8(md, "auxEncoder")
if err != pc.MD_Error_t.OK:
    raise SystemExit(f"Error reading auxEncoder: {err}")

print(f"***** MD {ids[0]} STATUS *****")
for reg_name in STATUS_REGISTERS:
    if reg_name == "auxEncoderStatus" and aux_encoder == 0:
        continue
    # Note: python reads one register per request, C++ can read all of them in one
    value, err = pc.readRegisterU32(md, reg_name)
    if err != pc.MD_Error_t.OK:
        raise SystemExit(f"Error reading {reg_name}: {err}")

    # Decode raw value into set flags, each flag is (name, isError)
    flags = pc.decodeMDStatus(reg_name, value)
    text = ", ".join(f"{name} [{'ERROR' if is_error else 'WARNING'}]" for name, is_error in flags)
    print(f"- {reg_name + ':':<22} 0x{value:08x} ({'Set flags: ' + text if flags else 'OK'})")
