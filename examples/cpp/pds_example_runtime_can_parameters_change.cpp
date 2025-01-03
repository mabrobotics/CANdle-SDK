/*
    MAB Robotics

    Power Distribution System Example

    Changing FDCan parameters in runtime
*/
#include "candle.hpp"
#include "pds.hpp"

using namespace mab;

constexpr u16 PDS_CAN_ID = 100;

int main()
{
    Logger _log;
    _log.m_tag = "PDS Example";

    PdsModule::error_E result = PdsModule::error_E::OK;

    Candle candle(mab::CAN_BAUD_1M, true);
    Pds    pds(PDS_CAN_ID, candle);
    u32    pdsBusVoltage = 0;

    _log.info("Changing PDS Device CAN ID to 103...");
    result = pds.setCanId(103);
    if (PdsModule::error_E::OK != result)
    {
        _log.error("Changing PDS Device CAN ID failed!");
        return EXIT_FAILURE;
    }
    _log.success("PDS Device CAN ID change OK");

    _log.info("Reading bus voltage on 1M CAN Baud...");
    result = pds.getBusVoltage(pdsBusVoltage);
    if (PdsModule::error_E::OK != result)
    {
        _log.error("Reading bus voltage failed!");
        return EXIT_FAILURE;
    }
    _log.success("Bus voltage: %0.2f", pdsBusVoltage / 1000.0f);

    _log.info("Changing CAN Baud on PDS device to 5M...");
    result = pds.setCanBaudrate(canBaudrate_E::BAUD_5M);
    if (PdsModule::error_E::OK != result)
    {
        _log.error("FDCan baudrate switching failed!");
        return EXIT_FAILURE;
    }
    _log.success("PDS CAN Baud change OK");

    _log.info("Changing CAN Baud on CANdle device to 5M...");
    if (!candle.configCandleBaudrate(CANdleBaudrate_E::CAN_BAUD_5M))
    {
        _log.error("FDCan baudrate switching failed!");
        return EXIT_FAILURE;
    }
    _log.success("CANdle CAN Baud change OK");

    usleep(10000);

    _log.info("Reading bus voltage on 5M CAN Baud...");
    result = pds.getBusVoltage(pdsBusVoltage);
    if (PdsModule::error_E::OK != result)
    {
        _log.error("Reading bus voltage failed!");
        return EXIT_FAILURE;
    }
    _log.success("Bus voltage: %0.2f", pdsBusVoltage / 1000.0f);

    return EXIT_SUCCESS;
}