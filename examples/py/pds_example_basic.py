#!/usr/bin/env python3
"""
PDS (Power Distribution System) Example Script

This script demonstrates how to use the PDS Python bindings to interact with
PDS devices connected via CANdle. It shows basic usage patterns for:
- Connecting to PDS devices
- Module discovery and management
- Status monitoring
- Configuration management
- Working with different module types (Power Stage, Brake Resistor, Isolated Converter)

Requirements:
- CANdle device connected via USB
- PDS device(s) connected to the CANdle
- Python bindings built and available
"""

import sys
import os
import time

# Add the build directory to the path to import pyCandle
sys.path.insert(0, os.path.join(os.path.dirname(__file__), 'build', 'pycandle'))

try:
    import pyCandle
    print("Successfully imported pyCandle module")
except ImportError as e:
    print(f"Error: Failed to import pyCandle module: {e}")
    print("Make sure the Python bindings are built and the build directory is correct.")
    sys.exit(1)


class PDSExample:
    """
    Example class demonstrating PDS usage patterns.
    """

    def __init__(self):
        self.candle = None
        self.pds = None
        self.power_stages = []
        self.brake_resistors = []
        self.isolated_converters = []

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
            print(f"Attempting to connect to CANdle with baudrate {baudrate} and bus type {bus_type}")
            self.candle = pyCandle.attachCandle(baudrate, bus_type)

            if self.candle is None:
                print("Failed to attach CANdle device")
                return False

            print("✓ Successfully connected to CANdle device")
            return True

        except Exception as e:
            print(f"Error connecting to CANdle: {e}")
            return False

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
            result = self.pds.init(can_id)

            print("✓ Successfully connected to PDS device")
            return True

        except Exception as e:
            print(f"Error connecting to PDS: {e}")
            return False

    def get_pds_info(self):
        """
        Get basic information about the connected PDS device.
        """
        if not self.pds:
            print("Error: PDS not connected")
            return

        try:
            print("\n=== PDS Device Information ===")

            # Get firmware metadata
            metadata = pyCandle.pdsFwMetadata_S()
            result = self.pds.getFwMetadata(metadata)
            if result == pyCandle.PDS_Error_t.OK:
                print(f"Firmware version: {metadata.version}")
                print(f"Git hash: {metadata.gitHash}")
            else:
                print(f"Failed to get firmware metadata: {result}")

            # Get CAN configuration
            can_id = self.pds.getCanId()
            print(f"CAN ID: {can_id}")

            baudrate = self.pds.getCanBaudrate()
            print(f"CAN Baudrate: {baudrate}")

            # Get status
            status = pyCandle.controlBoardStatus_S()
            result = self.pds.getStatus(status)
            if result == pyCandle.PDS_Error_t.OK:
                print(f"Status - Enabled: {status.ENABLED}")
                print(f"Status - Over Temperature: {status.OVER_TEMPERATURE}")
                print(f"Status - Over Current: {status.OVER_CURRENT}")
            else:
                print(f"Failed to get status: {result}")

            # Get temperature and voltage
            temperature = [0.0]  # Using list for reference parameter
            result = self.pds.getTemperature(temperature)
            if result == pyCandle.PDS_Error_t.OK:
                print(f"Temperature: {temperature[0]}°C")

            bus_voltage = [0]  # Using list for reference parameter
            result = self.pds.getBusVoltage(bus_voltage)
            if result == pyCandle.PDS_Error_t.OK:
                print(f"Bus Voltage: {bus_voltage[0]}mV")

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
            print(f"  Socket 1: {pyCandle.Pds.moduleTypeToString(modules.moduleTypeSocket1)}")
            print(f"  Socket 2: {pyCandle.Pds.moduleTypeToString(modules.moduleTypeSocket2)}")
            print(f"  Socket 3: {pyCandle.Pds.moduleTypeToString(modules.moduleTypeSocket3)}")
            print(f"  Socket 4: {pyCandle.Pds.moduleTypeToString(modules.moduleTypeSocket4)}")
            print(f"  Socket 5: {pyCandle.Pds.moduleTypeToString(modules.moduleTypeSocket5)}")
            print(f"  Socket 6: {pyCandle.Pds.moduleTypeToString(modules.moduleTypeSocket6)}")

            # Print module information
            self.pds.printModuleInfo()

        except Exception as e:
            print(f"Error discovering modules: {e}")

    def attach_modules(self):
        """
        Attach to discovered modules and store references.
        """
        if not self.pds:
            print("Error: PDS not connected")
            return

        try:
            print("\n=== Attaching Modules ===")

            # Try to attach modules on each socket
            for socket_num in range(1, 7):
                socket = getattr(pyCandle.socketIndex_E, f'SOCKET_{socket_num}')

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
                        print(f"✓ Attached Brake Resistor at socket {socket_num}")

                # Try Isolated Converter
                elif self.pds.verifyModuleSocket(pyCandle.moduleType_E.ISOLATED_CONVERTER, socket):
                    isolated_conv = self.pds.attachIsolatedConverter(socket)
                    if isolated_conv:
                        self.isolated_converters.append(isolated_conv)
                        print(f"✓ Attached Isolated Converter at socket {socket_num}")

            print(f"Total attached modules: {len(self.power_stages)} Power Stages, "
                  f"{len(self.brake_resistors)} Brake Resistors, "
                  f"{len(self.isolated_converters)} Isolated Converters")

        except Exception as e:
            print(f"Error attaching modules: {e}")

    def demonstrate_power_stage_usage(self):
        """
        Demonstrate Power Stage module usage.
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

                # Get status
                status = pyCandle.powerStageStatus_S()
                result = power_stage.getStatus(status)
                if result == pyCandle.PDS_Error_t.OK:
                    print(f"  Status - Enabled: {status.ENABLED}")
                    print(f"  Status - Over Temperature: {status.OVER_TEMPERATURE}")
                    print(f"  Status - Over Current: {status.OVER_CURRENT}")

                # Get measurements
                temperature = [0.0]
                if power_stage.getTemperature(temperature) == pyCandle.PDS_Error_t.OK:
                    print(f"  Temperature: {temperature[0]}°C")

                voltage = [0]
                if power_stage.getOutputVoltage(voltage) == pyCandle.PDS_Error_t.OK:
                    print(f"  Output Voltage: {voltage[0]}mV")

                current = [0]
                if power_stage.getLoadCurrent(current) == pyCandle.PDS_Error_t.OK:
                    print(f"  Load Current: {current[0]}mA")

                power = [0]
                if power_stage.getPower(power) == pyCandle.PDS_Error_t.OK:
                    print(f"  Power: {power[0]}mW")

                # Example configuration (be careful with these!)
                print("  Example configuration commands (commented out for safety):")
                print("  # power_stage.setTemperatureLimit(85.0)  # Set temperature limit to 85°C")
                print("  # power_stage.setOcdLevel(5000)          # Set overcurrent detection to 5A")
                print("  # power_stage.setOcdDelay(1000)          # Set OCD delay to 1ms")

        except Exception as e:
            print(f"Error demonstrating Power Stage usage: {e}")

    def demonstrate_brake_resistor_usage(self):
        """
        Demonstrate Brake Resistor module usage.
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

                # Get status
                status = pyCandle.brakeResistorStatus_S()
                result = brake_resistor.getStatus(status)
                if result == pyCandle.PDS_Error_t.OK:
                    print(f"  Status - Enabled: {status.ENABLED}")
                    print(f"  Status - Over Temperature: {status.OVER_TEMPERATURE}")

                # Get temperature
                temperature = [0.0]
                if brake_resistor.getTemperature(temperature) == pyCandle.PDS_Error_t.OK:
                    print(f"  Temperature: {temperature[0]}°C")

                # Example configuration
                print("  Example configuration commands (commented out for safety):")
                print("  # brake_resistor.setTemperatureLimit(100.0)  # Set temperature limit")

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

        # Step 7: Demonstrate module usage
        self.demonstrate_power_stage_usage()
        self.demonstrate_brake_resistor_usage()

        print("\n" + "=" * 50)
        print("Example completed successfully!")
        print("This example showed basic PDS interaction patterns.")
        print("For production use, add proper error handling and safety checks.")

        return True


def main():
    """
    Main function to run the PDS example.
    """
    # Set logging verbosity (optional)
    pyCandle.logVerbosity(pyCandle.Verbosity_E.VERBOSITY_1)

    # Create and run example
    example = PDSExample()

    try:
        success = example.run_example()
        if not success:
            print("Example failed to complete")
            sys.exit(1)
    except KeyboardInterrupt:
        print("\nExample interrupted by user")
    except Exception as e:
        print(f"Unexpected error: {e}")
        sys.exit(1)


if __name__ == "__main__":
    main()
