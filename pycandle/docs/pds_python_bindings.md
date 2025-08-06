# PDS Python Bindings Documentation

This document provides documentation for the Power Distribution System (PDS) Python bindings in the CANdle SDK.

### Importing the Module

```python
import pyCandle
```

## Basic Usage

```python
# Connect to CANdle device
candle = pyCandle.attachCandle(
    pyCandle.CANdleBaudrate_E.CAN_BAUD_1M,
    pyCandle.busTypes_t.USB
)
# Discover PDS devices on the CAN bus
pds_ids = pyCandle.Pds.discoverPDS(candle)
print(f"Found PDS devices: {pds_ids}")
# Connect to PDS device
pds = pyCandle.Pds(pds_ids[0], candle)
result = pds.init()
```

## Enumerations

### PDS_Error_t

Error codes returned by PDS operations:

- `OK`: Operation successful
- `INTERNAL_ERROR`: Internal PDS error
- `PROTOCOL_ERROR`: Communication protocol error
- `COMMUNICATION_ERROR`: CAN communication error

### moduleType_E

Types of PDS modules:

- `UNDEFINED`: None
- `CONTROL_BOARD`: Main control board
- `BRAKE_RESISTOR`: Brake resistor module
- `ISOLATED_CONVERTER`: Isolated DC-DC converter
- `POWER_STAGE`: Power stage module

### socketIndex_E

Physical socket indices:

- `UNASSIGNED`: No socket assigned
- `SOCKET_1` through `SOCKET_6`: Physical socket positions

### moduleVersion_E

Module hardware versions:

- `UNKNOWN`: Unknown version
- `V0_1`, `V0_2`, `V0_3`: Hardware versions

## Data Structures

### modulesSet_S

Contains module types for all sockets:

```python
modules = pyCandle.modulesSet_S()
modules.moduleTypeSocket1 = pyCandle.moduleType_E.POWER_STAGE
modules.moduleTypeSocket2 = pyCandle.moduleType_E.BRAKE_RESISTOR
# ... and so on for sockets 3-6
```

### pdsFwMetadata_S

Firmware metadata information:

```python
metadata = pyCandle.pdsFwMetadata_S()
result = pds.getFwMetadata(metadata)
if result == pyCandle.PDS_Error_t.OK:
    print(f"Firmware version: {metadata.version}")
    print(f"Git hash: {metadata.gitHash}")
```

### Status Structures

#### controlBoardStatus_S

Main control board status:

```python
status = pyCandle.controlBoardStatus_S()
result = pds.getStatus(status)
if result == pyCandle.PDS_Error_t.OK:
    print(f"Enabled: {status.ENABLED}")
    print(f"Over Temperature: {status.OVER_TEMPERATURE}")
    print(f"Over Current: {status.OVER_CURRENT}")
    # Additional status fields available
```

#### powerStageStatus_S

Power stage module status:

```python
status = pyCandle.powerStageStatus_S()
result = power_stage.getStatus(status)
```

#### brakeResistorStatus_S

Brake resistor module status:

```python
status = pyCandle.brakeResistorStatus_S()
result = brake_resistor.getStatus(status)
```

#### isolatedConverterStatus_S

Isolated converter module status:

```python
status = pyCandle.isolatedConverterStatus_S()
result = isolated_conv.getStatus(status)
```

## Classes

### Pds Class

Main PDS device interface.

#### Constructor

```python
pds = pyCandle.Pds(can_id, candle_device)
```

#### Methods

##### Device Control

- `init()`: Initialize PDS device
- `init(can_id)`: Initialize with specific CAN ID
- `printModuleInfo()`: Print connected module information
- `shutdown()`: Shutdown the PDS device
- `reboot()`: Reboot the PDS device
- `saveConfig()`: Save configuration to memory

##### Module Management

- `getModules()`: Get module configuration
- `verifyModuleSocket(type, socket)`: Verify module type at socket
- `attachBrakeResistor(socket)`: Attach brake resistor module
- `attachPowerStage(socket)`: Attach power stage module
- `attachIsolatedConverter(socket)`: Attach isolated converter module

##### Status and Monitoring

```python
# Get status
status = pyCandle.controlBoardStatus_S()
result = pds.getStatus(status)

# Clear status
result = pds.clearStatus(status)
result = pds.clearErrors()

# Get measurements
temperature = [0.0]  # Reference parameter
result = pds.getTemperature(temperature)

bus_voltage = [0]
result = pds.getBusVoltage(bus_voltage)
```

##### Configuration

```python
# CAN configuration
can_id = pds.getCanId()
result = pds.setCanId(new_can_id)

baudrate = pds.getCanBaudrate()
result = pds.setCanBaudrate(pyCandle.CANdleBaudrate_E.CAN_BAUD_2M)

# Temperature limits
temp_limit = [0.0]
result = pds.getTemperatureLimit(temp_limit)
result = pds.setTemperatureLimit(85.0)

# Shutdown configuration
shutdown_time = [0]
result = pds.getShutdownTime(shutdown_time)
result = pds.setShutdownTime(10000)  # 10 seconds

# Battery voltage levels
battery_lvl1 = [0]
battery_lvl2 = [0]
result = pds.getBatteryVoltageLevels(battery_lvl1, battery_lvl2)
result = pds.setBatteryVoltageLevels(22000, 20000)  # mV
```

##### Brake Resistor Management

```python
# Bind brake resistor to socket
result = pds.bindBrakeResistor(pyCandle.socketIndex_E.SOCKET_2)

# Get bound brake resistor socket
socket = [pyCandle.socketIndex_E.UNASSIGNED]
result = pds.getBindBrakeResistor(socket)

# Set trigger voltage
result = pds.setBrakeResistorTriggerVoltage(28000)  # 28V in mV

# Get trigger voltage
trigger_voltage = [0]
result = pds.getBrakeResistorTriggerVoltage(trigger_voltage)
```

##### Static Methods

```python
# Convert module type to string
type_str = pyCandle.Pds.moduleTypeToString(pyCandle.moduleType_E.POWER_STAGE)

# Discover PDS devices
pds_list = pyCandle.Pds.discoverPDS(candle)
```

### BrakeResistor Class

Interface for brake resistor modules.

#### Methods

```python
# Module control
brake_resistor.printModuleInfo()
result = brake_resistor.enable()
result = brake_resistor.disable()

# Status monitoring
status = pyCandle.brakeResistorStatus_S()
result = brake_resistor.getStatus(status)
result = brake_resistor.clearStatus(status)

enabled = [False]
result = brake_resistor.getEnabled(enabled)

# Temperature monitoring
temperature = [0.0]
result = brake_resistor.getTemperature(temperature)

# Configuration
result = brake_resistor.setTemperatureLimit(100.0)
temp_limit = [0.0]
result = brake_resistor.getTemperatureLimit(temp_limit)
```

### PowerStage Class

Interface for power stage modules.

#### Methods

```python
# Module control
power_stage.printModuleInfo()
result = power_stage.enable()
result = power_stage.disable()

# Status monitoring
status = pyCandle.powerStageStatus_S()
result = power_stage.getStatus(status)
result = power_stage.clearStatus(status)

enabled = [False]
result = power_stage.getEnabled(enabled)

# Measurements
voltage = [0]
result = power_stage.getOutputVoltage(voltage)

current = [0]
result = power_stage.getLoadCurrent(current)

power = [0]
result = power_stage.getPower(power)

energy = [0]
result = power_stage.getEnergy(energy)

temperature = [0.0]
result = power_stage.getTemperature(temperature)

# Configuration
result = power_stage.setAutostart(True)
autostart = [False]
result = power_stage.getAutostart(autostart)

result = power_stage.setOcdLevel(5000)  # 5A in mA
ocd_level = [0]
result = power_stage.getOcdLevel(ocd_level)

result = power_stage.setOcdDelay(1000)  # 1ms in µs
ocd_delay = [0]
result = power_stage.getOcdDelay(ocd_delay)

result = power_stage.setTemperatureLimit(85.0)
temp_limit = [0.0]
result = power_stage.getTemperatureLimit(temp_limit)

# Brake resistor binding
result = power_stage.bindBrakeResistor(pyCandle.socketIndex_E.SOCKET_2)
socket = [pyCandle.socketIndex_E.UNASSIGNED]
result = power_stage.getBindBrakeResistor(socket)

result = power_stage.setBrakeResistorTriggerVoltage(28000)
trigger_voltage = [0]
result = power_stage.getBrakeResistorTriggerVoltage(trigger_voltage)
```

### IsolatedConv Class

Interface for isolated converter modules.

#### Methods

```python
# Module control
isolated_conv.printModuleInfo()
result = isolated_conv.enable()
result = isolated_conv.disable()

# Status monitoring
status = pyCandle.isolatedConverterStatus_S()
result = isolated_conv.getStatus(status)
result = isolated_conv.clearStatus(status)

enabled = [False]
result = isolated_conv.getEnabled(enabled)

# Measurements
voltage = [0]
result = isolated_conv.getOutputVoltage(voltage)

current = [0]
result = isolated_conv.getLoadCurrent(current)

power = [0]
result = isolated_conv.getPower(power)

energy = [0]
result = isolated_conv.getEnergy(energy)

temperature = [0.0]
result = isolated_conv.getTemperature(temperature)

# Configuration
result = isolated_conv.setOcdLevel(3000)  # 3A in mA
ocd_level = [0]
result = isolated_conv.getOcdLevel(ocd_level)

result = isolated_conv.setOcdDelay(500)  # 0.5ms in µs
ocd_delay = [0]
result = isolated_conv.getOcdDelay(ocd_delay)

result = isolated_conv.setTemperatureLimit(80.0)
temp_limit = [0.0]
result = isolated_conv.getTemperatureLimit(temp_limit)
```

## Error Handling

Always check return values from PDS operations:

```python
result = pds.someOperation()
if result != pyCandle.PDS_Error_t.OK:
    print(f"Operation failed: {result}")
    # Handle error appropriately
```

Common error patterns:

```python
try:
    result = pds.init()
    if result == pyCandle.PDS_Error_t.COMMUNICATION_ERROR:
        print("CAN communication failed - check connections")
    elif result == pyCandle.PDS_Error_t.PROTOCOL_ERROR:
        print("Protocol error - check device compatibility")
    elif result != pyCandle.PDS_Error_t.OK:
        print(f"Unknown error: {result}")
except Exception as e:
    print(f"Exception occurred: {e}")
```

## Examples

### Complete PDS Setup Example

```python
import pyCandle

# Connect to CANdle
candle = pyCandle.attachCandle(
    pyCandle.CANdleBaudrate_E.CAN_BAUD_1M,
    pyCandle.busTypes_t.USB
)

if not candle:
    print("Failed to connect to CANdle")
    exit(1)

# Discover and connect to PDS
pds_ids = pyCandle.Pds.discoverPDS(candle)
if not pds_ids:
    print("No PDS devices found")
    exit(1)

pds = pyCandle.Pds(pds_ids[0], candle)
result = pds.init()
if result != pyCandle.PDS_Error_t.OK:
    print(f"Failed to initialize PDS: {result}")
    exit(1)

# Get system information
print(f"Connected to PDS with CAN ID: {pds.getCanId()}")

temperature = [0.0]
if pds.getTemperature(temperature) == pyCandle.PDS_Error_t.OK:
    print(f"PDS Temperature: {temperature[0]}°C")

# Work with modules
modules = pds.getModules()
if modules.moduleTypeSocket1 == pyCandle.moduleType_E.POWER_STAGE:
    power_stage = pds.attachPowerStage(pyCandle.socketIndex_E.SOCKET_1)
    if power_stage:
        power_stage.printModuleInfo()

        # Get power stage measurements
        voltage = [0]
        current = [0]
        if (power_stage.getOutputVoltage(voltage) == pyCandle.PDS_Error_t.OK and
            power_stage.getLoadCurrent(current) == pyCandle.PDS_Error_t.OK):
            print(f"Power Stage - Voltage: {voltage[0]}mV, Current: {current[0]}mA")
```

### Safety Example with Error Handling

```python
def safe_pds_operation(pds):
    """Example of safe PDS operation with proper error handling."""

    # Always check device status first
    status = pyCandle.controlBoardStatus_S()
    result = pds.getStatus(status)

    if result != pyCandle.PDS_Error_t.OK:
        print(f"Failed to get status: {result}")
        return False

    if status.OVER_TEMPERATURE:
        print("WARNING: Device over temperature!")
        return False

    if status.OVER_CURRENT:
        print("WARNING: Device over current!")
        return False

    # Proceed with operation
    temperature = [0.0]
    if pds.getTemperature(temperature) == pyCandle.PDS_Error_t.OK:
        if temperature[0] > 80.0:
            print(f"WARNING: High temperature: {temperature[0]}°C")
            return False

    return True
```

## Best Practices

### 1. Always Check Return Values

```python
# Good
result = pds.someOperation()
if result != pyCandle.PDS_Error_t.OK:
    handle_error(result)

# Bad
pds.someOperation()  # Ignoring return value
```

### 2. Use Reference Parameters Correctly

Python bindings use lists for reference parameters:

```python
# Correct way to get temperature
temperature = [0.0]  # Use list for reference parameter
result = pds.getTemperature(temperature)
if result == pyCandle.PDS_Error_t.OK:
    print(f"Temperature: {temperature[0]}")  # Access first element
```

### 3. Handle Hardware Safely

```python
# Always check device status before operations
def check_device_safety(device):
    status = pyCandle.controlBoardStatus_S()
    result = device.getStatus(status)

    if result != pyCandle.PDS_Error_t.OK:
        return False

    return not (status.OVER_TEMPERATURE or status.OVER_CURRENT)

# Use it before operations
if check_device_safety(pds):
    # Proceed with operation
    pass
else:
    print("Device not safe for operation")
```

### 4. Proper Resource Management

```python
# Initialize in correct order
candle = pyCandle.attachCandle(baudrate, bus_type)
if candle:
    pds = pyCandle.Pds(can_id, candle)
    if pds.init() == pyCandle.PDS_Error_t.OK:
        # Use PDS
        pass
```

### 5. Module Discovery Pattern

```python
def discover_and_attach_modules(pds):
    """Standard pattern for module discovery and attachment."""
    modules = {}

    for socket_num in range(1, 7):
        socket = getattr(pyCandle.socketIndex_E, f'SOCKET_{socket_num}')

        if pds.verifyModuleSocket(pyCandle.moduleType_E.POWER_STAGE, socket):
            module = pds.attachPowerStage(socket)
            if module:
                modules[f'power_stage_{socket_num}'] = module

        elif pds.verifyModuleSocket(pyCandle.moduleType_E.BRAKE_RESISTOR, socket):
            module = pds.attachBrakeResistor(socket)
            if module:
                modules[f'brake_resistor_{socket_num}'] = module

        elif pds.verifyModuleSocket(pyCandle.moduleType_E.ISOLATED_CONVERTER, socket):
            module = pds.attachIsolatedConverter(socket)
            if module:
                modules[f'isolated_conv_{socket_num}'] = module

    return modules
```

## Troubleshooting

### Common Issues

1. **Import Error**: Make sure the Python bindings are built and the path is correct
2. **CANdle Connection Failed**: Check USB connection and permissions
3. **PDS Not Found**: Verify CAN bus wiring and baudrate settings
4. **Communication Errors**: Check CAN termination and bus integrity
5. **Temperature Warnings**: Ensure adequate cooling and ventilation

### Debug Mode

Enable verbose logging for debugging:

```python
pyCandle.logVerbosity(pyCandle.Verbosity_E.VERBOSITY_3)
```

This documentation covers the complete PDS Python bindings interface. For additional examples and updates, refer to the example scripts and source code in the CANdle SDK repository.
