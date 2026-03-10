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

enum motorType
{
    GL_30,
    GL_35
};
void runImpedance(mab::MD&, Logger, int, int);
void runVelocity(mab::MD&, Logger, int);
void runPosition();
void runConfigurations(int, int, int, mab::MD&, mab::MDRegisters_S*);
int  getMotorType(mab::MD&, mab::MDRegisters_S*);

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
    // int maxChoice     = 5;
    int maxMotionMode = 3;
    // int maxVelocityMode = 2;
    // int maxPositionMode = 0;
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

    // The code below is an easy explenation OF how to use auto detection of md modules, it can be
    // also found in the md_example_impedance.cpp file

    // Look for MAB devices present on the can network
    // auto ids = mab::MD::discoverMDs(candle);

    // This method provides you with memory-safe map handle which you can assign MDs too manually
    // via addMD method or via discoverDevices
    // std::vector<mab::MD> mds;
    // for (auto id : ids)
    // {
    //     mab::MD md(id, candle);
    //     if (md.init() == mab::MD::Error_t::OK)
    //         mds.push_back(md);
    // }

    // if (mds.size() == 0)
    // {
    //     log.error("No MDs found!");
    //     return EXIT_FAILURE;
    // }

    // Get first md that was detected. This is not a memory safe handle so it should not be moved a
    // lot in this form.
    // mab::MD md = mds[0];

    // You can also hardcode the ID of the device you are using
    constexpr mab::canId_t id = 968;  // 968;  // hardcoded ID
    mab::MD md(id, candle);  // create MD object with your motors id and attach it to the candle

    // Selecting motion mode to be impedance. Important note is that motion mode resets every time
    // MD is disabled or timed out
    int                choice, motionMode;
    int                motorType;
    mab::MDRegisters_S registerBuffer;
    motorType = getMotorType(md, &registerBuffer);
    std::cout << "Motor type: " << motorType << "\n";
    while (true)
    {
        std::tie(choice, motionMode) = getUserChoice();
        if (choice == EXIT)
            break;
        runConfigurations(motionMode, choice, motorType, md, &registerBuffer);
    }

    mab::detachCandle(candle);
    return EXIT_SUCCESS;
}

///
int getMotorType(mab::MD& md, mab::MDRegisters_S* registerBuffer)
{
    char             tmp[3];
    mab::MD::Error_t err          = md.readRegister(registerBuffer->motorName);
    std::string      actuatorName = std::string(registerBuffer->motorName.value);

    if (err != mab::MD::Error_t::OK)
    {
        std::cout << "Error reading registers: " << static_cast<u8>(err) << "\n";
        return EXIT_FAILURE;
    }
    else
        std::cout << "Motor name: " << std::string(registerBuffer->motorName.value) << "\n";
    tmp[0] = actuatorName[7];
    tmp[1] = actuatorName[8];
    tmp[2] = '\0';
    if (std::atoi(tmp) == 35)
        return GL_35;

    return GL_30;
}
///

void runConfigurations(
    int motionMode, int choice, int motorType, mab::MD& md, mab::MDRegisters_S* registerBuffer)
{
    Logger::g_m_verbosity = Logger::Verbosity_E::VERBOSITY_1;
    Logger log(Logger::ProgramLayer_E::TOP, "User Program");
    if (registerBuffer != nullptr)
    {
        // This parameters sets global internal logging level

        md.setMotionMode(mab::MdMode_E::IDLE);
        switch (motionMode)
        {
            case VELOCITY:
                runVelocity(md, log, choice);
                break;
            case POSITION:
                // runPosition();
                break;
            case IMPEDANCE:
                runImpedance(md, log, choice, motorType);
                break;

            default:
                log.info("MOTION MODE ERROR");
                break;
        }

        log.info("\n");
        md.disable();
        return;
    }
    else
    {
        log.info("Program Error");
        return;
    }
}

////

void runVelocity(mab::MD& md, Logger log, int choice)
{
    md.enable();
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

    md.zero();
    md.setProfileVelocity(targetVelocity);
    md.setVelocityPIDparam(kp, ki, kd, windup);
    md.setMaxTorque(maxTorque);
    bool upRamp = true;  // logic value to check if the current behavior is set to acceleration or
                         // deceleration
    for (int i = 0; i < 10000; i++)
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

///
void runImpedance(mab::MD& md, Logger log, int choice, int motorType)
{
    md.enable();
    md.setMaxTorque(0.2);
    // kp and kd values are stored in the vectors below for easy access
    std::vector<std::vector<float>> kp_values = {
        {0.055, 0.05, 0.02, 0},  // values for gl-30
        {0.02, 0.05, 0.01, 0}    // values for gl-35
    };
    std::vector<std::vector<float>> kd_values = {
        {0.0015, 0.0001, 0.002, 0.005},  // values for gl-30
        {0.0005, 0.0001, 0.002, 0.005}   // values for gl-35
    };
    // float newTargetVelocity       = 1.0f;
    // registerBuffer.targetVelocity = newTargetVelocity;
    // md.writeRegister(registerBuffer.targetVelocity);
    // Logger is a standalone builtin class for handling output from different modules

    // md.readRegister(0x010, motorName, sizeof(motorName);

    md.setMotionMode(mab::MdMode_E::IMPEDANCE);
    // md->setImpedanceParams(kp_values[0][choice], kd_values[0][choice]);
    md.setImpedanceParams(0.055, 0.0015);

    //  md.setImpedanceParams(0.0015f, 0.00005f);
    //  Enable the drive. From this point on it is our objective to send regular messages to the MD,
    //  otherwise it will timeout via watchdog and disable itself.

    // Set the target position (rad)
    float targetPosition = 6.28f;  // approximate of 360 degrees
    float histeresis     = 0.1f;

    // md.setTargetVelocity(0.1f);

    // zero the md position
    md.zero();
    bool isMovementFinished = false;
    bool messageFlag        = false;

    // The code below is an example provided by MAB Robotics team, you can change the kp and kd
    // values but do it carefully, when regulated with random values (or exaggerated values) might
    // lead to sustained oscillations and/or to uncontrolled acceleration of the motor

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

    sleep(4);
    int placeHolder = 5000;

    for (int i = 0; i < placeHolder; i++)
    {
        if (choice != HIGH_KD)
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
                    updateProgressBar(i, placeHolder);
            }
        }

        else
        {
            // md.setTargetVelocity(3);
            // md.setTargetPosition(targetPosition);

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