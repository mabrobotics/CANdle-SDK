#include "ui.hpp"

#include <iomanip>
#include <iostream>
#include <streambuf>
#include "manufacturer_data.hpp"

namespace ui
{
    class mystreambuf : public std::streambuf
    {
    };
    mystreambuf  nostreambuf;
    std::ostream nocout(&nostreambuf);
#define vout (log)  // For easy customization later on

    Logger log(Logger::ProgramLayer_E::TOP, "Candle");

    void printVersion(const std::string& version)
    {
        std::cerr << "[CANDLESDK] Version: " << version << std::endl;
    }

    void printLatencyTestResult(uint8_t actuatorCount, float average, float stdev, std::string bus)
    {
        vout << std::fixed;
        vout << "**********************************************************************************"
                "********************************************"
             << std::endl;
        vout << std::endl;
        vout << "Communication speed results during 10s test for " << (int)actuatorCount
             << " actuators and " << bus << " bus" << std::endl;
        vout << "Average speed: " << std::setprecision(2) << average << "Hz" << std::endl;
        vout << "Standard deviation: " << std::setprecision(2) << stdev << "Hz" << std::endl;
        vout << std::endl;
        vout << "For more information on this test please refer to the manual: "
             << GREEN__("https://mabrobotics.pl/servos/manual") << std::endl;
        vout << std::endl;
        vout << "**********************************************************************************"
                "********************************************"
             << std::endl;
    }

    bool getCalibrationConfirmation()
    {
        vout << "This step will start drive calibration. If calibration is done incorrectly or "
                "fails the drive will not move. In such case please rerun the calibration and if "
                "the problem persists contact MABRobotics."
             << std::endl;
        vout << "The process takes around 40-50 seconds, and should not be cancelled or stopped."
             << std::endl;
        vout << "Ensure that the power supply's voltage is stable @24V and it is able to deliver "
                "more than 1A of current."
             << std::endl;
        vout << "For proper calibration, there mustn't be any load on the actuator's output shaft, "
                "ideally there shouldn't be anything attached to the output shaft."
             << std::endl;
        vout << std::endl;
        vout << "For more information please refer to the manual: "
             << GREEN__("https://mabrobotics.pl/servos/manual") << std::endl;
        vout << std::endl;
        vout << "Are you sure you want to start the calibration? [Y/n]" << std::endl;
        char x;
        std::cin >> x;
        if (x != 'Y' && x != 'y')
        {
            vout << "Calibration abandoned." << std::endl;
            return false;
        }
        return true;
    }

    bool getCalibrationOutputConfirmation()
    {
        vout << "This step will start output encoder calibration." << std::endl;
        vout << "The process takes around 40-50 seconds, and should not be cancelled or stopped."
             << std::endl;
        vout << "Ensure that the power supply's voltage is stable and it is able to deliver more "
                "than 1A of current."
             << std::endl;
        vout << "The actuator output shaft (after the gearbox) will move - make sure it is able to "
                "rotate for at least two full rotations."
             << std::endl;
        vout << std::endl;
        vout << "For more information please refer to the manual: "
             << GREEN__("https://mabrobotics.pl/servos/manual") << std::endl;
        vout << std::endl;
        vout << "Are you sure you want to start the calibration? [Y/n]" << std::endl;
        char x;
        std::cin >> x;
        if (x != 'Y' && x != 'y')
        {
            vout << "Output encoder calibration abandoned." << std::endl;
            return false;
        }
        return true;
    }

    void printPositionVelocity(int id, float pos)
    {
        vout << "Drive " << id << " Position: " << pos << std::endl;
    }

    void printPositionAndVelocity(int id, float pos, float velocity)
    {
        vout << "Drive " << id << " Position: " << pos << "\tVelocity: " << velocity << std::endl;
    }

    void printFoundDrives(std::vector<uint16_t> ids)
    {
        if (ids.size() == 0)
        {
            vout << "No drives found." << std::endl;
            return;
        }
        vout << "Found drives." << std::endl;
        for (size_t i = 0; i < ids.size(); i++)
        {
            if (ids[i] == 0)
                break;  // No more ids in the message

            vout << std::to_string(i + 1) << ": ID = " << ids[i] << " (0x" << std::hex << ids[i]
                 << std::dec << ")" << std::endl;
            if (ids[i] > 2047)
            {
                vout << "Error! This ID is invalid! Probably two or more drives share same ID."
                     << "Communication will most likely be broken until IDs are unique! [FAILED] "
                     << std::endl;
                std::vector<uint16_t> empty;
                return;
            }
        }
    }

    void printDriveInfoExtended(mab::MD& drive, const mab::MDRegisters_S& registers, bool printAll)
    {
        auto getStringBuildDate = [](uint32_t date)
        {
            return std::to_string(date % 100) + '.' + std::to_string((date / 100) % 100) + '.' +
                   "20" + std::to_string(date / 10000);
        };

        auto getLegacyHardwareVersion = [](uint8_t version)
        {
            switch (version)
            {
                case 0:
                    return "HV13";
                case 1:
                    return "HW11";
                case 2:
                    return "HW20";
                case 3:
                    return "HW21";
                case 4:
                    return "HW30";
                default:
                    return "UNKNOWN";
            }
        };

        auto getListElement = [](std::vector<std::string> vec, uint32_t idx)
        {
            if (idx < vec.size())
                return vec[idx];
            else
                return std::string("UNKNOWN (") + std::to_string(idx) + std::string(")");
        };

        vout << std::fixed;
        vout << "Drive " << drive.m_canId << ":" << std::endl;
        vout << "- actuator name: " << registers.motorName.value << std::endl;
        vout << "- CAN speed: " << registers.canBaudrate.value / 1000000 << " M" << std::endl;
        vout << "- CAN termination resistor: "
             << ((registers.canTermination.value == true) ? "enabled" : "disabled") << std::endl;
        vout << "- gear ratio: " << std::setprecision(5) << registers.motorGearRatio.value
             << std::endl;
        mab::version_ut firmwareVersion = {{0, 0, 0, 0}};
        firmwareVersion.i               = registers.firmwareVersion.value;
        vout << "- firmware version: v" << (int)firmwareVersion.s.major << "."
             << (int)firmwareVersion.s.minor << "." << (int)firmwareVersion.s.revision << "."
             << firmwareVersion.s.tag << std::endl;
        vout << "- hardware version(legacy): "
             << "Not implemented yet" << std::endl;
        vout << "- hardware type: "
             << getLegacyHardwareVersion(registers.legacyHardwareVersion.value) << std::endl;
        vout << "- build date: " << getStringBuildDate(registers.buildDate.value) << std::endl;
        vout << "- commit hash: " << registers.commitHash.value[0]
             << std::endl;  // TODO: printable format
        vout << "- max current: " << std::setprecision(1) << registers.motorIMax.value << " A"
             << std::endl;
        vout << "- bridge type: " << std::to_string(registers.bridgeType.value) << std::endl;
        vout << "- shunt resistance: " << std::setprecision(4) << registers.shuntResistance.value
             << " Ohm" << std::endl;
        vout << "- pole pairs: " << std::to_string(registers.motorPolePairs.value) << std::endl;
        vout << "- KV rating: " << std::to_string(registers.motorKV.value) << " rpm/V" << std::endl;
        vout << "- motor shutdown temperature: "
             << std::to_string(registers.motorShutdownTemp.value) << " *C" << std::endl;
        vout << "- motor calibration mode: "
             << motorCalibrationModes[registers.motorCalibrationMode.value] << std::endl;
        vout << "- motor torque constant: " << std::setprecision(4) << registers.motorKt.value
             << " Nm/A" << std::endl;
        vout << "- d-axis resistance: " << std::setprecision(3) << registers.motorResistance.value
             << " Ohm" << std::endl;
        vout << "- d-axis inductance: " << std::setprecision(6) << registers.motorInductance.value
             << " H" << std::endl;
        vout << "- torque bandwidth: " << registers.motorTorqueBandwidth.value << " Hz"
             << std::endl;
        vout << "- CAN watchdog: " << registers.canWatchdog.value << " ms" << std::endl;
        vout << "- GPIO mode: " << getListElement(GPIOmodes, registers.userGpioConfiguration.value)
             << std::endl;

        if (printAll)
        {
            float stddevE = registers.calMainEncoderStdDev.value;
            float minE    = registers.calMainEncoderMinE.value;
            float maxE    = registers.calMainEncoderMaxE.value;
            vout << "- main encoder last check error standard deviation: "
                 << (stddevE < mainEncoderStdDevMax ? std::to_string(stddevE)
                                                    : YELLOW_(std::to_string(stddevE)))
                 << " rad" << std::endl;
            vout << "- main encoder last check max negative error: "
                 << (minE > -mainEncoderMaxError ? std::to_string(minE)
                                                 : YELLOW_(std::to_string(minE)))
                 << " rad" << std::endl;
            vout << "- main encoder last check max positive error: "
                 << (maxE < mainEncoderMaxError ? std::to_string(maxE)
                                                : YELLOW_(std::to_string(maxE)))
                 << " rad" << std::endl;
        }

        vout << "- output encoder: "
             << (registers.auxEncoder.value
                     ? getListElement(encoderTypes, registers.auxEncoder.value)
                     : "no")
             << std::endl;

        if (registers.auxEncoder.value != 0)
        {
            vout << "   - output encoder mode: "
                 << getListElement(encoderModes, registers.auxEncoderMode.value) << std::endl;
            vout << "   - output encoder calibration mode: "
                 << getListElement(encoderCalibrationModes,
                                   registers.auxEncoderCalibrationMode.value)
                 << std::endl;
            vout << "   - output encoder position: " << registers.auxEncoderPosition.value << " rad"
                 << std::endl;
            vout << "   - output encoder velocity: " << registers.auxEncoderVelocity.value
                 << " rad/s" << std::endl;

            if (printAll)
            {
                float stddevE = registers.calAuxEncoderStdDev.value;
                float minE    = registers.calAuxEncoderMinE.value;
                float maxE    = registers.calAuxEncoderMaxE.value;
                vout << "   - output encoder last check error stddev: "
                     << (stddevE < outputEncoderStdDevMax ? std::to_string(stddevE)
                                                          : YELLOW_(std::to_string(stddevE)))
                     << " rad" << std::endl;
                vout << "   - output encoder last check max negative error: "
                     << (minE > -outputEncoderMaxError ? std::to_string(minE)
                                                       : YELLOW_(std::to_string(minE)))
                     << " rad" << std::endl;
                vout << "   - output encoder last check max positive error: "
                     << (maxE < outputEncoderMaxError ? std::to_string(maxE)
                                                      : YELLOW_(std::to_string(maxE)))
                     << " rad" << std::endl;
            }
        }

        vout << "- homing: "
             << (registers.homingMode.value
                     ? getListElement(homingModes, registers.homingMode.value)
                     : "off")
             << std::endl;

        if (registers.homingMode.value != 0)
        {
            vout << "   - homing max travel: " << std::setprecision(2)
                 << registers.homingMaxTravel.value << " rad" << std::endl;
            vout << "   - homing max torque: " << std::setprecision(2)
                 << registers.homingTorque.value << " Nm" << std::endl;
            vout << "   - homing max velocity: " << std::setprecision(2)
                 << registers.homingVelocity.value << " rad/s" << std::endl;
        }
        vout << "- motion limits: " << std::endl;
        vout << "   - max torque: " << std::setprecision(2) << registers.maxTorque.value << " Nm"
             << std::endl;
        vout << "   - max acceleration: " << std::setprecision(2) << registers.maxAcceleration.value
             << " rad/s^2" << std::endl;
        vout << "   - max deceleration: " << std::setprecision(2) << registers.maxDeceleration.value
             << " rad/s^2" << std::endl;
        vout << "   - max velocity: " << std::setprecision(2) << registers.maxVelocity.value
             << " rad/s" << std::endl;
        vout << "   - position limit min: " << std::setprecision(2)
             << registers.positionLimitMin.value << " rad" << std::endl;
        vout << "   - position limit max: " << std::setprecision(2)
             << registers.positionLimitMax.value << " rad" << std::endl;

        vout << "- position: " << std::setprecision(2) << drive.getPosition().first << " rad"
             << std::endl;
        vout << "- velocity: " << std::setprecision(2) << drive.getVelocity().first << " rad/s"
             << std::endl;
        vout << "- torque: " << std::setprecision(2) << drive.getTorque().first << " Nm"
             << std::endl;
        vout << "- MOSFET temperature: " << std::setprecision(2)
             << registers.mosfetTemperature.value << " *C" << std::endl;
        vout << "- motor temperature: " << std::setprecision(2) << registers.motorTemperature.value
             << " *C" << std::endl;
        vout << std::endl;

        vout << "***** ERRORS *****" << std::endl;
        printAllErrors(std::reference_wrapper(registers));
    }

    void printAllErrors(const mab::MDRegisters_S& registers)
    {
        vout << "- main encoder error: 	0x" << std::hex
             << (unsigned short)registers.mainEncoderStatus.value << std::dec;
        printErrorDetails(registers.mainEncoderStatus.value, encoderErrorList);

        if (registers.auxEncoder.value != 0)
        {
            vout << "- output encoder status: 0x" << std::hex
                 << (unsigned short)registers.auxEncoderStatus.value << std::dec;
            printErrorDetails(registers.auxEncoderStatus.value, encoderErrorList);
        }

        vout << "- calibration status: 	0x" << std::hex
             << (unsigned short)registers.calibrationStatus.value << std::dec;
        printErrorDetails(registers.calibrationStatus.value, calibrationErrorList);
        vout << "- bridge status: 	0x" << std::hex << (unsigned short)registers.bridgeStatus.value
             << std::dec;
        printErrorDetails(registers.bridgeStatus.value, bridgeErrorList);
        vout << "- hardware status: 	0x" << std::hex
             << (unsigned short)registers.hardwareStatus.value << std::dec;
        printErrorDetails(registers.hardwareStatus.value, hardwareErrorList);
        vout << "- communication status: 0x" << std::hex
             << (unsigned short)registers.communicationStatus.value << std::dec;
        printErrorDetails(registers.communicationStatus.value, communicationErrorList);
        vout << "- motion status: 	0x" << std::hex << (unsigned short)registers.motionStatus.value
             << std::dec;
    }

    void printErrorDetails(uint32_t error, const std::vector<std::string>& errorList)
    {
        vout << "	(";
        if (error == 0)
        {
            vout << GREEN__("ALL OK") << ")" << std::endl;
            return;
        }

        for (uint32_t i = 0; i < errorList.size(); i++)
        {
            if (error & (1 << i))
                vout << RED_(errorList[i]) << ", ";
        }
        vout << ")";
        vout << std::endl;
    }

    void printErrorDetails(uint32_t error, const std::map<std::string, uint8_t>& errorMap)
    {
        vout << "	(";
        if (error == 0)
        {
            vout << GREEN__("ALL OK") << ")" << std::endl;
            return;
        }

        for (auto& entry : errorMap)
        {
            if (error & (1 << entry.second) && entry.first[0] == 'E')
                vout << RED_(entry.first) << ", ";
            else if (error & (1 << entry.second) && entry.first[0] == 'W')
                vout << YELLOW_(entry.first) << ", ";
        }

        vout << ")";
        vout << std::endl;
    }

    void printParameterOutOfBounds(std::string category, std::string field)
    {
        vout << "Motor config parameter in category [" << category << "] named [" << field
             << "] is out of bounds!" << std::endl;
    }
    void printMotorConfig(mINI::INIStructure Ini)
    {
        vout << "Motor config:" << std::endl;
        for (auto const& it : Ini)
        {
            auto const& section    = it.first;
            auto const& collection = it.second;
            std::cout << "- " << section << ":" << std::endl;
            for (auto const& it2 : collection)
            {
                auto const& key   = it2.first;
                auto const& value = it2.second;
                std::cout << "   - " << key << ": " << value << std::endl;
            }
        }
    }

    bool getDifferentConfigsConfirmation(std::string configName)
    {
        vout << "[CANDLETOOL] The default " << configName << " config was modified." << std::endl;
        vout << "[CANDLETOOL] Would you like to revert it to default before downloading? [Y/n]"
             << std::endl;
        char x;
        std::cin >> x;
        if (x != 'Y' && x != 'y')
        {
            vout << "[CANDLETOOL] Using modified config." << std::endl;
            return false;
        }
        vout << "[CANDLETOOL] Using default config." << std::endl;
        return true;
    }
    bool getUpdateConfigConfirmation(std::string configName)
    {
        vout << "[CANDLETOOL] The " << configName << " config is not complete." << std::endl;
        vout << "[CANDLETOOL] Would you like to update lacking fields with default values before "
                "downloading? [Y/n]"
             << std::endl;
        char x;
        std::cin >> x;
        if (x != 'Y' && x != 'y')
        {
            vout << "[CANDLETOOL] Using unchanged user's config." << std::endl;
            return false;
        }
        vout << "[CANDLETOOL] Updating config with default values." << std::endl;
        vout << "[CANDLETOOL] New confing saved under name: " +
                    configName.substr(0, configName.find_last_of(".")) + "_updated.cfg."
             << std::endl;
        return true;
    }
    bool getSaveConfigConfirmation(std::string configName)
    {
        vout << "[CANDLETOOL] Would you like to save device config under the name: " << configName
             << " in your current directory? [Y/n]" << std::endl;
        char x;
        std::cin >> x;
        if (x != 'Y' && x != 'y')
        {
            vout << "[CANDLETOOL] Reading device config without saving to a file." << std::endl;
            return false;
        }
        vout << "[CANDLETOOL] Saving device config." << std::endl;
        return true;
    }

    bool getOverwriteMotorConfigConfirmation(std::string configName)
    {
        vout << "[CANDLETOOL] The " << configName
             << " file already exist in your current directory, would you like to overwrite the "
                "file? [Y/n]"
             << std::endl;
        char x;
        std::cin >> x;
        if (x != 'Y' && x != 'y')
        {
            return false;
        }
        vout << "[CANDLETOOL] Overwriting the motor config file." << std::endl;
        return true;
    }
    std::string getNewMotorConfigName(std::string configName)
    {
        std::string newName = configName.substr(0, configName.find_last_of(".")) + "_new.cfg";
        vout << "[CANDLETOOL] Please type the new config name." << std::endl
             << "[CANDLETOOL] The default new name is: " << newName << std::endl
             << "[CANDLETOOL] (Press Enter to accept the default)" << std::endl;
        std::string x;
        std::cin.ignore();
        std::getline(std::cin, x);
        if (!x.empty())
        {
            std::string str(x);
            return str;
        }
        return newName;
    }
}  // namespace ui
