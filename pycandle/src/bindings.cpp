#include <algorithm>
#include <pybind11/detail/common.h>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <cstdint>
#include <memory>
#include <vector>

#include "mab_types.hpp"
#include "candle_types.hpp"
#include "candle.hpp"
#include "MD.hpp"
#include "logger.hpp"
#include "pds.hpp"
#include "pds_module.hpp"
#include "pds_types.hpp"
#include "pds_properties.hpp"
#include "brake_resistor.hpp"
#include "power_stage.hpp"
#include "isolated_converter.hpp"

namespace py = pybind11;

namespace mab
{
    Candle* pyAttachCandle(const CANdleDatarate_E datarate, candleTypes::busTypes_t busType)
    {
        return attachCandle(datarate, busType);
    }

    MD createMD(int canId, std::shared_ptr<Candle> candle)
    {
        return MD(canId, candle.get());
    }

    template <typename T>
    std::pair<T, MD::Error_t> readReg(MD& md, const std::string& regName)
    {
        Logger log(Logger::ProgramLayer_E::TOP, "MD_READ_REG");

        MDRegisters_S mdRegisters;
        bool          found = false;

        T           value = T{};
        MD::Error_t err   = MD::Error_t::OK;

        auto getReg = [&]<typename R>(MDRegisterEntry_S<R>& reg)
        {
            if constexpr (std::is_same_v<T, R>)
            {
                if (reg.m_name == regName)
                {
                    found = true;
                    err   = md.readRegisters(reg);
                    value = reg.value;
                }
            }
        };

        mdRegisters.forEachRegister(getReg);
        if (!found)
        {
            log.error("Wrong name or type!");
            err = MD::Error_t::REQUEST_INVALID;
        }
        return std::make_pair(value, err);
    }

    std::pair<std::string, MD::Error_t> readRegString(MD& md, const std::string& regName)
    {
        Logger log(Logger::ProgramLayer_E::TOP, "MD_READ_REG");

        MDRegisters_S mdRegisters;
        bool          found = false;

        std::string value = "";
        MD::Error_t err   = MD::Error_t::OK;

        auto getReg = [&]<typename R>(MDRegisterEntry_S<R>& reg)
        {
            if constexpr (std::is_same<std::decay_t<R>, char*>::value)
            {
                if (reg.m_name == regName)
                {
                    found = true;
                    err   = md.readRegisters(reg);
                    value = std::string(reg.value);
                }
            }
        };

        mdRegisters.forEachRegister(getReg);
        if (!found)
        {
            log.error("Wrong name or type!");
            err = MD::Error_t::REQUEST_INVALID;
        }
        return std::make_pair(value, err);
    }

    template <typename T>
    std::pair<py::array_t<T>, MD::Error_t> readRegArray(MD& md, const std::string& regName)
    {
        Logger log(Logger::ProgramLayer_E::TOP, "MD_READ_REG_ARRAY");

        MDRegisters_S mdRegisters;
        bool          found = false;

        py::array_t<T> value;
        MD::Error_t    err = MD::Error_t::OK;

        auto getReg = [&]<typename R>(MDRegisterEntry_S<R>& reg)
        {
            if constexpr (std::is_array_v<R> && std::is_same_v<T, std::remove_extent_t<R>>)
            {
                if (reg.m_name == regName)
                {
                    found                         = true;
                    constexpr size_t reg_elements = std::extent_v<R>;
                    value                         = py::array_t<T>(reg_elements);

                    md.readRegisters(reg);
                    T* dest = static_cast<T*>(value.request().ptr);
                    memcpy(dest, reg.value, reg_elements * sizeof(T));
                }
            }
        };

        mdRegisters.forEachRegister(getReg);
        if (!found)
        {
            log.error("Wrong name or type!");
            err = MD::Error_t::REQUEST_INVALID;
        }
        return std::make_pair(value, err);
    }

    template <typename T>
    MD::Error_t writeReg(MD& md, const std::string& regName, T value)
    {
        Logger log(Logger::ProgramLayer_E::TOP, "MD_WRITE_REG");

        MDRegisters_S mdRegisters;
        bool          found = false;

        MD::Error_t err = MD::Error_t::OK;

        auto getReg = [&]<typename R>(MDRegisterEntry_S<R>& reg)
        {
            if constexpr (std::is_same_v<T, R>)
            {
                if (reg.m_name == regName)
                {
                    found     = true;
                    reg.value = value;
                    err       = md.writeRegisters(reg);
                }
            }
        };

        mdRegisters.forEachRegister(getReg);
        if (!found)
        {
            log.error("Wrong name or type!");
            err = MD::Error_t::REQUEST_INVALID;
        }
        return err;
    }

    MD::Error_t writeRegString(MD& md, const std::string& regName, const std::string& value)
    {
        Logger log(Logger::ProgramLayer_E::TOP, "MD_WRITE_REG");

        MDRegisters_S mdRegisters;
        bool          found = false;

        MD::Error_t err = MD::Error_t::OK;

        auto getReg = [&]<typename R>(MDRegisterEntry_S<R>& reg)
        {
            if constexpr (std::is_same<std::decay_t<R>, char*>::value)
            {
                if (reg.m_name == regName)
                {
                    found = true;
                    if (value.size() + 1 > sizeof(reg.value))
                    {
                        log.error("String too long!");
                        err = MD::Error_t::REQUEST_INVALID;
                        return;
                    }
                    std::memset(reg.value, 0, sizeof(reg.value));
                    std::strncpy(reg.value, value.c_str(), sizeof(value.c_str()));
                    err = md.writeRegisters(reg);
                }
            }
        };

        mdRegisters.forEachRegister(getReg);
        if (!found)
        {
            log.error("Wrong name or type!");
            err = MD::Error_t::REQUEST_INVALID;
        }
        return err;
    }

    template <typename T>
    MD::Error_t writeRegArray(MD&                                                       md,
                              const std::string&                                        regName,
                              py::array_t<T, py::array::c_style | py::array::forcecast> array)
    {
        Logger log(Logger::ProgramLayer_E::TOP, "MD_WRITE_REG");

        if (array.ndim() != 1)
        {
            log.error("Array must be 1D!");
            return MD::Error_t::REQUEST_INVALID;
        }

        MDRegisters_S mdRegisters;
        bool          found = false;

        MD::Error_t err = MD::Error_t::OK;

        auto getReg = [&]<typename R>(MDRegisterEntry_S<R>& reg)
        {
            if constexpr (std::is_array_v<R> && std::is_same_v<T, std::remove_extent_t<R>>)
            {
                if (reg.m_name == regName)
                {
                    found                         = true;
                    constexpr size_t reg_elements = std::extent_v<R>;
                    if (array.size() != reg_elements)
                    {
                        log.error((std::string("Array size mismatch: expected ") +
                                   std::to_string(reg_elements) + ", got " +
                                   std::to_string(array.size()))
                                      .c_str());
                        err = MD::Error_t::REQUEST_INVALID;
                        return;
                    }
                    std::memcpy(&reg.value, array.data(), array.size() * sizeof(T));
                    md.writeRegisters(reg);
                }
            }
        };

        mdRegisters.forEachRegister(getReg);
        if (!found)
        {
            log.error("Wrong name or type!");
            err = MD::Error_t::REQUEST_INVALID;
        }
        return err;
    }
    // PDS Wrapper Functions

    // Pds class wrappers
    std::pair<pdsFwMetadata_S, PdsModule::error_E> pdsFwMetadataWrapper(Pds& pds)
    {
        pdsFwMetadata_S    metadata;
        PdsModule::error_E error = pds.getFwMetadata(metadata);
        return std::make_pair(metadata, error);
    }

    std::pair<controlBoardStatus_S, PdsModule::error_E> pdsGetStatusWrapper(Pds& pds)
    {
        controlBoardStatus_S status = {0};
        PdsModule::error_E   error  = pds.getStatus(status);
        return std::make_pair(status, error);
    }

    std::pair<u32, PdsModule::error_E> pdsGetBusVoltageWrapper(Pds& pds)
    {
        u32                busVoltage;
        PdsModule::error_E error = pds.getBusVoltage(busVoltage);
        return std::make_pair(busVoltage, error);
    }

    std::pair<f32, PdsModule::error_E> pdsGetTemperatureWrapper(Pds& pds)
    {
        f32                temperature;
        PdsModule::error_E error = pds.getTemperature(temperature);
        return std::make_pair(temperature, error);
    }

    std::pair<f32, PdsModule::error_E> pdsGetTemperatureLimitWrapper(Pds& pds)
    {
        f32                temperatureLimit;
        PdsModule::error_E error = pds.getTemperatureLimit(temperatureLimit);
        return std::make_pair(temperatureLimit, error);
    }

    std::pair<u32, PdsModule::error_E> pdsGetShutdownTimeWrapper(Pds& pds)
    {
        u32                shutdownTime;
        PdsModule::error_E error = pds.getShutdownTime(shutdownTime);
        return std::make_pair(shutdownTime, error);
    }

    std::pair<std::pair<u32, u32>, PdsModule::error_E> pdsGetBatteryVoltageLevelsWrapper(Pds& pds)
    {
        u32                batteryLvl1, batteryLvl2;
        PdsModule::error_E error = pds.getBatteryVoltageLevels(batteryLvl1, batteryLvl2);
        return std::make_pair(std::make_pair(batteryLvl1, batteryLvl2), error);
    }

    std::pair<socketIndex_E, PdsModule::error_E> pdsGetBindBrakeResistorWrapper(Pds& pds)
    {
        socketIndex_E      brakeResistorSocketIndex;
        PdsModule::error_E error = pds.getBindBrakeResistor(brakeResistorSocketIndex);
        return std::make_pair(brakeResistorSocketIndex, error);
    }

    std::pair<u32, PdsModule::error_E> pdsGetBrakeResistorTriggerVoltageWrapper(Pds& pds)
    {
        u32                brTriggerVoltage;
        PdsModule::error_E error = pds.getBrakeResistorTriggerVoltage(brTriggerVoltage);
        return std::make_pair(brTriggerVoltage, error);
    }

    // PowerStage class wrappers
    std::pair<powerStageStatus_S, PdsModule::error_E> powerStageGetStatusWrapper(
        PowerStage& powerStage)
    {
        powerStageStatus_S status;
        PdsModule::error_E error = powerStage.getStatus(status);
        return std::make_pair(status, error);
    }

    std::pair<bool, PdsModule::error_E> powerStageGetEnabledWrapper(PowerStage& powerStage)
    {
        bool               enabled;
        PdsModule::error_E error = powerStage.getEnabled(enabled);
        return std::make_pair(enabled, error);
    }

    std::pair<socketIndex_E, PdsModule::error_E> powerStageGetBindBrakeResistorWrapper(
        PowerStage& powerStage)
    {
        socketIndex_E      brakeResistorSocketIndex;
        PdsModule::error_E error = powerStage.getBindBrakeResistor(brakeResistorSocketIndex);
        return std::make_pair(brakeResistorSocketIndex, error);
    }

    std::pair<u32, PdsModule::error_E> powerStageGetBrakeResistorTriggerVoltageWrapper(
        PowerStage& powerStage)
    {
        u32                brTriggerVoltage;
        PdsModule::error_E error = powerStage.getBrakeResistorTriggerVoltage(brTriggerVoltage);
        return std::make_pair(brTriggerVoltage, error);
    }

    std::pair<u32, PdsModule::error_E> powerStageGetOutputVoltageWrapper(PowerStage& powerStage)
    {
        u32                outputVoltage;
        PdsModule::error_E error = powerStage.getOutputVoltage(outputVoltage);
        return std::make_pair(outputVoltage, error);
    }

    std::pair<bool, PdsModule::error_E> powerStageGetAutostartWrapper(PowerStage& powerStage)
    {
        bool               autoStart;
        PdsModule::error_E error = powerStage.getAutostart(autoStart);
        return std::make_pair(autoStart, error);
    }

    std::pair<s32, PdsModule::error_E> powerStageGetLoadCurrentWrapper(PowerStage& powerStage)
    {
        s32                loadCurrent;
        PdsModule::error_E error = powerStage.getLoadCurrent(loadCurrent);
        return std::make_pair(loadCurrent, error);
    }

    std::pair<s32, PdsModule::error_E> powerStageGetPowerWrapper(PowerStage& powerStage)
    {
        s32                power;
        PdsModule::error_E error = powerStage.getPower(power);
        return std::make_pair(power, error);
    }

    std::pair<u32, PdsModule::error_E> powerStageGetEnergyWrapper(PowerStage& powerStage)
    {
        u32                energy;
        PdsModule::error_E error = powerStage.getTotalDeliveredEnergy(energy);
        return std::make_pair(energy, error);
    }

    std::pair<f32, PdsModule::error_E> powerStageGetTemperatureWrapper(PowerStage& powerStage)
    {
        f32                temperature;
        PdsModule::error_E error = powerStage.getTemperature(temperature);
        return std::make_pair(temperature, error);
    }

    std::pair<u32, PdsModule::error_E> powerStageGetOcdLevelWrapper(PowerStage& powerStage)
    {
        u32                ocdLevel;
        PdsModule::error_E error = powerStage.getOcdLevel(ocdLevel);
        return std::make_pair(ocdLevel, error);
    }

    std::pair<u32, PdsModule::error_E> powerStageGetOcdDelayWrapper(PowerStage& powerStage)
    {
        u32                ocdDelay;
        PdsModule::error_E error = powerStage.getOcdDelay(ocdDelay);
        return std::make_pair(ocdDelay, error);
    }

    std::pair<f32, PdsModule::error_E> powerStageGetTemperatureLimitWrapper(PowerStage& powerStage)
    {
        f32                temperatureLimit;
        PdsModule::error_E error = powerStage.getTemperatureLimit(temperatureLimit);
        return std::make_pair(temperatureLimit, error);
    }

    // BrakeResistor class wrappers
    std::pair<brakeResistorStatus_S, PdsModule::error_E> brakeResistorGetStatusWrapper(
        BrakeResistor& brakeResistor)
    {
        brakeResistorStatus_S status;
        PdsModule::error_E    error = brakeResistor.getStatus(status);
        return std::make_pair(status, error);
    }

    std::pair<bool, PdsModule::error_E> brakeResistorGetEnabledWrapper(BrakeResistor& brakeResistor)
    {
        bool               enabled;
        PdsModule::error_E error = brakeResistor.getEnabled(enabled);
        return std::make_pair(enabled, error);
    }

    std::pair<f32, PdsModule::error_E> brakeResistorGetTemperatureWrapper(
        BrakeResistor& brakeResistor)
    {
        f32                temperature;
        PdsModule::error_E error = brakeResistor.getTemperature(temperature);
        return std::make_pair(temperature, error);
    }

    std::pair<f32, PdsModule::error_E> brakeResistorGetTemperatureLimitWrapper(
        BrakeResistor& brakeResistor)
    {
        f32                temperatureLimit;
        PdsModule::error_E error = brakeResistor.getTemperatureLimit(temperatureLimit);
        return std::make_pair(temperatureLimit, error);
    }

    // IsolatedConv class wrappers
    std::pair<isolatedConverterStatus_S, PdsModule::error_E> isolatedConvGetStatusWrapper(
        IsolatedConv& isolatedConv)
    {
        isolatedConverterStatus_S status;
        PdsModule::error_E        error = isolatedConv.getStatus(status);
        return std::make_pair(status, error);
    }

    std::pair<bool, PdsModule::error_E> isolatedConvGetEnabledWrapper(IsolatedConv& isolatedConv)
    {
        bool               enabled;
        PdsModule::error_E error = isolatedConv.getEnabled(enabled);
        return std::make_pair(enabled, error);
    }

    std::pair<u32, PdsModule::error_E> isolatedConvGetOutputVoltageWrapper(
        IsolatedConv& isolatedConv)
    {
        u32                outputVoltage;
        PdsModule::error_E error = isolatedConv.getOutputVoltage(outputVoltage);
        return std::make_pair(outputVoltage, error);
    }

    std::pair<s32, PdsModule::error_E> isolatedConvGetLoadCurrentWrapper(IsolatedConv& isolatedConv)
    {
        s32                loadCurrent;
        PdsModule::error_E error = isolatedConv.getLoadCurrent(loadCurrent);
        return std::make_pair(loadCurrent, error);
    }

    std::pair<s32, PdsModule::error_E> isolatedConvGetPowerWrapper(IsolatedConv& isolatedConv)
    {
        s32                power;
        PdsModule::error_E error = isolatedConv.getPower(power);
        return std::make_pair(power, error);
    }

    std::pair<s32, PdsModule::error_E> isolatedConvGetEnergyWrapper(IsolatedConv& isolatedConv)
    {
        s32                energy;
        PdsModule::error_E error = isolatedConv.getEnergy(energy);
        return std::make_pair(energy, error);
    }

    std::pair<f32, PdsModule::error_E> isolatedConvGetTemperatureWrapper(IsolatedConv& isolatedConv)
    {
        f32                temperature;
        PdsModule::error_E error = isolatedConv.getTemperature(temperature);
        return std::make_pair(temperature, error);
    }

    std::pair<u32, PdsModule::error_E> isolatedConvGetOcdLevelWrapper(IsolatedConv& isolatedConv)
    {
        u32                ocdLevel;
        PdsModule::error_E error = isolatedConv.getOcdLevel(ocdLevel);
        return std::make_pair(ocdLevel, error);
    }

    std::pair<u32, PdsModule::error_E> isolatedConvGetOcdDelayWrapper(IsolatedConv& isolatedConv)
    {
        u32                ocdDelay;
        PdsModule::error_E error = isolatedConv.getOcdDelay(ocdDelay);
        return std::make_pair(ocdDelay, error);
    }

    std::pair<f32, PdsModule::error_E> isolatedConvGetTemperatureLimitWrapper(
        IsolatedConv& isolatedConv)
    {
        f32                temperatureLimit;
        PdsModule::error_E error = isolatedConv.getTemperatureLimit(temperatureLimit);
        return std::make_pair(temperatureLimit, error);
    }

}  // namespace mab

PYBIND11_MODULE(pyCandle, m)
{
    m.doc() = "pyCandle module for interfacing with MD drives using Python";

    // CANdle class

    py::enum_<mab::CANdleDatarate_E>(m, "CANdleDatarate_E")
        .value("CAN_DATARATE_1M", mab::CAN_DATARATE_1M)
        .value("CAN_DATARATE_2M", mab::CAN_DATARATE_2M)
        .value("CAN_DATARATE_5M", mab::CAN_DATARATE_5M)
        .value("CAN_DATARATE_8M", mab::CAN_DATARATE_8M)
        .export_values();

    py::enum_<mab::candleTypes::busTypes_t>(m, "busTypes_t")
        .value("USB", mab::candleTypes::busTypes_t::USB)
        .value("SPI", mab::candleTypes::busTypes_t::SPI)
        .export_values();

    py::enum_<mab::candleTypes::Error_t>(m, "CandleTypesError")
        .value("OK", mab::candleTypes::OK)
        .value("DEVICE_NOT_CONNECTED", mab::candleTypes::DEVICE_NOT_CONNECTED)
        .value("INITIALIZATION_ERROR", mab::candleTypes::INITIALIZATION_ERROR)
        .value("UNINITIALIZED", mab::candleTypes::UNINITIALIZED)
        .value("DATA_TOO_LONG", mab::candleTypes::DATA_TOO_LONG)
        .value("DATA_EMPTY", mab::candleTypes::DATA_EMPTY)
        .value("RESPONSE_TIMEOUT", mab::candleTypes::RESPONSE_TIMEOUT)
        .value("CAN_DEVICE_NOT_RESPONDING", mab::candleTypes::CAN_DEVICE_NOT_RESPONDING)
        .value("INVALID_ID", mab::candleTypes::INVALID_ID)
        .value("BAD_RESPONSE", mab::candleTypes::BAD_RESPONSE)
        .value("UNKNOWN_ERROR", mab::candleTypes::UNKNOWN_ERROR)
        .export_values();

    m.def("attachCandle",
          &mab::pyAttachCandle,
          py::arg("datarate"),
          py::arg("busType"),
          py::return_value_policy::take_ownership,
          "Attach a CANdle device to the system.");

    py::class_<mab::Candle>(m, "Candle");

    // MD class
    py::enum_<mab::MD::Error_t>(m, "MD_Error_t")
        .value("OK", mab::MD::Error_t::OK)
        .value("REQUEST_INVALID", mab::MD::Error_t::REQUEST_INVALID)
        .value("TRANSFER_FAILED", mab::MD::Error_t::TRANSFER_FAILED)
        .value("NOT_CONNECTED", mab::MD::Error_t::NOT_CONNECTED)
        .export_values();

    py::enum_<mab::MdMode_E>(m, "MotionMode_t")
        .value("IDLE", mab::MdMode_E::IDLE)
        .value("POSITION_PID", mab::MdMode_E::POSITION_PID)
        .value("VELOCITY_PID", mab::MdMode_E::VELOCITY_PID)
        .value("RAW_TORQUE", mab::MdMode_E::RAW_TORQUE)
        .value("IMPEDANCE", mab::MdMode_E::IMPEDANCE)
        .value("PROFILE_POSITION", mab::MdMode_E::POSITION_PROFILE)
        .value("PROFILE_VELOCITY", mab::MdMode_E::VELOCITY_PROFILE)
        .export_values();

    py::class_<mab::MD>(m, "MD")
        .def(
            py::init([](int canId, mab::Candle* candle) -> auto { return mab::MD(canId, candle); }))
        .def("init", &mab::MD::init, "Initialize the MD device. Returns an error if not connected.")
        .def("blink", &mab::MD::blink, "Blink the built-in LEDs.")
        .def("enable", &mab::MD::enable, "Enable PWM output of the drive.")
        .def("disable", &mab::MD::disable, "Disable PWM output of the drive.")
        .def("reset", &mab::MD::reset, "Reset the driver.")
        .def("clearErrors", &mab::MD::clearErrors, "Clear errors present in the driver.")
        .def("save", &mab::MD::save, "Save configuration data to the memory.")
        .def("zero", &mab::MD::zero, "Zero out the position of the encoder.")
        .def("setCurrentLimit",
             &mab::MD::setCurrentLimit,
             py::arg("currentLimit"),
             "Set the current limit associated with the motor that is driven.")
        .def("setTorqueBandwidth",
             &mab::MD::setTorqueBandwidth,
             py::arg("torqueBandwidth"),
             "Set the torque bandwidth of the MD device.")
        .def("setMotionMode",
             &mab::MD::setMotionMode,
             py::arg("mode"),
             "Set the motion mode of the MD device.")
        .def("setPositionPIDparam",
             &mab::MD::setPositionPIDparam,
             py::arg("kp"),
             py::arg("ki"),
             py::arg("kd"),
             py::arg("integralMax"),
             "Set position controller PID parameters.")
        .def("setVelocityPIDparam",
             &mab::MD::setVelocityPIDparam,
             py::arg("kp"),
             py::arg("ki"),
             py::arg("kd"),
             py::arg("integralMax"),
             "Set velocity controller PID parameters.")
        .def("setImpedanceParams",
             &mab::MD::setImpedanceParams,
             py::arg("kp"),
             py::arg("kd"),
             "Set impedance controller parameters.")
        .def("setMaxTorque",
             &mab::MD::setMaxTorque,
             py::arg("maxTorque"),
             "Set the maximum torque to be output by the controller.")
        .def("setProfileVelocity",
             &mab::MD::setProfileVelocity,
             py::arg("profileVelocity"),
             "Set the target velocity of the profile movement.")
        .def("setProfileAcceleration",
             &mab::MD::setProfileAcceleration,
             py::arg("profileAcceleration"),
             "Set the target profile acceleration when performing profile movement.")
        .def("setTargetPosition",
             &mab::MD::setTargetPosition,
             py::arg("position"),
             "Set the target position of the MD device.")
        .def("setTargetVelocity",
             &mab::MD::setTargetVelocity,
             py::arg("velocity"),
             "Set the target velocity of the MD device.")
        .def("setTargetTorque",
             &mab::MD::setTargetTorque,
             py::arg("torque"),
             "Set the target torque of the MD device.")
        .def("getPosition", &mab::MD::getPosition, "Get the current position of the MD device.")
        .def("getVelocity", &mab::MD::getVelocity, "Get the current velocity of the MD device.")
        .def("getTorque", &mab::MD::getTorque, "Get the current torque of the MD device.")
        .def("getOutputEncoderPosition",
             &mab::MD::getOutputEncoderPosition,
             "Get the output position of the MD device.")
        .def("getOutputEncoderVelocity",
             &mab::MD::getOutputEncoderVelocity,
             "Get the output velocity of the MD device.")
        .def("getTemperature",
             &mab::MD::getTemperature,
             "Get the current temperature of the MD device.");

    // Register read/write methods
    m.def("readRegisterFloat",
          &mab::readReg<float>,
          py::arg("md"),
          py::arg("regName"),
          "Read a register from the MD device.");
    m.def("readRegisterU8",
          &mab::readReg<u8>,
          py::arg("md"),
          py::arg("regName"),
          "Read a register from the MD device.");
    m.def("readRegisterU16",
          &mab::readReg<u16>,
          py::arg("md"),
          py::arg("regName"),
          "Read a register from the MD device.");
    m.def("readRegisterU32",
          &mab::readReg<u32>,
          py::arg("md"),
          py::arg("regName"),
          "Read a register from the MD device.");
    m.def("readRegisterString",
          &mab::readRegString,
          py::arg("md"),
          py::arg("regName"),
          "Read a register from the MD device.",
          py::return_value_policy::copy);
    m.def("readRegisterFloatArray",
          &mab::readRegArray<float>,
          py::arg("md"),
          py::arg("regName"),
          "Read a register from the MD device.",
          py::return_value_policy::copy);
    m.def("readRegisterU8Array",
          &mab::readRegArray<u8>,
          py::arg("md"),
          py::arg("regName"),
          "Read a register from the MD device.",
          py::return_value_policy::copy);

    m.def("writeRegisterFloat",
          &mab::writeReg<float>,
          py::arg("md"),
          py::arg("regName"),
          py::arg("value"),
          "Write a register to the MD device.");
    m.def("writeRegisterU8",
          &mab::writeReg<u8>,
          py::arg("md"),
          py::arg("regName"),
          py::arg("value"),
          "Write a register to the MD device.");
    m.def("writeRegisterU16",
          &mab::writeReg<u16>,
          py::arg("md"),
          py::arg("regName"),
          py::arg("value"),
          "Write a register to the MD device.");
    m.def("writeRegisterU32",
          &mab::writeReg<u32>,
          py::arg("md"),
          py::arg("regName"),
          py::arg("value"),
          "Write a register to the MD device.");
    m.def("writeRegisterString",
          &mab::writeRegString,
          py::arg("md"),
          py::arg("regName"),
          py::arg("value"),
          "Write a register to the MD device.");
    m.def("writeRegisterFloatArray",
          &mab::writeRegArray<float>,
          py::arg("md"),
          py::arg("regName"),
          py::arg("array"),
          "Write a register array to the MD device.");
    m.def("writeRegisterU8Array",
          &mab::writeRegArray<u8>,
          py::arg("md"),
          py::arg("regName"),
          py::arg("array"),
          "Write a register array to the MD device.");

    // Logger
    py::enum_<Logger::Verbosity_E>(m, "Verbosity_E")
        .value("DEFAULT", Logger::Verbosity_E::DEFAULT)
        .value("VERBOSITY_1", Logger::Verbosity_E::VERBOSITY_1)
        .value("VERBOSITY_2", Logger::Verbosity_E::VERBOSITY_2)
        .value("VERBOSITY_3", Logger::Verbosity_E::VERBOSITY_3)
        .value("SILENT", Logger::Verbosity_E::SILENT)
        .export_values();

    m.def(
        "logVerbosity",
        [](Logger::Verbosity_E verbosity) { Logger::g_m_verbosity = verbosity; },
        py::arg("verbosity"));

    // PDS Enums and Types
    py::enum_<mab::PdsModule::error_E>(m, "PDS_Error_t")
        .value("OK", mab::PdsModule::error_E::OK)
        .value("INTERNAL_ERROR", mab::PdsModule::error_E::INTERNAL_ERROR)
        .value("PROTOCOL_ERROR", mab::PdsModule::error_E::PROTOCOL_ERROR)
        .value("COMMUNICATION_ERROR", mab::PdsModule::error_E::COMMUNICATION_ERROR)
        .export_values();

    py::enum_<mab::moduleType_E>(m, "moduleType_E")
        .value("UNDEFINED", mab::moduleType_E::UNDEFINED)
        .value("CONTROL_BOARD", mab::moduleType_E::CONTROL_BOARD)
        .value("BRAKE_RESISTOR", mab::moduleType_E::BRAKE_RESISTOR)
        .value("ISOLATED_CONVERTER", mab::moduleType_E::ISOLATED_CONVERTER)
        .value("POWER_STAGE", mab::moduleType_E::POWER_STAGE)
        .export_values();

    py::enum_<mab::socketIndex_E>(m, "socketIndex_E")
        .value("UNASSIGNED", mab::socketIndex_E::UNASSIGNED)
        .value("SOCKET_1", mab::socketIndex_E::SOCKET_1)
        .value("SOCKET_2", mab::socketIndex_E::SOCKET_2)
        .value("SOCKET_3", mab::socketIndex_E::SOCKET_3)
        .value("SOCKET_4", mab::socketIndex_E::SOCKET_4)
        .value("SOCKET_5", mab::socketIndex_E::SOCKET_5)
        .value("SOCKET_6", mab::socketIndex_E::SOCKET_6)
        .export_values();

    py::enum_<mab::moduleVersion_E>(m, "moduleVersion_E")
        .value("UNKNOWN", mab::moduleVersion_E::UNKNOWN)
        .value("V0_1", mab::moduleVersion_E::V0_1)
        .value("V0_2", mab::moduleVersion_E::V0_2)
        .value("V0_3", mab::moduleVersion_E::V0_3)
        .export_values();

    py::class_<mab::Pds::modulesSet_S>(m, "modulesSet_S")
        .def(py::init<>())
        .def_readwrite("moduleTypeSocket1", &mab::Pds::modulesSet_S::moduleTypeSocket1)
        .def_readwrite("moduleTypeSocket2", &mab::Pds::modulesSet_S::moduleTypeSocket2)
        .def_readwrite("moduleTypeSocket3", &mab::Pds::modulesSet_S::moduleTypeSocket3)
        .def_readwrite("moduleTypeSocket4", &mab::Pds::modulesSet_S::moduleTypeSocket4)
        .def_readwrite("moduleTypeSocket5", &mab::Pds::modulesSet_S::moduleTypeSocket5)
        .def_readwrite("moduleTypeSocket6", &mab::Pds::modulesSet_S::moduleTypeSocket6);

    py::class_<mab::version_ut>(m, "version_ut")
        .def(py::init<>())
        .def_property(
            "minor",
            [](mab::version_ut& self) -> u8 { return self.s.minor; },
            [](mab::version_ut& self, u8 value) { self.s.minor = value; })
        .def_property(
            "major",
            [](mab::version_ut& self) -> u8 { return self.s.major; },
            [](mab::version_ut& self, u8 value) { self.s.major = value; })
        .def_property(
            "revision",
            [](mab::version_ut& self) -> u8 { return self.s.revision; },
            [](mab::version_ut& self, u8 value) { self.s.revision = value; })
        .def_property(
            "tag",
            [](mab::version_ut& self) -> char { return self.s.tag; },
            [](mab::version_ut& self, char value) { self.s.tag = value; });

    py::class_<mab::pdsFwMetadata_S>(m, "pdsFwMetadata_S")
        .def(py::init<>())
        .def_readwrite("metadataStructVersion", &mab::pdsFwMetadata_S::metadataStructVersion)
        .def_readwrite("version", &mab::pdsFwMetadata_S::version)
        .def_property(
            "gitHash",
            [](const mab::pdsFwMetadata_S& s) { return std::string(s.gitHash, 8); },
            [](mab::pdsFwMetadata_S& s, const std::string& value)
            { std::strncpy(s.gitHash, value.c_str(), 8); });

    py::class_<mab::controlBoardStatus_S>(m, "controlBoardStatus_S")
        .def(py::init<>())
        .def_readwrite("ENABLED", &mab::controlBoardStatus_S::ENABLED)
        .def_readwrite("OVER_TEMPERATURE", &mab::controlBoardStatus_S::OVER_TEMPERATURE)
        .def_readwrite("OVER_CURRENT", &mab::controlBoardStatus_S::OVER_CURRENT)
        .def_readwrite("STO_1", &mab::controlBoardStatus_S::STO_1)
        .def_readwrite("STO_2", &mab::controlBoardStatus_S::STO_2)
        .def_readwrite("FDCAN_TIMEOUT", &mab::controlBoardStatus_S::FDCAN_TIMEOUT)
        .def_readwrite("SUBMODULE_1_ERROR", &mab::controlBoardStatus_S::SUBMODULE_1_ERROR)
        .def_readwrite("SUBMODULE_2_ERROR", &mab::controlBoardStatus_S::SUBMODULE_2_ERROR)
        .def_readwrite("SUBMODULE_3_ERROR", &mab::controlBoardStatus_S::SUBMODULE_3_ERROR)
        .def_readwrite("SUBMODULE_4_ERROR", &mab::controlBoardStatus_S::SUBMODULE_4_ERROR)
        .def_readwrite("SUBMODULE_5_ERROR", &mab::controlBoardStatus_S::SUBMODULE_5_ERROR)
        .def_readwrite("SUBMODULE_6_ERROR", &mab::controlBoardStatus_S::SUBMODULE_6_ERROR)
        .def_readwrite("CHARGER_DETECTED", &mab::controlBoardStatus_S::CHARGER_DETECTED)
        .def_readwrite("SHUTDOWN_SCHEDULED", &mab::controlBoardStatus_S::SHUTDOWN_SCHEDULED);

    py::class_<mab::powerStageStatus_S>(m, "powerStageStatus_S")
        .def(py::init<>())
        .def_readwrite("ENABLED", &mab::powerStageStatus_S::ENABLED)
        .def_readwrite("OVER_TEMPERATURE", &mab::powerStageStatus_S::OVER_TEMPERATURE)
        .def_readwrite("OVER_CURRENT", &mab::powerStageStatus_S::OVER_CURRENT);

    py::class_<mab::brakeResistorStatus_S>(m, "brakeResistorStatus_S")
        .def(py::init<>())
        .def_readwrite("ENABLED", &mab::brakeResistorStatus_S::ENABLED)
        .def_readwrite("OVER_TEMPERATURE", &mab::brakeResistorStatus_S::OVER_TEMPERATURE);

    py::class_<mab::isolatedConverterStatus_S>(m, "isolatedConverterStatus_S")
        .def(py::init<>())
        .def_readwrite("ENABLED", &mab::isolatedConverterStatus_S::ENABLED)
        .def_readwrite("OVER_TEMPERATURE", &mab::isolatedConverterStatus_S::OVER_TEMPERATURE)
        .def_readwrite("OVER_CURRENT", &mab::isolatedConverterStatus_S::OVER_CURRENT);

    // PDS Module Classes
    py::class_<mab::BrakeResistor, py::smart_holder>(m, "BrakeResistor")
        .def("printModuleInfo", &mab::BrakeResistor::printModuleInfo)
        .def("enable", &mab::BrakeResistor::enable)
        .def("disable", &mab::BrakeResistor::disable)
        .def("getStatus", &mab::brakeResistorGetStatusWrapper)
        .def("clearStatus", &mab::BrakeResistor::clearStatus)
        .def("getEnabled", &mab::brakeResistorGetEnabledWrapper)
        .def("getTemperature", &mab::brakeResistorGetTemperatureWrapper)
        .def("setTemperatureLimit",
             &mab::BrakeResistor::setTemperatureLimit,
             py::arg("temperatureLimit"))
        .def("getTemperatureLimit", &mab::brakeResistorGetTemperatureLimitWrapper);

    py::class_<mab::PowerStage, py::smart_holder>(m, "PowerStage")
        .def("printModuleInfo", &mab::PowerStage::printModuleInfo)
        .def("enable", &mab::PowerStage::enable)
        .def("disable", &mab::PowerStage::disable)
        .def("getStatus", &mab::powerStageGetStatusWrapper)
        .def("clearStatus", &mab::PowerStage::clearStatus)
        .def("getEnabled", &mab::powerStageGetEnabledWrapper)
        .def("bindBrakeResistor",
             &mab::PowerStage::bindBrakeResistor,
             py::arg("brakeResistorSocketIndex"))
        .def("getBindBrakeResistor", &mab::powerStageGetBindBrakeResistorWrapper)
        .def("setBrakeResistorTriggerVoltage",
             &mab::PowerStage::setBrakeResistorTriggerVoltage,
             py::arg("brTriggerVoltage"))
        .def("getBrakeResistorTriggerVoltage",
             &mab::powerStageGetBrakeResistorTriggerVoltageWrapper)
        .def("getOutputVoltage", &mab::powerStageGetOutputVoltageWrapper)
        .def("setAutostart", &mab::PowerStage::setAutostart, py::arg("autoStart"))
        .def("getAutostart", &mab::powerStageGetAutostartWrapper)
        .def("getLoadCurrent", &mab::powerStageGetLoadCurrentWrapper)
        .def("getPower", &mab::powerStageGetPowerWrapper)
        .def("getEnergy", &mab::powerStageGetEnergyWrapper)
        .def("getTemperature", &mab::powerStageGetTemperatureWrapper)
        .def("setOcdLevel", &mab::PowerStage::setOcdLevel, py::arg("ocdLevel"))
        .def("getOcdLevel", &mab::powerStageGetOcdLevelWrapper)
        .def("setOcdDelay", &mab::PowerStage::setOcdDelay, py::arg("ocdDelay"))
        .def("getOcdDelay", &mab::powerStageGetOcdDelayWrapper)
        .def("setTemperatureLimit",
             &mab::PowerStage::setTemperatureLimit,
             py::arg("temperatureLimit"))
        .def("getTemperatureLimit", &mab::powerStageGetTemperatureLimitWrapper);

    py::class_<mab::IsolatedConv, py::smart_holder>(m, "IsolatedConv")
        .def("printModuleInfo", &mab::IsolatedConv::printModuleInfo)
        .def("enable", &mab::IsolatedConv::enable)
        .def("disable", &mab::IsolatedConv::disable)
        .def("getStatus", &mab::isolatedConvGetStatusWrapper)
        .def("clearStatus", &mab::IsolatedConv::clearStatus)
        .def("getEnabled", &mab::isolatedConvGetEnabledWrapper)
        .def("getOutputVoltage", &mab::isolatedConvGetOutputVoltageWrapper)
        .def("getLoadCurrent", &mab::isolatedConvGetLoadCurrentWrapper)
        .def("getPower", &mab::isolatedConvGetPowerWrapper)
        .def("getEnergy", &mab::isolatedConvGetEnergyWrapper)
        .def("getTemperature", &mab::isolatedConvGetTemperatureWrapper)
        .def("setOcdLevel", &mab::IsolatedConv::setOcdLevel, py::arg("ocdLevel"))
        .def("getOcdLevel", &mab::isolatedConvGetOcdLevelWrapper)
        .def("setOcdDelay", &mab::IsolatedConv::setOcdDelay, py::arg("ocdDelay"))
        .def("getOcdDelay", &mab::isolatedConvGetOcdDelayWrapper)
        .def("setTemperatureLimit",
             &mab::IsolatedConv::setTemperatureLimit,
             py::arg("temperatureLimit"))
        .def("getTemperatureLimit", &mab::isolatedConvGetTemperatureLimitWrapper);

    // PDS Main Class
    py::class_<mab::Pds>(m, "Pds")
        .def(py::init([](int canId, mab::Candle* candle) -> auto
                      { return mab::Pds(canId, candle); }))
        .def("init", py::overload_cast<>(&mab::Pds::init), "Initialize the PDS device")
        .def("initWithNewId",
             py::overload_cast<u16>(&mab::Pds::init),
             "Initialize the PDS device with the provided ID")
        .def("printModuleInfo",
             &mab::Pds::printModuleInfo,
             "Print information about connected modules")
        .def("getFwMetadata", &mab::pdsFwMetadataWrapper, "Get firmware metadata")
        .def("getModules", &mab::Pds::getModules, "Get information about connected modules")
        .def("verifyModuleSocket",
             &mab::Pds::verifyModuleSocket,
             py::arg("type"),
             py::arg("socket"),
             "Verify if a module type is at a specific socket")
        .def("attachBrakeResistor",
             &mab::Pds::attachBrakeResistor,
             py::arg("socket"),

             "Attach a brake resistor module at the specified socket")
        .def("attachPowerStage",
             &mab::Pds::attachPowerStage,
             py::arg("socket"),
             "Attach a power stage module at the specified socket")
        .def("attachIsolatedConverter",
             &mab::Pds::attachIsolatedConverter,
             py::arg("socket"),
             "Attach an isolated converter module at the specified socket")
        .def("getStatus", &mab::pdsGetStatusWrapper, "Get control board status")
        .def("clearStatus", &mab::Pds::clearStatus, "Clear control board status")
        .def("clearErrors", &mab::Pds::clearErrors, "Clear all errors")
        .def("getCanId", &mab::Pds::getCanId, "Get CAN ID")
        .def("setCanId", &mab::Pds::setCanId, py::arg("canId"), "Set CAN ID")
        .def("getCanDatarate", &mab::Pds::getCanDatarate, "Get CAN datarate")
        .def(
            "setCanDatarate", &mab::Pds::setCanDatarate, py::arg("canDatarate"), "Set CAN datarate")
        .def("getBusVoltage", &mab::pdsGetBusVoltageWrapper, "Get bus voltage")
        .def("getTemperature", &mab::pdsGetTemperatureWrapper)
        .def("getTemperatureLimit", &mab::pdsGetTemperatureLimitWrapper, "Get temperature limit")
        .def("setTemperatureLimit",
             &mab::Pds::setTemperatureLimit,
             py::arg("temperatureLimit"),
             "Set temperature limit")
        .def("getShutdownTime", &mab::pdsGetShutdownTimeWrapper, "Get shutdown time")
        .def("setShutdownTime",
             &mab::Pds::setShutdownTime,
             py::arg("shutdownTime"),
             "Set shutdown time")
        .def("getBatteryVoltageLevels",
             &mab::pdsGetBatteryVoltageLevelsWrapper,
             "Get battery voltage levels")
        .def("setBatteryVoltageLevels",
             &mab::Pds::setBatteryVoltageLevels,
             py::arg("batteryLvl1"),
             py::arg("batteryLvl2"),
             "Set battery voltage levels")
        .def("bindBrakeResistor",
             &mab::Pds::bindBrakeResistor,
             py::arg("brakeResistorSocketIndex"),
             "Bind brake resistor to socket")
        .def("getBindBrakeResistor",
             &mab::pdsGetBindBrakeResistorWrapper,
             "Get bound brake resistor socket")
        .def("setBrakeResistorTriggerVoltage",
             &mab::Pds::setBrakeResistorTriggerVoltage,
             py::arg("brTriggerVoltage"),
             "Set brake resistor trigger voltage")
        .def("getBrakeResistorTriggerVoltage",
             &mab::pdsGetBrakeResistorTriggerVoltageWrapper,
             "Get brake resistor trigger voltage")
        .def("shutdown", &mab::Pds::shutdown, "Shutdown the PDS device")
        .def("reboot", &mab::Pds::reboot, "Reboot the PDS device")
        .def("saveConfig", &mab::Pds::saveConfig, "Save configuration to memory")
        .def_static("moduleTypeToString",
                    &mab::Pds::moduleTypeToString,
                    py::arg("type"),
                    "Convert module type to string")
        .def_static("discoverPDS",
                    &mab::Pds::discoverPDS,
                    py::arg("candle"),
                    "Discover PDS devices on the CAN bus");
}
