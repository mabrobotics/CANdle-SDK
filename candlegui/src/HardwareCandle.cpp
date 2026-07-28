#include "HardwareCandle.hpp"

HardwareCandle::HardwareCandle(std::shared_ptr<commonMemory_S> commonMemory) : m_data(commonMemory)
{
}

void HardwareCandle::testMD(mab::MD& md)
{
    switch (m_data->currentMode)
    {
        case mab::MdMode_E::IDLE:
            break;
        case mab::MdMode_E::VELOCITY_PID:
            md.setTargetVelocity(m_data->targetVelocity);
            break;
        case mab::MdMode_E::POSITION_PID:
            md.setTargetPosition(m_data->targetPosition);
            break;
        case mab::MdMode_E::IMPEDANCE:
            md.setTargetPosition(m_data->targetPosition);
            break;
        case mab::MdMode_E::RAW_TORQUE:  // case unused
            md.setTargetTorque(m_data->targetTorque);
            break;
        case mab::MdMode_E::VELOCITY_PROFILE:
            md.setTargetPosition(m_data->targetPosition);
            md.setTargetVelocity(m_data->targetVelocity);
            md.setProfileAcceleration(m_data->targetAcceleration);
            md.setProfileDeceleration(m_data->targetDeceleration);
            break;
        case mab::MdMode_E::POSITION_PROFILE:
            md.setTargetPosition(m_data->targetPosition);
            md.setTargetVelocity(m_data->targetVelocity);
            md.setProfileAcceleration(m_data->targetAcceleration);
            md.setProfileDeceleration(m_data->targetDeceleration);
            break;
        default:
            break;
    }
}

void HardwareCandle::downloadParameters(mab::MD& md)
{
    mab::MD::Error_t err = md.readRegisters(md.m_mdRegisters.motorImpPidKp,
                                            md.m_mdRegisters.motorImpPidKd,
                                            md.m_mdRegisters.motorVelPidKp,
                                            md.m_mdRegisters.motorVelPidKi,
                                            md.m_mdRegisters.motorVelPidKd,
                                            md.m_mdRegisters.motorPosPidKp,
                                            md.m_mdRegisters.motorPosPidKi,
                                            md.m_mdRegisters.motorPosPidKd,
                                            md.m_mdRegisters.positionWindow,
                                            md.m_mdRegisters.velocityWindow);

    mab::MD::Error_t err2 = md.readRegisters(md.m_mdRegisters.maxVelocity,
                                             md.m_mdRegisters.positionLimitMax,
                                             md.m_mdRegisters.positionLimitMin,
                                             md.m_mdRegisters.maxTorque,
                                             md.m_mdRegisters.maxAcceleration,
                                             md.m_mdRegisters.maxDeceleration,
                                             md.m_mdRegisters.motorVelPidWindup,
                                             md.m_mdRegisters.motorPosPidWindup);

    if (err != mab::MD::Error_t::OK && err2 != mab::MD::Error_t::OK)
    {
        std::cout << "Error reading registers: " << static_cast<uint8_t>(err) << "\n";
    }
    else
    {
        {
            std::lock_guard<std::mutex> lock(m_data->mtx);
            m_data->Kp_vel          = float(md.m_mdRegisters.motorVelPidKp.value);
            m_data->Ki_vel          = float(md.m_mdRegisters.motorVelPidKi.value);
            m_data->Kd_vel          = float(md.m_mdRegisters.motorVelPidKd.value);
            m_data->integralMax_vel = float(md.m_mdRegisters.motorVelPidWindup.value);

            m_data->Kp_pos          = float(md.m_mdRegisters.motorPosPidKp.value);
            m_data->Ki_pos          = float(md.m_mdRegisters.motorPosPidKi.value);
            m_data->Kd_pos          = float(md.m_mdRegisters.motorPosPidKd.value);
            m_data->integralMax_pos = float(md.m_mdRegisters.motorPosPidWindup.value);

            m_data->Kp_imp = float(md.m_mdRegisters.motorImpPidKp.value);
            m_data->Kd_imp = float(md.m_mdRegisters.motorImpPidKd.value);
        }

        m_data->positionWindowSlider = float(md.m_mdRegisters.positionWindow.value);
        m_data->velocityWindowSlider = float(md.m_mdRegisters.velocityWindow.value);

        m_data->maxVelocityClamp     = float(md.m_mdRegisters.maxVelocity.value);
        m_data->maxPositionClamp     = float(md.m_mdRegisters.positionLimitMax.value);
        m_data->minPositionClamp     = float(md.m_mdRegisters.positionLimitMin.value);
        m_data->maxTorqueClamp       = float(md.m_mdRegisters.maxTorque.value);
        m_data->maxAccelerationClamp = float(md.m_mdRegisters.maxAcceleration.value);
        m_data->maxDecelerationClamp = float(md.m_mdRegisters.maxDeceleration.value);

        // Write them on init to sliders
        m_data->Kp_velSlider          = m_data->Kp_vel;
        m_data->Ki_velSlider          = m_data->Ki_vel;
        m_data->Kd_velSlider          = m_data->Kd_vel;
        m_data->integralMax_velSlider = m_data->integralMax_vel;

        m_data->Kp_posSlider          = m_data->Kp_pos;
        m_data->Ki_posSlider          = m_data->Ki_pos;
        m_data->Kd_posSlider          = m_data->Kd_pos;
        m_data->integralMax_posSlider = m_data->integralMax_pos;

        m_data->Kp_impSlider = m_data->Kp_imp;
        m_data->Kd_impSlider = m_data->Kd_imp;
    }
}

void HardwareCandle::updateVelParameters()
{
    m_data->Kp_vel          = m_data->Kp_velSlider;
    m_data->Ki_vel          = m_data->Ki_velSlider;
    m_data->Kd_vel          = m_data->Kd_velSlider;
    m_data->integralMax_vel = m_data->integralMax_velSlider;
}

void HardwareCandle::updatePosParameters()
{
    m_data->Kp_pos          = m_data->Kp_posSlider;
    m_data->Ki_pos          = m_data->Ki_posSlider;
    m_data->Kd_pos          = m_data->Kd_posSlider;
    m_data->integralMax_pos = m_data->integralMax_posSlider;
}

void HardwareCandle::updateImpParameters()
{
    m_data->Kp_imp = m_data->Kp_impSlider;
    m_data->Kd_imp = m_data->Kd_impSlider;
}

const char* HardwareCandle::errorToString(mab::candleTypes::Error_t error)
{
    switch (error)
    {
        case mab::candleTypes::Error_t::OK:
            return "OK";
        case mab::candleTypes::Error_t::DEVICE_NOT_CONNECTED:
            return "DEVICE_NOT_CONNECTED";
        case mab::candleTypes::Error_t::INITIALIZATION_ERROR:
            return "INITIALIZATION_ERROR";
        case mab::candleTypes::Error_t::UNINITIALIZED:
            return "UNINITIALIZED";
        case mab::candleTypes::Error_t::DATA_TOO_LONG:
            return "DATA_TOO_LONG";
        case mab::candleTypes::Error_t::DATA_EMPTY:
            return "DATA_EMPTY";
        case mab::candleTypes::Error_t::RESPONSE_TIMEOUT:
            return "RESPONSE_TIMEOUT";
        case mab::candleTypes::Error_t::CAN_DEVICE_NOT_RESPONDING:
            return "CAN_DEVICE_NOT_RESPONDING";
        case mab::candleTypes::Error_t::TRANSMITTER_ERROR:
            return "TRANSMITTER_ERROR";
        case mab::candleTypes::Error_t::RECEIVER_ERROR:
            return "RECEIVER_ERROR";
        case mab::candleTypes::Error_t::INVALID_ID:
            return "INVALID_ID";
        case mab::candleTypes::Error_t::BAD_RESPONSE:
            return "BAD_RESPONSE";
        case mab::candleTypes::Error_t::UNKNOWN_ERROR:
            return "UNKNOWN_ERROR";
        default:
            return "UNDEFINED";
    }
}

void HardwareCandle::init()
{
    candle = nullptr;

    min = 0;
    max = 100;

    timeoutCounter = 0;
}

void HardwareCandle::candleLoop(std::atomic<bool>& isRunning)
{
    const std::chrono::microseconds                    dt = std::chrono::microseconds(200);
    std::chrono::time_point<std::chrono::steady_clock> nextExecTime =
        std::chrono::steady_clock::now();

    static std::chrono::time_point<std::chrono::steady_clock> testStartTime =
        std::chrono::steady_clock::now();
    static bool hardwareLastTestStarted = false;

    constexpr mab::canId_t MAX_VALID_ID = 0x7FF;
    while (isRunning)
    {
        bool testStarted             = m_data->testStarted.load();
        bool updateParametersTest    = m_data->updateParametersTest.exchange(false);
        bool buttonDiscoverMdPressed = m_data->buttonDiscoverMdPressed.exchange(false);
        bool buttonSelectMdPressed   = m_data->buttonSelectMdPressed.exchange(false);

        mab::MdMode_E currentMode;
        mab::canId_t  chosenID;

        currentMode = m_data->currentMode;
        chosenID    = m_data->chosenID;

        if (candle == nullptr)
        {
            auto busType =
                std::make_unique<mab::USB>(mab::Candle::CANDLE_VID, mab::Candle::CANDLE_PID);

            if (busType->connect() == mab::I_CommunicationInterface::Error_t::OK)
            {
                candle =
                    mab::attachCandle(mab::CANdleDatarate_E::CAN_DATARATE_1M, std::move(busType));
                if (candle != nullptr)
                {
                    m_data->candleAvailable = true;
                }
            }
        }

        if (candle != nullptr)
        {
            mab::candleTypes::Error_t errMsg = candle->legacyCheckConnection();

            if (errMsg == mab::candleTypes::Error_t::RESPONSE_TIMEOUT ||
                errMsg == mab::candleTypes::Error_t::RECEIVER_ERROR)
            {
                timeoutCounter += 1;
            }

            if ((errMsg != mab::candleTypes::Error_t::OK &&
                 errMsg != mab::candleTypes::Error_t::RESPONSE_TIMEOUT &&
                 errMsg != mab::candleTypes::Error_t::RECEIVER_ERROR) ||
                timeoutCounter > 5)
            {
                std::cout << "Error: " << errorToString(errMsg) << std::endl;
                std::lock_guard<std::mutex> lock(m_data->mtx);
                m_data->testStarted     = false;
                m_data->testOngoing     = false;
                m_data->candleAvailable = false;
                m_data->mdIDs.clear();
                buttonDiscoverMdPressed = false;
                m_data->discoverOngoing = false;

                min = 0;
                max = 100;

                mab::detachCandle(candle);
                candle = nullptr;
            }
            else
                timeoutCounter = 0;
        }

        if (candle != nullptr)
        {
            mab::MD md(chosenID, candle);

            if (buttonSelectMdPressed)
            {
                min = 0;
                max = 100;
                md.init();
                downloadParameters(md);
            }

            if (!testStarted && hardwareLastTestStarted)
            {
                {
                    std::lock_guard<std::mutex> lock(m_data->mtx);
                    m_data->testOngoing = false;
                }
                md.disable();
            }

            if (testStarted && !hardwareLastTestStarted)
            {
                testStartTime                = std::chrono::steady_clock::now();
                m_data->updateParametersTest = true;
            }
            hardwareLastTestStarted = testStarted;

            if (updateParametersTest)
            {
                md.zero();  // ZEROING FOR SAFETY TODO
                if (md.setMotionMode(currentMode) != mab::MD::Error_t::OK)
                {
                    std::cout << "MD mode setting failed \n";
                }

                switch (currentMode)
                {
                    case mab::MdMode_E::IDLE:
                        break;
                    case mab::MdMode_E::VELOCITY_PID:
                        m_data->targetVelocity = m_data->targetVelocitySlider;
                        m_data->targetPosition = 0.0f;
                        m_data->positionWindow = 0.01;
                        m_data->velocityWindow = m_data->velocityWindowSlider;
                        updateVelParameters();
                        md.setVelocityPIDparam(m_data->Kp_vel,
                                               m_data->Ki_vel,
                                               m_data->Kd_vel,
                                               m_data->integralMax_vel);
                        break;
                    case mab::MdMode_E::POSITION_PID:
                        m_data->targetPosition = m_data->targetPositionSlider;
                        m_data->targetVelocity = 0.0f;
                        m_data->velocityWindow = 0.01;
                        m_data->positionWindow = m_data->positionWindowSlider;
                        updatePosParameters();
                        md.setPositionPIDparam(m_data->Kp_pos,
                                               m_data->Ki_pos,
                                               m_data->Kd_pos,
                                               m_data->integralMax_pos);
                        break;
                    case mab::MdMode_E::IMPEDANCE:
                        m_data->targetPosition = m_data->targetPositionSlider;
                        updateImpParameters();
                        md.setImpedanceParams(m_data->Kp_imp, m_data->Kd_imp);
                        break;
                    case mab::MdMode_E::RAW_TORQUE:  // case unused
                        break;
                    case mab::MdMode_E::VELOCITY_PROFILE:
                        m_data->targetPosition     = m_data->targetPositionSlider;
                        m_data->targetVelocity     = m_data->targetVelocitySlider;
                        m_data->targetAcceleration = m_data->targetAccelerationSlider;
                        m_data->targetDeceleration = m_data->targetDecelerationSlider;
                        m_data->positionWindow     = 0.01;
                        m_data->velocityWindow     = m_data->velocityWindowSlider;
                        updateVelParameters();
                        updatePosParameters();
                        md.setVelocityPIDparam(m_data->Kp_vel,
                                               m_data->Ki_vel,
                                               m_data->Kd_vel,
                                               m_data->integralMax_vel);
                        md.setPositionPIDparam(m_data->Kp_pos,
                                               m_data->Ki_pos,
                                               m_data->Kd_pos,
                                               m_data->integralMax_pos);
                        break;
                    case mab::MdMode_E::POSITION_PROFILE:
                        m_data->targetPosition     = m_data->targetPositionSlider;
                        m_data->targetVelocity     = m_data->targetVelocitySlider;
                        m_data->targetAcceleration = m_data->targetAccelerationSlider;
                        m_data->targetDeceleration = m_data->targetDecelerationSlider;
                        m_data->velocityWindow     = 0.01;
                        m_data->positionWindow     = m_data->positionWindowSlider;
                        updateVelParameters();
                        updatePosParameters();
                        md.setVelocityPIDparam(m_data->Kp_vel,
                                               m_data->Ki_vel,
                                               m_data->Kd_vel,
                                               m_data->integralMax_vel);
                        md.setPositionPIDparam(m_data->Kp_pos,
                                               m_data->Ki_pos,
                                               m_data->Kd_pos,
                                               m_data->integralMax_pos);
                        break;
                    default:
                        break;
                }

                md.enable();
                m_data->updateParametersTest = false;
            }

            if ((testStarted && currentMode != mab::MdMode_E::IDLE))
            {
                {
                    std::lock_guard<std::mutex> lock(m_data->mtx);
                    m_data->testOngoing = true;
                }

                std::chrono::time_point<std::chrono::steady_clock> now =
                    std::chrono::steady_clock::now();
                std::chrono::duration<float> elapsed         = now - testStartTime;
                float                        realTimeSeconds = elapsed.count();

                testMD(md);

                md.readRegisters(
                    md.m_mdRegisters.velocity, md.m_mdRegisters.position, md.m_mdRegisters.torque);

                float vel = float(md.m_mdRegisters.velocity.value);
                float pos = float(md.m_mdRegisters.position.value);
                float trq = float(md.m_mdRegisters.torque.value);

                std::lock_guard<std::mutex> lock(m_data->mtx);
                uint32_t writeData = m_data->plotWriteData.load(std::memory_order_relaxed);
                uint32_t idx       = writeData % commonMemory_S::PLOT_BUFFER_SIZE;

                m_data->plotTime[idx]           = realTimeSeconds;
                m_data->plotVelocity[idx]       = vel;
                m_data->plotPosition[idx]       = pos;
                m_data->plotTorque[idx]         = trq;
                m_data->plotTargetVelocity[idx] = m_data->targetVelocity;
                m_data->plotTargetPosition[idx] = m_data->targetPosition;
                m_data->plotTargetTorque[idx]   = m_data->targetTorque;

                m_data->plotWriteData.store(writeData + 1, std::memory_order_release);
            }

            if (buttonDiscoverMdPressed)
            {
                m_data->discoverOngoing = true;
                {
                    std::lock_guard<std::mutex> lock(m_data->mtx);
                    m_data->mdIDs.clear();
                }
            }

            if (m_data->discoverOngoing)
            {
                for (const mab::canId_t& id : mab::MD::discoverRangedMDs(candle, min, max))
                {
                    m_data->mdIDs.push_back(id);
                }
                min += 100;
                max += 100;
                if (max > MAX_VALID_ID)
                {
                    m_data->discoverOngoing = false;
                    min                     = 0;
                    max                     = 100;
                }
            }
        }
        nextExecTime += dt;

        std::chrono::time_point<std::chrono::steady_clock> now = std::chrono::steady_clock::now();

        if (now < nextExecTime)
        {
            std::chrono::duration<float> timeLeft       = nextExecTime - now;
            std::chrono::milliseconds    sleepThreshold = std::chrono::milliseconds(1);

            if (timeLeft > sleepThreshold)
            {
                std::this_thread::sleep_for(timeLeft - sleepThreshold);
            }
            while (std::chrono::steady_clock::now() < nextExecTime)
            {
                std::this_thread::yield();
            }
        }
        else
            nextExecTime = std::chrono::steady_clock::now();
    }
}