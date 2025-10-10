import pyCandle as pc
import time

candle = pc.attachCandle(pc.CANdleDatarate_E.CAN_DATARATE_1M, pc.busTypes_t.USB)
md = pc.MD(100, candle)
err = md.init()
print(f"MD initialized with following status: {err}")
if err != pc.MD_Error_t.OK:
    exit(1)

# Direct Id/Iq
pc.writeRegisterU8(md, "iqControlMode", 2)
pc.writeRegisterU8(md, "idControlMode", 2)
md.setMotionMode(pc.MotionMode_t.RAW_TORQUE)
pc.writeRegisterU8(md, "decouplingEnable", 0);
pc.writeRegisterFloat(md, "iqDirect", 0.)
pc.writeRegisterFloat(md, "idDirect", 0.)

time.sleep(2)

md.enable()
targetIq = 50.


for i in range(100):
    pc.writeRegisterFloat(md, "iqDirect", i/100.*targetIq );

for i in range(400):
    id, err = pc.readRegisterFloat(md, "idTarget")
    iq, err = pc.readRegisterFloat(md, "iqTarget")
    if i == 300:
        pc.writeRegisterFloat(md, "iqDirect", targetIq * -1.)
    if i == 305:
        pc.writeRegisterFloat(md, "iqDirect", targetIq)
    if i % 10 == 0:
        print(f"Iq: {round(iq,2)}, id: {round(id,2)}") 
    
for i in range(200):
    pc.writeRegisterFloat(md, "iqDirect", 0.)
    pc.writeRegisterFloat(md, "idDirect", 0.)

pc.writeRegisterU8(md, "decouplingEnable", 1);
print("#####\nDecupling enabled!\n#####");

for i in range(100):
    pc.writeRegisterFloat(md, "iqDirect", i/100.*targetIq );

for i in range(400):
    id, err = pc.readRegisterFloat(md, "idTarget")
    iq, err = pc.readRegisterFloat(md, "iqTarget")
    if i == 300:
        pc.writeRegisterFloat(md, "iqDirect", targetIq * -1.)
    if i == 305:
        pc.writeRegisterFloat(md, "iqDirect", targetIq)
    if i % 10 == 0:
        print(f"Iq: {round(iq,2)}, id: {round(id,2)}") 
    
md.disable();

