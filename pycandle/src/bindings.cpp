#include <pybind11/cast.h>
#include <pybind11/detail/common.h>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <cstdint>
#include <vector>

#include "I_communication_interface.hpp"
#include "candle_types.hpp"
#include "candle_v2.hpp"
#include "register.hpp"

namespace py = pybind11;

namespace mab
{
    CandleV2* pyAttachCandle(const CANdleBaudrate_E baudrate, candleTypes::busTypes_t busType)
    {
        return attachCandle(baudrate, busType);
    }

    // class CandlePython : public Candle
    // {
    //   public:
    //     CandlePython(CANdleBaudrate_E  canBaudrate,
    //                  bool              printVerbose = true,
    //                  mab::BusType_E    busType      = BusType_E::USB,
    //                  const std::string device       = "")
    //         : Candle(canBaudrate, printVerbose, busType, device)
    //     {
    //     }
    //     CandlePython(CANdleBaudrate_E canBaudrate, bool printVerbose, std::shared_ptr<Bus> bus)
    //         : Candle(canBaudrate, printVerbose, bus)
    //     {
    //     }
    //     virtual ~CandlePython() = default;

    //     float readMd80Register_(uint16_t canId, Md80Reg_E regId, float regValue)
    //     {
    //         (void)regValue;
    //         md80Register->read(canId, regId, regValue);
    //         return regValue;
    //     }

    //     int64_t readMd80Register_(uint16_t canId, Md80Reg_E regId, int64_t regValue)
    //     {
    //         (void)regValue;
    //         int64_t regValue_i64 = 0;

    //         md80Register->read(canId, regId, regValue_i64);

    //         // if (md80Register->getType(regId) == Register::type::I8)
    //         // 	return *(uint8_t*)&regValue_i64;
    //         // else if (md80Register->getType(regId) == Register::type::I16)
    //         // 	return *(uint16_t*)&regValue_i64;
    //         // else if (md80Register->getType(regId) == Register::type::I32)
    //         // 	return *(uint32_t*)&regValue_i64;
    //         // else
    //         return regValue_i64;
    //     }

    //     std::string readMd80Register_(uint16_t canId, Md80Reg_E regId, char* regValue)
    //     {
    //         (void)regValue;
    //         char regValue_[64] = {0};
    //         md80Register->read(canId, regId, regValue_);
    //         return std::string(regValue_);
    //     }

    //     bool writeMd80Register_(uint16_t canId, Md80Reg_E regId, float regValue)
    //     {
    //         return md80Register->write(canId, regId, regValue);
    //     }

    //     bool writeMd80Register_(uint16_t canId, Md80Reg_E regId, uint32_t regValue)
    //     {
    //         return md80Register->write(canId, regId, regValue);
    //     }

    //     bool writeMd80Register_(uint16_t canId, Md80Reg_E regId, int32_t regValue)
    //     {
    //         return md80Register->write(canId, regId, regValue);
    //     }

    //     bool writeMd80Register_(uint16_t canId, Md80Reg_E regId, std::string regValue)
    //     {
    //         char regValue_[64] = {0};
    //         memcpy(regValue_, regValue.c_str(), regValue.length());
    //         return md80Register->write(canId, regId, regValue_);
    //     }
    // };
}  // namespace mab

PYBIND11_MODULE(pyCandle, m)
{
    m.doc() = "pyCandle module for interfacing with md80 drives using Python";

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
          py::return_value_policy::reference,
          "Attach a CANdle device to the system.");
    // py::class_<mab::CandleV2>(m, "CandleV2").def();
}
