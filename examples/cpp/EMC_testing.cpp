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

constexpr u16 PDS_CAN_ID = 100;

constexpr u16 MD_CAN_ID  = 100;
constexpr u16 MD2_CAN_ID = 101;

#define MOTOR_2

constexpr u32 INITIAL_BR_TRIGGER_VOLTAGE = 30000u;
constexpr f32 MIN_TORQUE                 = 0.0f;
constexpr f32 INITIAL_TORQUE             = 6.0f;
constexpr f32 MAX_TORQUE                 = 10.0f;

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

int main()
{
    setNonBlockingInput(true);

    _log.m_tag = "EMC Testing";

    std::shared_ptr<Candle> pCandle = std::make_shared<Candle>(mab::CAN_BAUD_1M, true);

    bool motorRun = false;
    s8   motorDir = 1;

    f32 motorTorque = INITIAL_TORQUE;

    pCandle->addMd80(MD_CAN_ID);
    pCandle->controlMd80Mode(MD_CAN_ID, mab::Md80Mode_E::RAW_TORQUE);  // Set mode to velocity PID

    // pCandle->writeMd80Register(MD_CAN_ID, Md80Reg_E::motorTorqueBandwidth, xxxx);

#ifdef MOTOR_2
    pCandle->addMd80(MD2_CAN_ID);
    pCandle->controlMd80Mode(MD2_CAN_ID, mab::Md80Mode_E::RAW_TORQUE);  // Set mode to velocity PID
#endif

    pCandle->controlMd80Enable(MD_CAN_ID, true);
#ifdef MOTOR_2
    pCandle->controlMd80Enable(MD2_CAN_ID, true);
#endif
    while (1)
    {
        usleep(100000);
        // pCandle->readMd80Register
        pCandle->writeMd80Register(
            MD_CAN_ID, Md80Reg_E::targetTorque, motorRun ? ((float)motorDir * motorTorque) : 0.0f);

#ifdef MOTOR_2
        pCandle->writeMd80Register(
            MD2_CAN_ID, Md80Reg_E::targetTorque, motorRun ? ((float)motorDir * motorTorque) : 0.0f);

#endif
        char ch;
        if (read(STDIN_FILENO, &ch, 1) == 1)
        {
            if (ch == 27)
            {
                _log.info("Exiting the program...");
                break;
            }

            else if (ch == 'm')
            {
                motorRun ^= 1;
            }

            else if (ch == 'n')
            {
                motorDir *= -1;
            }

            else if (ch == 'j')
            {
                if (motorTorque >= MIN_TORQUE)
                    motorTorque -= 0.1f;
            }

            else if (ch == 'k')
            {
                if (motorTorque <= MAX_TORQUE)
                    motorTorque += 0.1f;
            }
        }

        _log.info(
            "Motor TQ | %s | %d | %.2f nm |", motorRun ? "ON " : "OFF", motorDir, motorTorque);

        // std::cout << "\033[2A";
    }

    return EXIT_SUCCESS;
}