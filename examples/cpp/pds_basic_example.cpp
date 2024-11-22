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

constexpr u16 PDS_CAN_ID = 100;
constexpr u16 MD_CAN_ID  = 105;

struct controlBoardData_S
{
    u32 vbusVoltage   = 0;
    f32 temperature   = 0.0f;
    f32 vBusVoltage_f = 0.0f;
};

struct powerStageData_S
{
    PowerStage::status_S status = {0};

    u32 vbusVoltage   = 0;
    u32 brTrigVoltage = 26000;
    s32 loadCurrent   = 0;
    s32 loadPower     = 0;
    u32 ocdLevel      = 10000;
    u32 ocdDelay      = 1000;
    f32 temperature   = 0.0f;

    f32 vBusVoltage_f   = 0.0f;
    f32 brTrigVoltage_f = 0.0f;
    f32 loadCurrent_f   = 0.0f;
    f32 loadPower_f     = 0.0f;
    f32 ocdLevel_f      = 0.0f;
    f32 ocdDelay_f      = 0.0f;
};

struct brakeResistorData_S
{
    BrakeResistor::status_S status = {0};

    f32 temperature      = 0.0f;
    f32 temperatureLimit = 50.0f;
};

static PdsModule::error_E getControlBoardData(Pds& pds, controlBoardData_S& cbData)
{
    PdsModule::error_E result = PdsModule::error_E::OK;

    result = pds.getBusVoltage(cbData.vbusVoltage);
    if (result != PdsModule::error_E::OK)
    {
        _log.error("Reading temperature error [ %u ]", static_cast<u8>(result));
    }

    result = pds.getTemperature(cbData.temperature);
    if (result != PdsModule::error_E::OK)
    {
        _log.error("Reading temperature error [ %u ]", static_cast<u8>(result));
    }

    cbData.vBusVoltage_f = static_cast<f32>(cbData.vbusVoltage) / 1000.0f;

    return result;
}
static PdsModule::error_E getPowerStageData(PowerStage& powerStage, powerStageData_S& psData)
{
    PdsModule::error_E result = PdsModule::error_E::OK;

    result = powerStage.getStatus(psData.status);
    if (result != PdsModule::error_E::OK)
    {
        _log.error("Reading status property error [ %u ]", static_cast<u8>(result));
        return result;
    }

    result = powerStage.getOutputVoltage(psData.vbusVoltage);
    if (result != PdsModule::error_E::OK)
    {
        _log.error("Reading VBus voltage error [ %u ]", static_cast<u8>(result));
        return result;
    }

    result = powerStage.getLoadCurrent(psData.loadCurrent);
    if (result != PdsModule::error_E::OK)
    {
        _log.error("Reading load current error [ %u ]", static_cast<u8>(result));
        return result;
    }

    result = powerStage.getPower(psData.loadPower);
    if (result != PdsModule::error_E::OK)
    {
        _log.error("Reading load power error [ %u ]", static_cast<u8>(result));
        return result;
    }

    result = powerStage.getTemperature(psData.temperature);
    if (result != PdsModule::error_E::OK)
    {
        _log.error("Reading temperature error [ %u ]", static_cast<u8>(result));
        return result;
    }

    result = powerStage.getOcdLevel(psData.ocdLevel);
    if (result != PdsModule::error_E::OK)
    {
        _log.error("Reading OCD Level error [ %u ]", static_cast<u8>(result));
        return result;
    }

    result = powerStage.getOcdDelay(psData.ocdDelay);
    if (result != PdsModule::error_E::OK)
    {
        _log.error("Reading OCD Delay error [ %u ]", static_cast<u8>(result));
        return result;
    }

    result = powerStage.getBrakeResistorTriggerVoltage(psData.brTrigVoltage);
    if (result != PdsModule::error_E::OK)
    {
        _log.error("Reading BR Trigger voltage error [ %u ]", static_cast<u8>(result));
        return result;
    }

    psData.vBusVoltage_f   = static_cast<f32>(psData.vbusVoltage) / 1000.0f;
    psData.brTrigVoltage_f = static_cast<f32>(psData.brTrigVoltage) / 1000.0f;
    psData.loadCurrent_f   = static_cast<f32>(psData.loadCurrent) / 1000.0f;
    psData.loadPower_f     = static_cast<f32>(psData.loadPower) / 1000.0f;
    psData.ocdLevel_f      = static_cast<f32>(psData.ocdLevel) / 1000.0f;
    psData.ocdDelay_f      = static_cast<f32>(psData.ocdDelay) / 1000.0f;

    return result;
}

static PdsModule::error_E getBrakeResistorData(BrakeResistor&       brakeResistor,
                                               brakeResistorData_S& brData)
{
    PdsModule::error_E result = PdsModule::error_E::OK;

    result = brakeResistor.getStatus(brData.status);
    if (result != PdsModule::error_E::OK)
    {
        _log.error("Reading BR Status error [ %u ]", static_cast<u8>(result));
        return result;
    }

    result = brakeResistor.getTemperature(brData.temperature);
    if (result != PdsModule::error_E::OK)
    {
        _log.error("Reading BR temperature error [ %u ]", static_cast<u8>(result));
        return result;
    }

    result = brakeResistor.getTemperatureLimit(brData.temperatureLimit);
    if (result != PdsModule::error_E::OK)
    {
        _log.error("Reading BR temperature limit error [ %u ]", static_cast<u8>(result));
        return result;
    }

    return result;
}

int main()
{
    setNonBlockingInput(true);

    controlBoardData_S  cbData;
    powerStageData_S    psData;
    brakeResistorData_S brData;
    _log.m_tag = "PDS Example";

    Candle candle(mab::CAN_BAUD_1M, true);
    Pds    pds(PDS_CAN_ID, candle);

    // Pds::modulesSet_t pdsModules = pds.getModules();

    PdsModule::error_E moduleResult = PdsModule::error_E::OK;

    // bool motorRun = false;
    // s8   motorDir = 1;

    // _log.info("PDS have the following numbers of connected modules:");
    // _log.info("\t PS    :: [ %u ]", pdsModules.powerStage);
    // _log.info("\t IC 12 :: [ %u ]", pdsModules.isolatedConv12V);
    // _log.info("\t IC 5  :: [ %u ]", pdsModules.isolatedConverter5V);
    // _log.info("\t BR    :: [ %u ]", pdsModules.brakeResistor);

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

    moduleResult = p_powerStage->disable();
    if (moduleResult != PdsModule::error_E::OK)
    {
        _log.error("Power stage enable error [ %u ]", static_cast<u8>(moduleResult));
        exit(EXIT_FAILURE);
    }

    moduleResult = p_powerStage->setOcdLevel(psData.ocdLevel);
    if (moduleResult != PdsModule::error_E::OK)
    {
        _log.error("Unable to set OCD Level [ %u ]", static_cast<u8>(moduleResult));
        exit(EXIT_FAILURE);
    }

    moduleResult = p_powerStage->setOcdDelay(psData.ocdDelay);
    if (moduleResult != PdsModule::error_E::OK)
    {
        _log.error("Unable to set OCD Delay [ %u ]", static_cast<u8>(moduleResult));
        exit(EXIT_FAILURE);
    }

    p_powerStage->enable();

    while (1)
    {
        usleep(100000);
        char ch;
        if (read(STDIN_FILENO, &ch, 1) == 1)
        {
            if (ch == ' ')
            {  // Check if the pressed key is the space key
                if (psData.status.ENABLED)
                {
                    _log.info("Disabling power stage!");
                    moduleResult = p_powerStage->disable();
                    if (moduleResult != PdsModule::error_E::OK)
                        _log.error("Power stage disable error [ %u ]",
                                   static_cast<u8>(moduleResult));
                }
                else
                {
                    _log.info("Enabling power stage!");
                    moduleResult = p_powerStage->enable();
                    if (moduleResult != PdsModule::error_E::OK)
                        _log.error("Power stage enable error [ %u ]",
                                   static_cast<u8>(moduleResult));
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
                moduleResult = p_powerStage->setOcdLevel(psData.ocdLevel);
                if (moduleResult != PdsModule::error_E::OK)
                {
                    _log.error("Unable to set OCD Level [ %u ]", static_cast<u8>(moduleResult));
                }
            }
            else if (ch == 'a')
            {
                if (psData.ocdLevel >= 1000)
                    psData.ocdLevel -= 1000;
                moduleResult = p_powerStage->setOcdLevel(psData.ocdLevel);
                if (moduleResult != PdsModule::error_E::OK)
                {
                    _log.error("Unable to set OCD Level [ %u ]", static_cast<u8>(moduleResult));
                }
            }
            else if (ch == 'w')
            {
                psData.brTrigVoltage += 100;
                moduleResult = p_powerStage->setBrakeResistorTriggerVoltage(psData.brTrigVoltage);
                if (moduleResult != PdsModule::error_E::OK)
                {
                    _log.error("Unable to set BR Trig Voltage [ %u ]",
                               static_cast<u8>(moduleResult));  // exit(EXIT_FAILURE);
                }
            }

            else if (ch == 's')
            {
                if (psData.brTrigVoltage >= 100)
                    psData.brTrigVoltage -= 100;

                moduleResult = p_powerStage->setBrakeResistorTriggerVoltage(psData.brTrigVoltage);
                if (moduleResult != PdsModule::error_E::OK)
                {
                    _log.error("Unable to set BR Trig Voltage [ %u ]",
                               static_cast<u8>(moduleResult));
                }
            }

            else if (ch == 'e')
            {
                brData.temperatureLimit += 5.0f;
                moduleResult = p_brakeResistor->setTemperatureLimit(brData.temperatureLimit);
                if (moduleResult != PdsModule::error_E::OK)
                {
                    _log.error("Unable to set BR Trig Voltage [ %u ]",
                               static_cast<u8>(moduleResult));  // exit(EXIT_FAILURE);
                }
            }

            else if (ch == 'c')
            {
                PowerStage::status_S statusClear = {
                    .OCD_EVENT = true,
                    .OVT_EVENT = true,
                };

                moduleResult = p_powerStage->clearStatus(statusClear);
                if (moduleResult != PdsModule::error_E::OK)
                    _log.error("Unable to clear Power Stage status [ %u ]",
                               static_cast<u8>(moduleResult));

                moduleResult = p_brakeResistor->clearStatus(statusClear);
                if (moduleResult != PdsModule::error_E::OK)
                    _log.error("Unable to clear Brake resistor status [ %u ]",
                               static_cast<u8>(moduleResult));
            }
        }

        if (PdsModule::error_E::OK != getControlBoardData(pds, cbData))
        {
            exit(EXIT_FAILURE);
        }

        if (PdsModule::error_E::OK != getPowerStageData(*p_powerStage, psData))
        {
            exit(EXIT_FAILURE);
        }

        if (PdsModule::error_E::OK != getBrakeResistorData(*p_brakeResistor, brData))
        {
            exit(EXIT_FAILURE);
        }

        _log.info("CTRL | ON  | --- | --- | %.2f V | ------ | ------- | %.2f *C ] ",
                  cbData.vBusVoltage_f,
                  cbData.temperature);

        _log.info(
            "PS %u | %s | %s | %s | %.2f V | %.2f A | %.2f W | %.2f "
            "*C ] "
            "[ %.1f A ] [ "
            "%.2f V ]",
            static_cast<uint8_t>(POWER_STAGE_SOCKET_INDEX),
            psData.status.ENABLED ? "ON " : "OFF",
            psData.status.OCD_EVENT ? "OCD" : "---",
            psData.status.OVT_EVENT ? "OVT" : "---",
            psData.vBusVoltage_f,
            psData.loadCurrent_f,
            psData.loadPower_f,
            psData.temperature,
            psData.ocdLevel_f,
            psData.brTrigVoltage_f);

        _log.info("BR %u | %s | --- | %s | ------ | ------ | ------- | %.2f *C ] ",
                  static_cast<uint8_t>(BRAKE_RESISTOR_SOCKET_INDEX),
                  brData.status.ENABLED ? "ON " : "OFF",
                  brData.status.OVT_EVENT ? "OVT" : "---",
                  brData.temperature);
    }

    return EXIT_SUCCESS;
}