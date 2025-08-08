#!/usr/bin/env python3
"""
PDS (Power Distribution System) Example Script
"""

import sys
import os
import time

# Add the build directory to the path to import pyCandle
sys.path.insert(0, os.path.join(
    os.path.dirname(__file__), 'build', 'pycandle'))

try:
    import pyCandle
    print("Successfully imported pyCandle module")
except ImportError as e:
    print(f"Error: Failed to import pyCandle module: {e}")
    print("Make sure the Python bindings are built and the build directory is correct.")
    sys.exit(1)


class PDSExample:
    """
    Example class demonstrating updated PDS usage patterns with new wrapper functions.
    """

    def __init__(self):
        self.power_stages = []
        self.brake_resistors = []
        self.isolated_converters = []
        self.pds = None
        self.candle = None

    def connect_candle(self, baudrate=pyCandle.CANdleBaudrate_E.CAN_BAUD_1M,
                       bus_type=pyCandle.busTypes_t.USB):
        """
        Connect to a CANdle device.

        Args:
            baudrate: CAN bus baudrate
            bus_type: Bus connection type (USB or SPI)

        Returns:
            bool: True if successful, False otherwise
        """
        try:
            print(f"Attempting to connect to CANdle with baudrate {
                  baudrate} and bus type {bus_type}")
            self.candle = pyCandle.attachCandle(baudrate, bus_type)

            if self.candle is None:
                print("Failed to attach CANdle device")
                return False

            print("✓ Successfully connected to CANdle device")
            return True

        except Exception as e:
            print(f"Error connecting to CANdle: {e}")
            return False

    def disconnect_candle(self):
        """
        Disconnect from CANdle device.
        """
        if self.candle:
            self.candle = None
            print("✓ Successfully disconnected from CANdle device")

    def discover_pds_devices(self):
        """
        Discover PDS devices on the CAN bus.

        Returns:
            list: List of discovered PDS CAN IDs
        """
        if not self.candle:
            print("Error: CANdle not connected")
            return []

        try:
            print("Discovering PDS devices on the CAN bus...")
            pds_ids = pyCandle.Pds.discoverPDS(self.candle)

            if pds_ids:
                print(f"✓ Found {len(pds_ids)} PDS device(s): {pds_ids}")
            else:
                print("No PDS devices found")

            return pds_ids

        except Exception as e:
            print(f"Error during PDS discovery: {e}")
            return []

    def connect_pds(self, can_id=100):
        """
        Connect to a specific PDS device.

        Args:
            can_id: CAN ID of the PDS device

        Returns:
            bool: True if successful, False otherwise
        """
        if not self.candle:
            print("Error: CANdle not connected")
            return False

        try:
            print(f"Connecting to PDS device with CAN ID {can_id}")
            self.pds = pyCandle.Pds(can_id, self.candle)

            # Initialize the PDS device
            result = self.pds.init()

            print("✓ Successfully connected to PDS device")
            return True

        except Exception as e:
            print(f"Error connecting to PDS: {e}")
            return False

    def get_pds_info(self):
        """
        Get basic information about the connected PDS device using updated wrapper functions.
        """
        if not self.pds:
            print("Error: PDS not connected")
            return

        try:
            print("\n=== PDS Device Information ===")

            # Get firmware metadata using wrapper function
            metadata, result = self.pds.getFwMetadata()
            if result == pyCandle.PDS_Error_t.OK:
                print(f"Firmware version: {metadata.version.major}.{
                      metadata.version.minor}.{metadata.version.revision}")
                print(f"Git hash: {metadata.gitHash}")
            else:
                print(f"Failed to get firmware metadata: {result}")

            # Get CAN configuration
            can_id = self.pds.getCanId()
            print(f"CAN ID: {can_id}")

            # Get CAN baudrate
            baudrate = self.pds.getCanBaudrate()
            print(f"CAN Baudrate: {baudrate}")

            # Get status using wrapper function
            status, result = self.pds.getStatus()
            if result == pyCandle.PDS_Error_t.OK:
                print(f"Status - Enabled: {status.ENABLED}")
                print(f"Status - Over Temperature: {status.OVER_TEMPERATURE}")
                print(f"Status - Over Current: {status.OVER_CURRENT}")
                print(f"Status - STO 1: {status.STO_1}")
                print(f"Status - STO 2: {status.STO_2}")
            else:
                print(f"Failed to get status: {result}")

            # Get temperature using wrapper function
            temperature, result = self.pds.getTemperature()
            if result == pyCandle.PDS_Error_t.OK:
                print(f"Temperature: {temperature}°C")
            else:
                print(f"Failed to get temperature: {result}")

            # Get bus voltage using wrapper function
            bus_voltage, result = self.pds.getBusVoltage()
            if result == pyCandle.PDS_Error_t.OK:
                print(f"Bus Voltage: {bus_voltage}mV")
            else:
                print(f"Failed to get bus voltage: {result}")

            # Get temperature limit using wrapper function
            temp_limit, result = self.pds.getTemperatureLimit()
            if result == pyCandle.PDS_Error_t.OK:
                print(f"Temperature Limit: {temp_limit}°C")
            else:
                print(f"Failed to get temperature limit: {result}")

            # Get shutdown time using wrapper function
            shutdown_time, result = self.pds.getShutdownTime()
            if result == pyCandle.PDS_Error_t.OK:
                print(f"Shutdown Time: {shutdown_time}s")
            else:
                print(f"Failed to get shutdown time: {result}")

            # Get battery voltage levels using wrapper function
            battery_levels, result = self.pds.getBatteryVoltageLevels()
            if result == pyCandle.PDS_Error_t.OK:
                print(f"Battery Level 1: {battery_levels[0]}mV")
                print(f"Battery Level 2: {battery_levels[1]}mV")
            else:
                print(f"Failed to get battery voltage levels: {result}")

        except Exception as e:
            print(f"Error getting PDS info: {e}")

    def discover_modules(self):
        """
        Discover and print information about connected modules.
        """
        if not self.pds:
            print("Error: PDS not connected")
            return

        try:
            print("\n=== Module Discovery ===")

            # Get module configuration
            modules = self.pds.getModules()
            print("Connected modules:")
            print(f"  Socket 1: {pyCandle.Pds.moduleTypeToString(
                modules.moduleTypeSocket1)}")
            print(f"  Socket 2: {pyCandle.Pds.moduleTypeToString(
                modules.moduleTypeSocket2)}")
            print(f"  Socket 3: {pyCandle.Pds.moduleTypeToString(
                modules.moduleTypeSocket3)}")
            print(f"  Socket 4: {pyCandle.Pds.moduleTypeToString(
                modules.moduleTypeSocket4)}")
            print(f"  Socket 5: {pyCandle.Pds.moduleTypeToString(
                modules.moduleTypeSocket5)}")
            print(f"  Socket 6: {pyCandle.Pds.moduleTypeToString(
                modules.moduleTypeSocket6)}")

            # Print module information
            self.pds.printModuleInfo()

        except Exception as e:
            print(f"Error discovering modules: {e}")

    def attach_module(self, module_type, socket):
        if not self.pds:
            print("Error: PDS not connected")
            return

        try:
            print(f"\n=== Attaching Module {
                  module_type} to Socket {socket} ===")
            self.pds.attachModule(module_type, socket)
            print(f"Module {module_type} attached to Socket {socket}")
        except Exception as e:
            print(f"Error attaching module {
                  module_type} to Socket {socket}: {e}")

    def attach_modules(self):
        """
        Attach to discovered modules and store references.
        """
        try:
            for socket_num in range(1, 7):
                socket = getattr(pyCandle.socketIndex_E,
                                 f'SOCKET_{socket_num}')

                # Try Power Stage
                if self.pds.verifyModuleSocket(pyCandle.moduleType_E.POWER_STAGE, socket):
                    power_stage = self.pds.attachPowerStage(socket)
                    if power_stage:
                        self.power_stages.append(power_stage)
                        print(f"✓ Attached Power Stage at socket {socket_num}")

                # Try Brake Resistor
                elif self.pds.verifyModuleSocket(pyCandle.moduleType_E.BRAKE_RESISTOR, socket):
                    brake_resistor = self.pds.attachBrakeResistor(socket)
                    if brake_resistor:
                        self.brake_resistors.append(brake_resistor)
                        print(f"✓ Attached Brake Resistor at socket {
                              socket_num}")

                # Try Isolated Converter
                elif self.pds.verifyModuleSocket(pyCandle.moduleType_E.ISOLATED_CONVERTER, socket):
                    isolated_conv = self.pds.attachIsolatedConverter(socket)
                    if isolated_conv:
                        self.isolated_converters.append(isolated_conv)
                        print(f"✓ Attached Isolated Converter at socket {
                              socket_num}")

            print(f"Total attached modules: {len(self.power_stages)} Power Stages, "
                  f"{len(self.brake_resistors)} Brake Resistors, "
                  f"{len(self.isolated_converters)} Isolated Converters")
        except Exception as e:
            print(f"Error attaching modules: {e}")

    def demonstrate_isolated_converter_usage(self):
        """
        Demonstrate Isolated Converter module usage with updated wrapper functions.
        """
        if not self.isolated_converters:
            print("No Isolated Converter modules available")
            return

        try:
            print("\n=== Isolated Converter Usage ===")

            for i, converter in enumerate(self.isolated_converters):
                print(f"\nIsolated Converter {i+1}:")

                # Print module info
                converter.printModuleInfo()

                # Get status using wrapper function
                status, result = converter.getStatus()
                if result == pyCandle.PDS_Error_t.OK:
                    print(f"  Status - Enabled: {status.ENABLED}")
                    print(
                        f"  Status - Over Temperature: {status.OVER_TEMPERATURE}")
                    print(f"  Status - Over Current: {status.OVER_CURRENT}")
                else:
                    print(f"  Error getting status: {result}")

                # Get enabled state using wrapper function
                enabled, result = converter.getEnabled()
                if result == pyCandle.PDS_Error_t.OK:
                    print(f"  Enabled: {enabled}")
                else:
                    print(f"  Error getting enabled state: {result}")

                # Get measurements using wrapper functions
                temperature, result = converter.getTemperature()
                if result == pyCandle.PDS_Error_t.OK:
                    print(f"  Temperature: {temperature}°C")
                else:
                    print(f"  Error getting temperature: {result}")

                output_voltage, result = converter.getOutputVoltage()
                if result == pyCandle.PDS_Error_t.OK:
                    print(f"  Output Voltage: {output_voltage}mV")
                else:
                    print(f"  Error getting output voltage: {result}")

                load_current, result = converter.getLoadCurrent()
                if result == pyCandle.PDS_Error_t.OK:
                    print(f"  Load Current: {load_current}mA")
                else:
                    print(f"  Error getting load current: {result}")

                power, result = converter.getPower()
                if result == pyCandle.PDS_Error_t.OK:
                    print(f"  Power: {power}mW")
                else:
                    print(f"  Error getting power: {result}")

                energy, result = converter.getEnergy()
                if result == pyCandle.PDS_Error_t.OK:
                    print(f"  Energy: {energy}mWh")
                else:
                    print(f"  Error getting energy: {result}")

                # Get configuration parameters using wrapper functions
                ocd_level, result = converter.getOcdLevel()
                if result == pyCandle.PDS_Error_t.OK:
                    print(f"  OCD Level: {ocd_level}mA")
                else:
                    print(f"  Error getting OCD level: {result}")

                ocd_delay, result = converter.getOcdDelay()
                if result == pyCandle.PDS_Error_t.OK:
                    print(f"  OCD Delay: {ocd_delay}μs")
                else:
                    print(f"  Error getting OCD delay: {result}")

                temp_limit, result = converter.getTemperatureLimit()
                if result == pyCandle.PDS_Error_t.OK:
                    print(f"  Temperature Limit: {temp_limit}°C")
                else:
                    print(f"  Error getting temperature limit: {result}")

        except Exception as e:
            print(f"Error demonstrating Isolated Converter usage: {e}")

    def demonstrate_power_stage_usage(self):
        """
        Demonstrate Power Stage module usage with updated wrapper functions.
        """
        if not self.power_stages:
            print("No Power Stage modules available")
            return

        try:
            print("\n=== Power Stage Usage ===")

            for i, power_stage in enumerate(self.power_stages):
                print(f"\nPower Stage {i+1}:")

                # Print module info
                power_stage.printModuleInfo()

                # Get status using wrapper function
                status, result = power_stage.getStatus()
                if result == pyCandle.PDS_Error_t.OK:
                    print(f"  Status - Enabled: {status.ENABLED}")
                    print(
                        f"  Status - Over Temperature: {status.OVER_TEMPERATURE}")
                    print(f"  Status - Over Current: {status.OVER_CURRENT}")
                else:
                    print(f"  Error getting status: {result}")

                # Get enabled state using wrapper function
                enabled, result = power_stage.getEnabled()
                if result == pyCandle.PDS_Error_t.OK:
                    print(f"  Enabled: {enabled}")
                else:
                    print(f"  Error getting enabled state: {result}")

                # Get measurements using wrapper functions
                temperature, result = power_stage.getTemperature()
                if result == pyCandle.PDS_Error_t.OK:
                    print(f"  Temperature: {temperature}°C")
                else:
                    print(f"  Error getting temperature: {result}")

                output_voltage, result = power_stage.getOutputVoltage()
                if result == pyCandle.PDS_Error_t.OK:
                    print(f"  Output Voltage: {output_voltage}mV")
                else:
                    print(f"  Error getting output voltage: {result}")

                load_current, result = power_stage.getLoadCurrent()
                if result == pyCandle.PDS_Error_t.OK:
                    print(f"  Load Current: {load_current}mA")
                else:
                    print(f"  Error getting load current: {result}")

                power, result = power_stage.getPower()
                if result == pyCandle.PDS_Error_t.OK:
                    print(f"  Power: {power}mW")
                else:
                    print(f"  Error getting power: {result}")

                energy, result = power_stage.getEnergy()
                if result == pyCandle.PDS_Error_t.OK:
                    print(f"  Energy: {energy}mWh")
                else:
                    print(f"  Error getting energy: {result}")

                # Get configuration parameters using wrapper functions
                autostart, result = power_stage.getAutostart()
                if result == pyCandle.PDS_Error_t.OK:
                    print(f"  Autostart: {autostart}")
                else:
                    print(f"  Error getting autostart: {result}")

                ocd_level, result = power_stage.getOcdLevel()
                if result == pyCandle.PDS_Error_t.OK:
                    print(f"  OCD Level: {ocd_level}mA")
                else:
                    print(f"  Error getting OCD level: {result}")

                ocd_delay, result = power_stage.getOcdDelay()
                if result == pyCandle.PDS_Error_t.OK:
                    print(f"  OCD Delay: {ocd_delay}μs")
                else:
                    print(f"  Error getting OCD delay: {result}")

                temp_limit, result = power_stage.getTemperatureLimit()
                if result == pyCandle.PDS_Error_t.OK:
                    print(f"  Temperature Limit: {temp_limit}°C")
                else:
                    print(f"  Error getting temperature limit: {result}")

                # Get brake resistor binding using wrapper function
                brake_socket, result = power_stage.getBindBrakeResistor()
                if result == pyCandle.PDS_Error_t.OK:
                    print(f"  Bound Brake Resistor Socket: {brake_socket}")
                else:
                    print(f"  Error getting brake resistor binding: {result}")

        except Exception as e:
            print(f"Error demonstrating Power Stage usage: {e}")

    def demonstrate_brake_resistor_usage(self):
        """
        Demonstrate Brake Resistor module usage with updated wrapper functions.
        """
        if not self.brake_resistors:
            print("No Brake Resistor modules available")
            return

        try:
            print("\n=== Brake Resistor Usage ===")

            for i, brake_resistor in enumerate(self.brake_resistors):
                print(f"\nBrake Resistor {i+1}:")

                # Print module info
                brake_resistor.printModuleInfo()

                # Get status using wrapper function
                status, result = brake_resistor.getStatus()
                if result == pyCandle.PDS_Error_t.OK:
                    print(f"  Status - Enabled: {status.ENABLED}")
                    print(
                        f"  Status - Over Temperature: {status.OVER_TEMPERATURE}")
                else:
                    print(f"  Error getting status: {result}")

                # Get enabled state using wrapper function
                enabled, result = brake_resistor.getEnabled()
                if result == pyCandle.PDS_Error_t.OK:
                    print(f"  Enabled: {enabled}")
                else:
                    print(f"  Error getting enabled state: {result}")

                # Get temperature using wrapper function
                temperature, result = brake_resistor.getTemperature()
                if result == pyCandle.PDS_Error_t.OK:
                    print(f"  Temperature: {temperature}°C")
                else:
                    print(f"  Error getting temperature: {result}")

                # Get temperature limit using wrapper function
                temp_limit, result = brake_resistor.getTemperatureLimit()
                if result == pyCandle.PDS_Error_t.OK:
                    print(f"  Temperature Limit: {temp_limit}°C")
                else:
                    print(f"  Error getting temperature limit: {result}")

        except Exception as e:
            print(f"Error demonstrating Brake Resistor usage: {e}")

    def run_example(self):
        """
        Run the complete PDS example.
        """
        print("PDS Usage Example")
        print("=" * 50)

        # Step 1: Connect to CANdle
        if not self.connect_candle():
            return False

        # Step 2: Discover PDS devices
        pds_ids = self.discover_pds_devices()
        if not pds_ids:
            print("No PDS devices found. Make sure PDS hardware is connected.")
            return False

        # Step 3: Connect to first PDS device
        if not self.connect_pds(pds_ids[0]):
            return False

        # Step 4: Get PDS information
        self.get_pds_info()

        # Step 5: Discover modules
        self.discover_modules()

        # Step 6: Attach to modules
        self.attach_modules()

        # Step 7: Demonstrate module usage with updated wrapper functions
        self.demonstrate_power_stage_usage()
        self.demonstrate_brake_resistor_usage()
        self.demonstrate_isolated_converter_usage()

        print("\n" + "=" * 50)

        return True


def main():
    """
    Main function to run the PDS example.
    """
    # Set logging verbosity (optional)
    pyCandle.logVerbosity(pyCandle.Verbosity_E.VERBOSITY_1)

    # Create and run updated example
    example = PDSExample()

    example.run_example()


if __name__ == "__main__":
    main()
