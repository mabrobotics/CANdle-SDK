import pyCandle as pc
import time

# Uncomment this for debugging the CANdlelib stack
# pc.logVerbosity(pc.Verbosity_E.VERBOSITY_3)

# Initialize CANdle
candle = pc.attachCandle(pc.CANdleDatarate_E.CAN_DATARATE_1M, pc.busTypes_t.USB, True)

# Create virtual MD representation (insert your motor id)
mdco = pc.MDCO(15, candle)

# Display the value contain in the object 0x6064:0 in the terminal
mdco.readOpenRegisters(0x6064, 0x00, False)

# You can write value with SDO, you can use register name or register address
# You can find all register detail in the object dictionary:
# https://mabrobotics.github.io/MD80-x-CANdle-Documentation/md_canopen/OD.html
mdco.writeOpenRegisters("Controlword",15,False)
mdco.writeOpenRegisters(0x6040, 0x00,15,False)

# You can get value from a register
returnValue = mdco.getValueFromOpenRegister(0x6064, 0x00)
if (returnValue > 0):
    print("The position of the motor is positive")

# You can use SDO segmented transfer for register with a size over 4 bytes
data, err = mdco.readLongOpenRegisters(0x2000,0x06)
if err == pc.MDCO_Error_t.OK:
    print("Motor name before:", bytes(data).decode(errors="ignore"))
mdco.writeLongOpenRegisters(0x2000,0x06,"My Custom Motor Name",False)
data, err = mdco.readLongOpenRegisters(0x2000,0x06)
if err == pc.MDCO_Error_t.OK:
    print("Motor name after:", bytes(data).decode(errors="ignore"))


# you can also use PDO for faster communication
frame = [0x00,0x0F]
mdco.writeOpenPDORegisters((0x200 + 15), frame)

# you can use this method for every other message who don't need answer (NMT,TimeStamp,etc.).
# e.g. send a NMT reset node message to the motor drive with the lowest id
data = [0x81,0x0F]  
mdco.sendCustomData(0x000, data)
