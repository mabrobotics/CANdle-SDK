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

int main()
{
    setNonBlockingInput(true);

    Logger log;
    log.m_tag = "PDS Example";

    bool enabled = false;

    u32 vbusVoltage = 0;
    s32 loadCurrent = 0;

    u32 ocdLevel = 10000;
    u32 ocdDelay = 1000;

    s32 loadPower = 0;

    float vBusVoltage_f = 0.0f;
    float loadCurrent_f = 0.0f;
    float loadPower_f   = 0.0f;
    float ocdLevel_f    = 0.0f;
    float ocdDelay_f    = 0.0f;
    float temperature   = 0.0f;

    std::shared_ptr<Candle> pCandle = std::make_shared<Candle>(mab::CAN_BAUD_1M, true);
    Pds                     pds(100, pCandle);

    Pds::modules_S pdsModules = {0};

    std::unique_ptr<PowerStage>    p_powerStage    = nullptr;
    std::unique_ptr<BrakeResistor> p_brakeResistor = nullptr;

    PdsModule::error_E result = PdsModule::error_E::OK;

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

    result = p_powerStage->disable();
    if (result != PdsModule::error_E::OK)
    {
        log.error("Power stage enable error [ %u ]", static_cast<u8>(result));
        exit(EXIT_FAILURE);
    }

    result = p_powerStage->setOcdLevel(ocdLevel);
    if (result != PdsModule::error_E::OK)
    {
        log.error("Unable to set OCD Level [ %u ]", static_cast<u8>(result));
        exit(EXIT_FAILURE);
    }

    result = p_powerStage->setOcdDelay(ocdDelay);
    if (result != PdsModule::error_E::OK)
    {
        log.error("Unable to set OCD Delay [ %u ]", static_cast<u8>(result));
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        usleep(100000);

        char ch;
        if (read(STDIN_FILENO, &ch, 1) == 1)
        {
            if (ch == ' ')
            {  // Check if the pressed key is the space key
                if (enabled)
                {
                    result = p_powerStage->disable();
                    if (result != PdsModule::error_E::OK)
                        log.error("Power stage disable error [ %u ]", static_cast<u8>(result));
                }
                else
                {
                    result = p_powerStage->enable();
                    if (result != PdsModule::error_E::OK)
                        log.error("Power stage enable error [ %u ]", static_cast<u8>(result));
                }
            }
            else if (ch == 27)
            {
                log.info("Exiting the program...");
                break;
            }
            else if (ch == 'q')
            {
                ocdLevel += 1000;
                result = p_powerStage->setOcdLevel(ocdLevel);
                if (result != PdsModule::error_E::OK)
                {
                    log.error("Unable to set OCD Level [ %u ]", static_cast<u8>(result));
                    // exit(EXIT_FAILURE);
                }
            }
            else if (ch == 'a')
            {
                if (ocdLevel >= 1000)
                    ocdLevel -= 1000;
                result = p_powerStage->setOcdLevel(ocdLevel);
                if (result != PdsModule::error_E::OK)
                {
                    log.error("Unable to set OCD Level [ %u ]", static_cast<u8>(result));
                    // exit(EXIT_FAILURE);
                }
            }
            else if (ch == 'w')
            {
                ocdDelay += 100;
                result = p_powerStage->setOcdDelay(ocdDelay);
                if (result != PdsModule::error_E::OK)
                {
                    log.error("Unable to set OCD Delay [ %u ]", static_cast<u8>(result));
                    // exit(EXIT_FAILURE);
                }
            }

            else if (ch == 's')
            {
                if (ocdDelay >= 100)
                    ocdDelay -= 100;

                result = p_powerStage->setOcdDelay(ocdDelay);
                if (result != PdsModule::error_E::OK)
                {
                    log.error("Unable to set OCD Delay [ %u ]", static_cast<u8>(result));
                    // exit(EXIT_FAILURE);
                }
            }

            else if (ch == 'b')
            {
                result = p_brakeResistor->enable();
                if (result != PdsModule::error_E::OK)
                    log.error("Brake resistor enable error [ %u ]", static_cast<u8>(result));
            }

            else if (ch == 'n')
            {
                result = p_brakeResistor->disable();
                if (result != PdsModule::error_E::OK)
                    log.error("Brake resistor enable error [ %u ]", static_cast<u8>(result));
            }
        }

        result = p_powerStage->getEnabled(enabled);
        if (result != PdsModule::error_E::OK)
            log.error("Reading enable property error [ %u ]", static_cast<u8>(result));

        result = p_powerStage->getOutputVoltage(vbusVoltage);
        if (result != PdsModule::error_E::OK)
            log.error("Reading VBus voltage error [ %u ]", static_cast<u8>(result));

        result = p_powerStage->getLoadCurrent(loadCurrent);
        if (result != PdsModule::error_E::OK)
            log.error("Reading load current error [ %u ]", static_cast<u8>(result));

        result = p_powerStage->getPower(loadPower);
        if (result != PdsModule::error_E::OK)
            log.error("Reading load power error [ %u ]", static_cast<u8>(result));

        result = p_powerStage->getTemperature(temperature);
        if (result != PdsModule::error_E::OK)
            log.error("Reading temperature error [ %u ]", static_cast<u8>(result));

        result = p_powerStage->getOcdLevel(ocdLevel);
        if (result != PdsModule::error_E::OK)
            log.error("Reading OCD Level error [ %u ]", static_cast<u8>(result));

        result = p_powerStage->getOcdDelay(ocdDelay);
        if (result != PdsModule::error_E::OK)
            log.error("Reading OCD Delay error [ %u ]", static_cast<u8>(result));

        vBusVoltage_f = static_cast<float>(vbusVoltage) / 1000.0f;
        loadCurrent_f = static_cast<float>(loadCurrent) / 1000.0f;
        loadPower_f   = static_cast<float>(loadPower) / 1000.0f;
        ocdLevel_f    = static_cast<float>(ocdLevel) / 1000.0f;
        ocdDelay_f    = static_cast<float>(ocdDelay) / 1000.0f;

        log.info(
            "Power stage [ %u ] [ %s ] [ %.2f V ] [ %.2f A ] [ %.2f W ] [ %.2f *C ] [ %.1f A ] [ "
            "%.1f ms ]",
            static_cast<uint8_t>(POWER_STAGE_SOCKET_INDEX),
            enabled ? "ON" : "OFF",
            vBusVoltage_f,
            loadCurrent_f,
            loadPower_f,
            temperature,
            ocdLevel_f,
            ocdDelay_f);
    }

    return EXIT_SUCCESS;
}