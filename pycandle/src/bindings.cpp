#include <pybind11/cast.h>
#include <pybind11/detail/common.h>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <cstdint>
#include <memory>
#include <vector>

#include "candle_types.hpp"
#include "candle.hpp"
#include "MD.hpp"
#include "logger.hpp"

namespace py = pybind11;

namespace mab
{
    Candle* pyAttachCandle(const CANdleBaudrate_E baudrate, candleTypes::busTypes_t busType)
    {
        return attachCandle(baudrate, busType);
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
                    md.readRegisters(reg);
                    value = reg.value;
                }
            }
        };

        mdRegisters.forEachRegister(mdRegisters, getReg);
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
                    md.readRegisters(reg);
                    value = std::string(reg.value);
                }
            }
        };

        mdRegisters.forEachRegister(mdRegisters, getReg);
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
                    md.writeRegisters(reg);
                }
            }
        };

        mdRegisters.forEachRegister(mdRegisters, getReg);
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
                    md.writeRegisters(reg);
                }
            }
        };

        mdRegisters.forEachRegister(mdRegisters, getReg);
        if (!found)
        {
            log.error("Wrong name or type!");
            err = MD::Error_t::REQUEST_INVALID;
        }
        return err;
    }

}  // namespace mab

PYBIND11_MODULE(pyCandle, m)
{
    m.doc() = "pyCandle module for interfacing with MD drives using Python";

    // CANdle class

    py::enum_<mab::CANdleBaudrate_E>(m, "CANdleBaudrate_E")
        .value("CAN_BAUD_1M", mab::CAN_BAUD_1M)
        .value("CAN_BAUD_2M", mab::CAN_BAUD_2M)
        .value("CAN_BAUD_5M", mab::CAN_BAUD_5M)
        .value("CAN_BAUD_8M", mab::CAN_BAUD_8M)
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
          py::arg("baudrate"),
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
}
