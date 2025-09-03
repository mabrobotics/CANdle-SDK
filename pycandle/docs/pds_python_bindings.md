# PDS Python Bindings Documentation

This document provides concise documentation for the Power Distribution System (PDS) Python bindings in the CANdle SDK.

## Basic Usage

```python
import pyCandle

# Connect to CANdle and discover PDS devices
candle = pyCandle.attachCandle(pyCandle.CANdleDatarate_E.CAN_DATARATE_1M, pyCandle.busTypes_t.USB)
pds_ids = pyCandle.Pds.discoverPDS(candle)
pds = pyCandle.Pds(pds_ids[0], candle)
result = pds.init()
```

## Enumerations

### Error Codes

- `pyCandle.PDS_Error_t.OK` - Operation successful
- `pyCandle.PDS_Error_t.INTERNAL_ERROR` - Internal PDS error
- `pyCandle.PDS_Error_t.PROTOCOL_ERROR` - Communication protocol error
- `pyCandle.PDS_Error_t.COMMUNICATION_ERROR` - CAN communication error

### Module Types

- `pyCandle.moduleType_E.CONTROL_BOARD` - Main control board
- `pyCandle.moduleType_E.BRAKE_RESISTOR` - Brake resistor module
- `pyCandle.moduleType_E.ISOLATED_CONVERTER` - Isolated DC-DC converter
- `pyCandle.moduleType_E.POWER_STAGE` - Power stage module

### Socket Indices

- `pyCandle.socketIndex_E.SOCKET_1` through `SOCKET_6` - Physical socket positions
- `pyCandle.socketIndex_E.UNASSIGNED` - No socket assigned

## Status Structures

All status structures have boolean flags accessible as attributes:

```python
# Control board status
status = pyCandle.controlBoardStatus_S()
pds.getStatus(status)
print(f"Enabled: {status.ENABLED}, Over Temp: {status.OVER_TEMPERATURE}")

# Module status (similar pattern for all module types)
ps_status = pyCandle.powerStageStatus_S()
power_stage.getStatus(ps_status)
```

## Main Classes

### Pds Class

Primary interface for PDS devices.

#### Device Management

- `pds.init()` - Initialize PDS device
- `pds.shutdown()` - Shutdown device
- `pds.reboot()` - Reboot device
- `pds.saveConfig()` - Save configuration to memory

#### Module Attachment

- `pds.attachPowerStage(socket)` - Attach power stage at socket
- `pds.attachBrakeResistor(socket)` - Attach brake resistor at socket
- `pds.attachIsolatedConverter(socket)` - Attach isolated converter at socket

#### Status and Monitoring

- `pds.getStatus(status_struct)` - Get device status
- `pds.getTemperature()` - Get device temperature (returns tuple: value, error)
- `pds.getBusVoltage()` - Get bus voltage (returns tuple: value, error)

#### Configuration

- `pds.setTemperatureLimit(temp)` - Set temperature limit
- `pds.setBatteryVoltageLevels(lvl1, lvl2)` - Set battery voltage levels
- `pds.setShutdownTime(time_ms)` - Set automatic shutdown time

### PowerStage Class

Interface for power stage modules.

#### Control

- `enable()` / `disable()` - Enable/disable module
- `setAutostart(enabled)` - Set autostart feature

#### Measurements

- `getOutputVoltage()` - Get output voltage in mV
- `getLoadCurrent()` - Get load current in mA
- `getPower()` - Get power in mW
- `getTemperature()` - Get temperature in °C

#### Configuration

- `setOcdLevel(level_ma)` - Set over-current detection level
- `setOcdDelay(delay_us)` - Set over-current detection delay
- `setTemperatureLimit(temp)` - Set temperature limit
- `bindBrakeResistor(socket)` - Bind brake resistor
- `setBrakeResistorTriggerVoltage(voltage_mv)` - Set BR trigger voltage

### BrakeResistor Class

Interface for brake resistor modules.

#### Control

- `enable()` / `disable()` - Enable/disable module

#### Monitoring

- `getTemperature()` - Get temperature in °C
- `getEnabled()` - Check if enabled

#### Configuration

- `setTemperatureLimit(temp)` - Set temperature limit

### IsolatedConv Class

Interface for isolated converter modules.

#### Control

- `enable()` / `disable()` - Enable/disable module

#### Measurements

- `getOutputVoltage()` - Get output voltage in mV
- `getLoadCurrent()` - Get load current in mA
- `getPower()` - Get power in mW
- `getTemperature()` - Get temperature in °C

#### Configuration

- `setOcdLevel(level_ma)` - Set over-current detection level
- `setOcdDelay(delay_us)` - Set over-current detection delay
- `setTemperatureLimit(temp)` - Set temperature limit

## Return Value Pattern

Most methods return tuples for getter functions and error codes for setter functions:

```python
# Getter methods return (value, error)
temperature, error = pds.getTemperature()
if error == pyCandle.PDS_Error_t.OK:
    print(f"Temperature: {temperature}°C")

# Setter methods return error code
error = pds.setTemperatureLimit(85.0)
if error != pyCandle.PDS_Error_t.OK:
    print(f"Failed to set temperature limit: {error}")
```
