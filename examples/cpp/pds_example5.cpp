/*
    MAB Robotics

    Power Distribution System Example 5

    Read / write multiple properties methods

    This example shows the usage of Power Stage module along with the Brake resistor
    It assumes that at least one Power Stage & Brake Resistor module are connected
    to the control board.

    This examples differ from example 4 the way how it fetches the data from PDS.
    Instead of calling the getOutputVoltage, getCurrent, getPower methods where
    each of them performs an end to end data exchange, in this example we are creating
    a custom messages to get all properties within a single command/response cycle.

    !!! Warning
    Note that this technique is much faster but moves the responsibility of error handling
    onto the user so use with care...

*/

#include "candle.hpp"
#include "pds.hpp"
#include "pds_protocol.hpp"
#include "time.h"

using namespace mab;

/*
Regardless that the software is detecting what modules are
connected to which sockets, user should now it explicitly too.
If user is using more then one module of the same type, the particular sockets
should be known to properly assign the module pointers when using the "<module>Attach" method.
*/
constexpr socketIndex_E POWER_STAGE_SOCKET_INDEX    = socketIndex_E::SOCKET_1;
constexpr socketIndex_E BRAKE_RESISTOR_SOCKET_INDEX = socketIndex_E::SOCKET_2;

constexpr int PDS_CAN_ID = 100u;

int main()
{
    Logger log;
    log.m_tag = "PDS Example";

    // Those variables will store raw binary representation of the corresponding data.
    // vbusVoltage is omitted because it does not require further conversion
    u32 loadCurrentRawData = 0;
    u32 loadPowerRawData   = 0;

    u32 vbusVoltage = 0;
    s32 loadCurrent = 0;
    s32 loadPower   = 0;

    float vBusVoltage_f = 0.0f;
    float loadCurrent_f = 0.0f;
    float loadPower_f   = 0.0f;

    std::shared_ptr<Candle> pCandle = std::make_shared<Candle>(mab::CAN_BAUD_1M, true);
    Pds                     pds(PDS_CAN_ID, pCandle);

    Pds::modules_S pdsModules = {0};

    std::unique_ptr<PowerStage>    p_powerStage    = nullptr;
    std::unique_ptr<BrakeResistor> p_brakeResistor = nullptr;

    std::vector<u8> serializedMessage;
    u8              responseBuffer[64] = {0};

    pds.getModules(pdsModules);

    log.info("PDS have the following numbers of connected modules:");
    log.info("\t PS    :: [ %u ]", pdsModules.powerStage);
    log.info("\t IC 12 :: [ %u ]", pdsModules.isolatedConv12V);
    log.info("\t IC 5  :: [ %u ]", pdsModules.isolatedConverter5V);
    log.info("\t BR    :: [ %u ]", pdsModules.brakeResistor);

    p_powerStage = pds.attachPowerStage(POWER_STAGE_SOCKET_INDEX);
    if (p_powerStage == nullptr)
    {
        log.error("Unable to attach power stage module!");
        exit(EXIT_FAILURE);
    }

    log.success("Power stage at socket [ %u ] attached!", p_powerStage->getSocketIndex());

    p_brakeResistor = pds.attachBrakeResistor(BRAKE_RESISTOR_SOCKET_INDEX);
    if (p_brakeResistor == nullptr)
    {
        log.error("Unable to attach brake resistor module!");
        exit(EXIT_FAILURE);
    }

    log.success("Brake resistor at socket [ %u ] attached!", p_brakeResistor->getSocketIndex());

    // Creating a message addressed to the Power Stage on its socket index
    PropertySetMessage psConfigMessage(moduleType_E::POWER_STAGE, p_powerStage->getSocketIndex());

    // Adding properties that are going to be set on the target Power Stage.

    // Property used to bind Brake resistor with power stage
    psConfigMessage.addProperty(PowerStage::properties_E::BR_SOCKET_INDEX,
                                BRAKE_RESISTOR_SOCKET_INDEX);

    // Property used to set the trigger voltage for the brake resistor
    psConfigMessage.addProperty(PowerStage::properties_E::BR_TRIGGER_VOLTAGE, 28000u);

    serializedMessage = psConfigMessage.serialize();

    if (!pCandle->sendGenericFDCanFrame(PDS_CAN_ID,
                                        serializedMessage.size(),
                                        reinterpret_cast<const char*>(serializedMessage.data()),
                                        reinterpret_cast<char*>(responseBuffer)))
    {
        log.error("Generic CAN Frame error");
        exit(EXIT_FAILURE);
    }

    if (psConfigMessage.parseResponse(responseBuffer, sizeof(responseBuffer)) !=
        PdsMessage::error_E::OK)
    {
        log.error("Failed to parse response");
        exit(EXIT_FAILURE);
    }

    p_powerStage->enable();

    // Prepare a message to fetch some data from power stage module
    PropertyGetMessage psDataGetMessage(moduleType_E::POWER_STAGE, p_powerStage->getSocketIndex());

    psDataGetMessage.addProperty(PowerStage::properties_E::BUS_VOLTAGE);
    psDataGetMessage.addProperty(PowerStage::properties_E::LOAD_CURRENT);
    psDataGetMessage.addProperty(PowerStage::properties_E::LOAD_POWER);

    /*
        This message could be serialized once and then reused
        to fetch the same set of data multiple times.
    */
    serializedMessage = psDataGetMessage.serialize();

    while (1)
    {
        usleep(100000);

        if (!pCandle->sendGenericFDCanFrame(PDS_CAN_ID,
                                            serializedMessage.size(),
                                            reinterpret_cast<const char*>(serializedMessage.data()),
                                            reinterpret_cast<char*>(responseBuffer)))
        {
            log.error("Generic CAN Frame error");
            exit(EXIT_FAILURE);
        }

        if (psDataGetMessage.parseResponse(responseBuffer, sizeof(responseBuffer)) !=
            PdsMessage::error_E::OK)
        {
            log.error("Failed to parse response");
            exit(EXIT_FAILURE);
        }

        /*
            After message is successfully parsed, previously defined set of properties should now
            have its values assigned. We can obtain them by using the getProperty method.
            But due to the generic binary form in which they are stored we
            have to convert each of them to its proper values later.
        */

        if (psDataGetMessage.getProperty(PowerStage::properties_E::BUS_VOLTAGE, &vbusVoltage) !=
            PdsMessage::error_E::OK)
        {
            log.error("Failed to parse property : BUS_VOLTAGE");
        }

        if (psDataGetMessage.getProperty(PowerStage::properties_E::LOAD_CURRENT,
                                         &loadCurrentRawData) != PdsMessage::error_E::OK)
        {
            log.error("Failed to parse property : LOAD_CURRENT");
        }

        loadCurrent = *reinterpret_cast<int32_t*>(&loadCurrentRawData);

        if (psDataGetMessage.getProperty(PowerStage::properties_E::LOAD_POWER, &loadPowerRawData) !=
            PdsMessage::error_E::OK)
        {
            log.error("Failed to parse property : LOAD_POWER");
        }

        loadPower = *reinterpret_cast<int32_t*>(&loadPowerRawData);

        vBusVoltage_f = static_cast<float>(vbusVoltage) / 1000.0f;
        loadCurrent_f = static_cast<float>(loadCurrent) / 1000.0f;
        loadPower_f   = static_cast<float>(loadPower) / 1000.0f;

        log.info("V: [ %.2f V ] ; I : [ %.2f A ] ; P : [ %.2f W ]",
                 vBusVoltage_f,
                 loadCurrent_f,
                 loadPower_f);
    }

    return EXIT_SUCCESS;
}