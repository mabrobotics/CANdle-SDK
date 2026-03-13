#include <tuple>
#include "candle.hpp"
#include "MD.hpp"

enum userChoice_IMPED
{
    STANDARD,
    HIGH_KP,
    KP_LOW,
    HIGH_KD,
    EXIT

};
enum userChoice_VEL
{
    CONSTANT_SPD,
    ACCELERATION
};
enum motionMode
{
    VELOCITY,
    POSITION,
    IMPEDANCE
};
///

void runImpedance(mab::MD&, Logger, int);
void runVelocity(mab::MD&, Logger, int);
void runPosition(mab::MD&, Logger, int, mab::MDRegisters_S&);
void runConfigurations(int, int, mab::MD&, mab::MDRegisters_S&);

///
void updateProgressBar(int current, int total, int width = 50)
{
    int pos = (current * width) / total;
    std::cout << "\r[";
    for (int i = 0; i < width; ++i)
    {
        if (i < pos)
            std::cout << "=";
        else
            std::cout << " ";
    }
    std::cout << "] " << (current * 100) / total << "%";
    std::cout.flush();
}
std::tuple<int, int> getUserChoice()
{
    int choice;
    int motionMode;

    int maxMotionMode = 3;

    int min = 1;

    while (true)
    {
        std::cout << "\033[1mChoose motion mode:\33[0m \n";
        std::cout << "1. Velocity\n";
        std::cout << "2. Position\n";
        std::cout << "3. Impedance\n";
        if (!(std::cin >> motionMode))
        {
            std::cout
                << "[\033[31mERROR\033[0m] Invalid input. Please enter a whole number (1-3)\n";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        }

        if (motionMode < min || motionMode > maxMotionMode)
        {
            std::cout << "[\033[31mERROR\033[0m] Please enter a number in range (1-3)\n";
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        }
        std::cout << motionMode << "\n";
        switch (motionMode - 1)
        {
            case VELOCITY:
                std::cout
                    << "\033[1mThe Velocity Mode allows to controll the velocity of the motor with "
                       "a torque limit which can be used as a safety feature\33[0m \n";
                std::cout << "\033[1mChoose test scenario by inserting a number (1-2):\33[0m \n";
                std::cout << "1. Constant speed limited by torque \n";
                std::cout << "2. Ramp speed mode (linear acceleration) \n";
                std::cin >> choice;
                if (choice >= 1 && choice <= 2)
                {
                    return std::make_tuple(choice - 1, motionMode - 1);
                }
                else
                {
                    std::cout << "[ERROR] Out of range. Please enter 1-2.\n";
                    continue;
                }
            case POSITION:
                std::cout
                    << "\033[1mThe Position Mode allows to controll the position of the motor and "
                       "hold it with a given torque. It also allows to controll the behaviour of "
                       "the motor when moved out of the desired position\33[0m \n";
                return std::make_tuple(choice - 1, motionMode - 1);
                continue;

            case IMPEDANCE:
                std::cout << "\033[1mChoose test scenario by inserting a number (1-5):\33[0m \n";
                std::cout << "1. Balanced mode: Kp and Kd tuned for fine performance\n";
                std::cout << "2. Oscillation mode: High Kp, low Kd (spring with little damping)\n";
                std::cout
                    << "3. Soft mode: Low Kp, high Kd, might result in big positioning error\n";
                std::cout << "4. Kd only mode, only the velocity error is beig regulated\n";
                std::cout << "5. EXIT\n";
                std::cout
                    << "You can always stop the program by using \033[1mCTRL+C\33[0m combination "
                       "in the terminal "
                       "window\n";

                // Check if input failed (non-numeric or overflow)
                if (!(std::cin >> choice))
                {
                    std::cin.clear();
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                    std::cout
                        << "[\033[31mERROR\033[0m] Invalid input. Please enter a whole number "
                           "between 1 and 4. To "
                           "exit enter 5.\n\n";
                    continue;
                }

                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

                if (choice >= 1 && choice <= 5 && motionMode >= 1 && motionMode)
                {
                    return std::make_tuple(choice - 1, motionMode - 1);
                }

                else
                {
                    std::cout << "[\033[31mERROR\033[0m] Out of range. Please enter 1-5.\n";
                }
                break;
            default:
                std::cout << "[\033[31mERROR\033[0m] Motion mode selection failed /n";
        }
    }
}
int main()
{
    // Attach Candle is an AIO method to get ready to use candle handle that corresponds to the real
    // CANdle USB-CAN converter. Its a main object so should have the longest lifetime of all
    // objects from the library.
    mab::Candle* candle = mab::attachCandle(mab::CANdleDatarate_E::CAN_DATARATE_1M,
                                            mab::candleTypes::busTypes_t::USB);

    constexpr mab::canId_t id = 968;  // 968;  // hardcoded ID
    mab::MD md(id, candle);  // create MD object with your motors id and attach it to the candle

    int choice, motionMode;

    mab::MDRegisters_S registerBuffer;

    while (true)
    {
        std::tie(choice, motionMode) = getUserChoice();
        if (choice == EXIT)
            break;
        runConfigurations(motionMode, choice, md, registerBuffer);
    }

    mab::detachCandle(candle);
    return EXIT_SUCCESS;
}

void runConfigurations(int motionMode, int choice, mab::MD& md, mab::MDRegisters_S& registerBuffer)
{
    // These parameters sets global internal logging level
    Logger::g_m_verbosity = Logger::Verbosity_E::VERBOSITY_1;
    Logger log(Logger::ProgramLayer_E::TOP, "User Program");

    // Selecting motion mode to IDLE Important note is that motion mode resets every time
    // MD is disabled or timed out
    md.setMotionMode(mab::MdMode_E::IDLE);
    md.setMaxVelocity(0.f);  // sets max velocity in rads/s

    switch (motionMode)
    {
        case VELOCITY:
            runVelocity(md, log, choice);
            break;
        case POSITION:
            runPosition(md, log, choice, registerBuffer);
            break;
        case IMPEDANCE:
            runImpedance(md, log, choice);
            break;

        default:
            log.info("MOTION MODE ERROR");
            break;
    }

    log.info("\n");
    md.disable();
    return;
}

////

void runVelocity(mab::MD& md, Logger log, int choice)
{
    md.setMotionMode(mab::MdMode_E::VELOCITY_PID);

    // regulator parameters and movement velocity limits
    float kp             = 0.003f;
    float ki             = 0.0005f;
    float kd             = 0.0f;
    float windup         = 0.25f;
    float targetVelocity = 10.f;
    float maxTorque      = 0.03f;
    float maxSpeed       = 350.f;
    float lowSpeed       = 10.f;
    float speedChange    = 8.f;
    float maxVelocity    = 400.f;

    md.zero();
    ///

    md.setVelocityPIDparam(kp, ki, kd, windup);
    md.setMaxTorque(maxTorque);
    md.setMaxVelocity(maxVelocity);

    ///
    bool upRamp = true;  // logic value to check if the current behavior is set to acceleration or
                         // deceleration
    constexpr int ITERATIONS = 10'000;

    md.enable();
    for (int i = 0; i < ITERATIONS; i++)
    {
        md.setTargetVelocity(targetVelocity);
        if (!(i % 100))
        {
            log.info("Position: %.2f | Velocity: %.2f | Torque: %.2f",
                     md.getPosition().first,
                     md.getVelocity().first,
                     md.getTorque().first);
            if (choice == userChoice_VEL::ACCELERATION)
            {
                if (upRamp)
                {
                    targetVelocity += speedChange;
                    if (targetVelocity >= maxSpeed)
                    {
                        targetVelocity = maxSpeed;
                        upRamp         = false;
                    }
                }
                else
                {
                    targetVelocity -= speedChange;
                    if (targetVelocity <= lowSpeed)
                    {
                        targetVelocity = lowSpeed;
                        upRamp         = true;
                    }
                }
            }
        }
        usleep(2'000);
    }
}
void runPosition(mab::MD& md, Logger log, int choice, mab::MDRegisters_S& registerBuffer)
{
    ///

    float kp_pos     = 25.1f;
    float ki_pos     = 2.8f;
    float kd_pos     = 0.f;
    float windup_pos = 5.0f;

    ///

    float kp_vel     = 0.003f;
    float ki_vel     = 0.0005f;
    float kd_vel     = 0.0f;
    float windup_vel = 0.25f;

    ///
    float         targetPosition = 6.24f;
    constexpr int ITERATIONS     = 5'000;
    float         maxVelocity    = 10.f;

    md.setMotionMode(mab::MdMode_E::POSITION_PID);
    md.setPositionPIDparam(kp_pos, ki_pos, kd_pos, windup_pos);
    md.setVelocityPIDparam(kp_vel, ki_vel, kd_vel, windup_vel);
    md.setMaxVelocity(maxVelocity);
    md.setMaxTorque(0.05);
    md.setTargetTorque(0.2);
    md.zero();
    md.enable();
    for (int i = 0; i < ITERATIONS; i++)
    {
        md.setTargetPosition(targetPosition);
        if (i % 500)
        {
            log.info(
                "Target position: %.3f | Current position: %.3f | Torque %.3f | Velocity: %.3f",
                targetPosition,
                md.getPosition().first /*Request positional data*/,
                md.getTorque().first,
                md.getVelocity().first);
        }

        usleep(2'000);
    }
}
///
void runImpedance(mab::MD& md, Logger log, int choice)
{
    // The code below is Impedance Mode example provided by MAB Robotics team, you can change the kp
    // and kd bear in mind that regulation with random values (or exaggerated values) might lead to
    // sustained oscillations and/or to uncontrolled acceleration of the motor

    md.setMaxVelocity(20.f);
    md.setMaxTorque(0.2);

    // kp and kd values are stored in the vectors below for easy access
    std::vector<float> kp_values = {0.055, 0.05, 0.02, 0};
    std::vector<float> kd_values = {0.0015, 0.0001, 0.002, 0.005};

    md.setMotionMode(mab::MdMode_E::IMPEDANCE);
    md.setImpedanceParams(kp_values[choice], kd_values[choice]);

    // Set the target position (rad)
    float targetPosition = 6.28f;  // approximate of 360 degrees
    float histeresis     = 0.1f;

    // zero the md position
    md.zero();

    ///
    // simple state flags to determine movement status
    bool isMovementFinished = false;
    bool messageFlag        = false;

    if (choice == KP_LOW)
    {
        histeresis = 1;
        log.info(
            "As a result of too low Kp combined with a relatively high Kd, the system may settle "
            "\n"
            "at a value lower than the desired setpoint"
            "\n"
            "to best visualize this effect, turn the arrow both clock-wise and counter clock-wise"
            "\n"
            "once the movement settles");
    }

    sleep(4);  // if you wish to add sleep functions to the code, do not do it after enabling
               // (md.enable) the driver

    ///
    constexpr int ITERATIONS = 5'000;
    md.enable();
    for (int i = 0; i < ITERATIONS; i++)
    {
        if (choice !=
            HIGH_KD)  // The if statement handles an exception in which we only have kd value with
                      // kp = 0, in such case setting target position is pointless and movement will
                      // be determined only by the target torque (the motor will run at constant
                      // speed with constant torque generated)
        {
            md.setTargetPosition(targetPosition);
            if ((md.getPosition().first + histeresis) >= targetPosition)
                isMovementFinished = true;
            if (i % 100 == 0 && !isMovementFinished)

                log.info("Target position: %.3f | Current position: %.3f",
                         targetPosition,
                         md.getPosition().first /*Request positional data*/);

            if (isMovementFinished)
            {
                if (!messageFlag)
                {
                    log.info(
                        "\033[1mIf the movement has finished, you are free to move the arrow by "
                        "hand\33[0m"
                        "\n "
                        "\033[1mto visualize the working principle of the Impedance mode\33[0m");
                    messageFlag = true;
                }
                else
                    updateProgressBar(i, ITERATIONS);
            }
        }

        else
        {
            md.setTargetTorque(0.2);
            if (i % 125 == 0 && !isMovementFinished)
            {
                log.info(
                    "Torque: %.7f | speed: %.3f", md.getTorque().first, md.getVelocity().first);
            }
        }
        usleep(2'000);
    }
}