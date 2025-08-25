# MD Python Bindings Documentation

This document provides concise documentation for the Motion Drive (MD) Python bindings in the CANdle SDK.

## Basic Usage

```python
import pyCandle

# Connect to CANdle and create MD instance
candle = pyCandle.attachCandle(pyCandle.CANdleBaudrate_E.CAN_BAUD_1M, pyCandle.busTypes_t.USB)
md = pyCandle.MD(can_id, candle)
result = md.init()
```

## Enumerations

### Error Codes

- `pyCandle.MD_Error_t.OK` - Operation successful
- `pyCandle.MD_Error_t.REQUEST_INVALID` - Invalid request
- `pyCandle.MD_Error_t.TRANSFER_FAILED` - Transfer failed
- `pyCandle.MD_Error_t.NOT_CONNECTED` - Device not connected

### Motion Modes

- `pyCandle.MotionMode_t.IDLE` - Motor idle (no control)
- `pyCandle.MotionMode_t.POSITION_PID` - Position control mode
- `pyCandle.MotionMode_t.VELOCITY_PID` - Velocity control mode
- `pyCandle.MotionMode_t.RAW_TORQUE` - Direct torque control
- `pyCandle.MotionMode_t.IMPEDANCE` - Impedance control mode

## MD Class

Primary interface for Motion Drive devices.

### Device Management

- `md.init()` - Initialize MD device
- `md.blink()` - Blink built-in LEDs for identification
- `md.enable()` - Enable PWM output
- `md.disable()` - Disable PWM output
- `md.reset()` - Reset the driver
- `md.clearErrors()` - Clear driver errors
- `md.save()` - Save configuration to memory
- `md.zero()` - Zero encoder position

### Motion Control

#### Mode Configuration

- `md.setMotionMode(mode)` - Set control mode
- `md.setCurrentLimit(limit)` - Set motor current limit
- `md.setTorqueBandwidth(bandwidth)` - Set torque control bandwidth
- `md.setMaxTorque(torque)` - Set maximum output torque

#### Position Control

- `md.setPositionPIDparam(kp, ki, kd, integralMax)` - Set position PID parameters
- `md.setTargetPosition(position)` - Set target position
- `md.setProfileVelocity(velocity)` - Set profile movement velocity
- `md.setProfileAcceleration(accel)` - Set profile movement acceleration

#### Velocity Control

- `md.setVelocityPIDparam(kp, ki, kd, integralMax)` - Set velocity PID parameters
- `md.setTargetVelocity(velocity)` - Set target velocity

#### Torque Control

- `md.setTargetTorque(torque)` - Set target torque

#### Impedance Control

- `md.setImpedanceParams(kp, kd)` - Set impedance control parameters (stiffness, damping)

### Measurements

- `md.getPosition()` - Get current motor position
- `md.getVelocity()` - Get current motor velocity
- `md.getTorque()` - Get current motor torque
- `md.getOutputEncoderPosition()` - Get output encoder position
- `md.getOutputEncoderVelocity()` - Get output encoder velocity
- `md.getTemperature()` - Get device temperature

## Register Access

Direct register read/write functions for advanced configuration:

### Read Functions

- `pyCandle.readRegisterFloat(md, "register_name")` - Read float register
- `pyCandle.readRegisterU32(md, "register_name")` - Read 32-bit unsigned integer
- `pyCandle.readRegisterU16(md, "register_name")` - Read 16-bit unsigned integer
- `pyCandle.readRegisterU8(md, "register_name")` - Read 8-bit unsigned integer
- `pyCandle.readRegisterString(md, "register_name")` - Read string register

### Write Functions

- `pyCandle.writeRegisterFloat(md, "register_name", value)` - Write float register
- `pyCandle.writeRegisterU32(md, "register_name", value)` - Write 32-bit unsigned integer
- `pyCandle.writeRegisterU16(md, "register_name", value)` - Write 16-bit unsigned integer
- `pyCandle.writeRegisterU8(md, "register_name", value)` - Write 8-bit unsigned integer
- `pyCandle.writeRegisterString(md, "register_name", value)` - Write string register

## Return Value Pattern

Register functions return tuples `(value, error)`, while control functions return error codes:

```python
# Register read returns (value, error)
position, error = pyCandle.readRegisterFloat(md, "main/position")
if error == pyCandle.MD_Error_t.OK:
    print(f"Position: {position}")

# Control functions return error code
error = md.setTargetPosition(1000.0)
if error != pyCandle.MD_Error_t.OK:
    print(f"Failed to set position: {error}")
```
