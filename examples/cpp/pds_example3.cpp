/*
    MAB Robotics

    Power Distribution System Example 3

    This example shows the usage of Power Stage module.
    It assumes that at least one Power Stage module is connected
    to the control board.

*/

#include "candle.hpp"
#include "pds.hpp"
#include "time.h"

#include <termios.h>  // For termios functions
#include <fcntl.h>    // For non-blocking input

using namespace mab;

Logger _log;

std::unique_ptr<PowerStage>    p_powerStage    = nullptr;
std::unique_ptr<BrakeResistor> p_brakeResistor = nullptr;

// Function to make terminal input non-blocking
void setNonBlockingInput(bool enable)
{
    struct termios t;
    tcgetattr(STDIN_FILENO, &t);  // Get the current terminal attributes
    if (enable)
    {
        t.c_lflag &= ~(ICANON | ECHO);             // Disable canonical mode and echo
        tcsetattr(STDIN_FILENO, TCSANOW, &t);      // Set the new attributes immediately
        fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);  // Make stdin non-blocking
    }
    else
    {
        t.c_lflag |= (ICANON | ECHO);          // Restore canonical mode and echo
        tcsetattr(STDIN_FILENO, TCSANOW, &t);  // Restore the terminal settings
        fcntl(STDIN_FILENO,
              F_SETFL,
              fcntl(STDIN_FILENO, F_GETFL) & ~O_NONBLOCK);  // Make stdin blocking
    }
}

/*
Regardless that the software is detecting what modules are
connected to which sockets, user should now it explicitly too.
If user is using more then one module of the same type, the particular sockets
should be known to properly assign the module pointers when using the "<module>Attach" method.
*/
constexpr socketIndex_E POWER_STAGE_SOCKET_INDEX    = socketIndex_E::SOCKET_2;
constexpr socketIndex_E BRAKE_RESISTOR_SOCKET_INDEX = socketIndex_E::SOCKET_3;

struct powerStageData_S
{
    PowerStage::status_S powerStageStatus = {0};

    u32   vbusVoltage = 0;
    s32   loadCurrent = 0;
    s32   loadPower   = 0;
    u32   ocdLevel    = 10000;
    u32   ocdDelay    = 1000;
    float temperature = 0.0f;

    float vBusVoltage_f = 0.0f;
    float loadCurrent_f = 0.0f;
    float loadPower_f   = 0.0f;
    float ocdLevel_f    = 0.0f;
    float ocdDelay_f    = 0.0f;
};

struct brakeResistorData_S
{
};

static PdsModule::error_E getPowerStageData(powerStageData_S& psData)
{
    PdsModule::error_E result = PdsModule::error_E::OK;

    result = p_powerStage->getStatus(psData.powerStageStatus);
    if (result != PdsModule::error_E::OK)
    {
        _log.error("Reading status property error [ %u ]", static_cast<u8>(result));
        return result;
    }

    result = p_powerStage->getOutputVoltage(psData.vbusVoltage);
    if (result != PdsModule::error_E::OK)
    {
        _log.error("Reading VBus voltage error [ %u ]", static_cast<u8>(result));
        return result;
    }

    result = p_powerStage->getLoadCurrent(psData.loadCurrent);
    if (result != PdsModule::error_E::OK)
    {
        _log.error("Reading load current error [ %u ]", static_cast<u8>(result));
        return result;
    }

    result = p_powerStage->getPower(psData.loadPower);
    if (result != PdsModule::error_E::OK)
    {
        _log.error("Reading load power error [ %u ]", static_cast<u8>(result));
        return result;
    }

    result = p_powerStage->getTemperature(psData.temperature);
    if (result != PdsModule::error_E::OK)
    {
        _log.error("Reading temperature error [ %u ]", static_cast<u8>(result));
        return result;
    }

    result = p_powerStage->getOcdLevel(psData.ocdLevel);
    if (result != PdsModule::error_E::OK)
    {
        _log.error("Reading OCD Level error [ %u ]", static_cast<u8>(result));
        return result;
    }

    result = p_powerStage->getOcdDelay(psData.ocdDelay);
    if (result != PdsModule::error_E::OK)
    {
        _log.error("Reading OCD Delay error [ %u ]", static_cast<u8>(result));
        return result;
    }

    psData.vBusVoltage_f = static_cast<float>(psData.vbusVoltage) / 1000.0f;
    psData.loadCurrent_f = static_cast<float>(psData.loadCurrent) / 1000.0f;
    psData.loadPower_f   = static_cast<float>(psData.loadPower) / 1000.0f;
    psData.ocdLevel_f    = static_cast<float>(psData.ocdLevel) / 1000.0f;
    psData.ocdDelay_f    = static_cast<float>(psData.ocdDelay) / 1000.0f;

    return result;
}

int main()
{
    setNonBlockingInput(true);

    powerStageData_S psData;

    _log.m_tag = "PDS Example";

    std::shared_ptr<Candle> pCandle = std::make_shared<Candle>(mab::CAN_BAUD_1M, true);
    Pds                     pds(100, pCandle);

    Pds::modules_S pdsModules = {0};

    PdsModule::error_E result = PdsModule::error_E::OK;

    pds.getModules(pdsModules);

    _log.info("PDS have the following numbers of connected modules:");
    _log.info("\t PS    :: [ %u ]", pdsModules.powerStage);
    _log.info("\t IC 12 :: [ %u ]", pdsModules.isolatedConv12V);
    _log.info("\t IC 5  :: [ %u ]", pdsModules.isolatedConverter5V);
    _log.info("\t BR    :: [ %u ]", pdsModules.brakeResistor);

    p_powerStage = pds.attachPowerStage(POWER_STAGE_SOCKET_INDEX);
    if (p_powerStage == nullptr)
    {
        _log.error("Unable to attach power stage module!");
        exit(EXIT_FAILURE);
    }

    _log.success("Power stage at socket [ %u ] attached!", p_powerStage->getSocketIndex());

    p_brakeResistor = pds.attachBrakeResistor(BRAKE_RESISTOR_SOCKET_INDEX);
    if (p_brakeResistor == nullptr)
    {
        _log.error("Unable to attach brake resistor module!");
        exit(EXIT_FAILURE);
    }

    _log.success("Brake resistor at socket [ %u ] attached!", p_brakeResistor->getSocketIndex());

    result = p_powerStage->disable();
    if (result != PdsModule::error_E::OK)
    {
        _log.error("Power stage enable error [ %u ]", static_cast<u8>(result));
        exit(EXIT_FAILURE);
    }

    result = p_powerStage->setOcdLevel(psData.ocdLevel);
    if (result != PdsModule::error_E::OK)
    {
        _log.error("Unable to set OCD Level [ %u ]", static_cast<u8>(result));
        exit(EXIT_FAILURE);
    }

    result = p_powerStage->setOcdDelay(psData.ocdDelay);
    if (result != PdsModule::error_E::OK)
    {
        _log.error("Unable to set OCD Delay [ %u ]", static_cast<u8>(result));
        exit(EXIT_FAILURE);
    }

    result = p_powerStage->bindBrakeResistor(p_brakeResistor->getSocketIndex());
    if (result != PdsModule::error_E::OK)
    {
        _log.error("Unable to bind Brake Resistor [ %u ]", static_cast<u8>(result));
        exit(EXIT_FAILURE);
    }

    _log.success("Power stage[ %u ] binding with Brake Resistor[ %u ] OK",
                 p_powerStage->getSocketIndex(),
                 p_brakeResistor->getSocketIndex());

    while (1)
    {
        usleep(100000);

        char ch;
        if (read(STDIN_FILENO, &ch, 1) == 1)
        {
            if (ch == ' ')
            {  // Check if the pressed key is the space key
                if (psData.powerStageStatus.ENABLED)
                {
                    result = p_powerStage->disable();
                    if (result != PdsModule::error_E::OK)
                        _log.error("Power stage disable error [ %u ]", static_cast<u8>(result));
                }
                else
                {
                    result = p_powerStage->enable();
                    if (result != PdsModule::error_E::OK)
                        _log.error("Power stage enable error [ %u ]", static_cast<u8>(result));
                }
            }
            else if (ch == 27)
            {
                _log.info("Exiting the program...");
                break;
            }
            else if (ch == 'q')
            {
                psData.ocdLevel += 1000;
                result = p_powerStage->setOcdLevel(psData.ocdLevel);
                if (result != PdsModule::error_E::OK)
                {
                    _log.error("Unable to set OCD Level [ %u ]", static_cast<u8>(result));
                    // exit(EXIT_FAILURE);
                }
            }
            else if (ch == 'a')
            {
                if (psData.ocdLevel >= 1000)
                    psData.ocdLevel -= 1000;
                result = p_powerStage->setOcdLevel(psData.ocdLevel);
                if (result != PdsModule::error_E::OK)
                {
                    _log.error("Unable to set OCD Level [ %u ]", static_cast<u8>(result));
                    // exit(EXIT_FAILURE);
                }
            }
            else if (ch == 'w')
            {
                psData.ocdDelay += 200;
                result = p_powerStage->setOcdDelay(psData.ocdDelay);
                if (result != PdsModule::error_E::OK)
                {
                    _log.error("Unable to set OCD Delay [ %u ]", static_cast<u8>(result));
                    // exit(EXIT_FAILURE);
                }
            }

            else if (ch == 's')
            {
                if (psData.ocdDelay >= 200)
                    psData.ocdDelay -= 200;

                result = p_powerStage->setOcdDelay(psData.ocdDelay);
                if (result != PdsModule::error_E::OK)
                {
                    _log.error("Unable to set OCD Delay [ %u ]", static_cast<u8>(result));
                    // exit(EXIT_FAILURE);
                }
            }

            else if (ch == 'b')
            {
                result = p_brakeResistor->enable();
                if (result != PdsModule::error_E::OK)
                    _log.error("Brake resistor enable error [ %u ]", static_cast<u8>(result));
            }

            else if (ch == 'n')
            {
                result = p_brakeResistor->disable();
                if (result != PdsModule::error_E::OK)
                    _log.error("Brake resistor enable error [ %u ]", static_cast<u8>(result));
            }

            else if (ch == 'c')
            {
                PowerStage::status_S statusClear = {
                    .OCD_EVENT = true,
                    .OVT_EVENT = true,
                };

                result = p_powerStage->clearStatus(statusClear);
                if (result != PdsModule::error_E::OK)
                    _log.error("Unable to clear Power Stage status [ %u ]",
                               static_cast<u8>(result));
            }
        }

        getPowerStageData(psData);

        _log.info(
            "Power stage [ %u ] [ %s ] [ %s ] [ %s ] [ %.2f V ] [ %.2f A ] [ %.2f W ] [ %.2f *C ] "
            "[ %.1f A ] [ "
            "%.1f ms ]",
            static_cast<uint8_t>(POWER_STAGE_SOCKET_INDEX),
            psData.powerStageStatus.ENABLED ? "ON" : "OFF",
            psData.powerStageStatus.OCD_EVENT ? "OCD" : "---",
            psData.powerStageStatus.OVT_EVENT ? "OVT" : "---",
            psData.vBusVoltage_f,
            psData.loadCurrent_f,
            psData.loadPower_f,
            psData.temperature,
            psData.ocdLevel_f,
            psData.ocdDelay_f);
    }

    return EXIT_SUCCESS;
}