#pragma once
#include "candle.hpp"
#include <mutex>
#include <atomic>
#include <vector>
#include <cmath>

struct commonMemory_S
{
    enum class busType_E
    {
        USB,
        SPI,
        UNKNOWN
    };
    busType_E busType;

    std::atomic<int> actual_thread_hz{0};
    std::mutex       mtx;
    float            targetVelocity     = 0.0f;
    float            targetPosition     = 0.0f;
    float            targetTorque       = 0.0f;
    float            targetAcceleration = 0.0f;
    float            targetDeceleration = 0.0f;

    // TUNING GAINS
    float Kp_vel          = 0.0f;
    float Ki_vel          = 0.0f;
    float Kd_vel          = 0.0f;
    float integralMax_vel = 0.0f;

    float Kp_pos          = 0.0f;
    float Ki_pos          = 0.0f;
    float Kd_pos          = 0.0f;
    float integralMax_pos = 0.0f;

    float Kp_imp = 0.0f;
    float Kd_imp = 0.0f;

    // MAXIMUM VALUES
    float maxVelocityClamp     = 0.0f;
    float maxPositionClamp     = 0.0f;
    float minPositionClamp     = 0.0f;
    float maxTorqueClamp       = 0.0f;
    float maxAccelerationClamp = 0.0f;
    float maxDecelerationClamp = 0.0f;

    float positionWindowSlider = 0.0f;
    float positionWindow       = 0.1f;
    float velocityWindowSlider = 0.0f;
    float velocityWindow       = 0.5f;

    float Kp_velSlider          = 0.0f;
    float Ki_velSlider          = 0.0f;
    float Kd_velSlider          = 0.0f;
    float integralMax_velSlider = 0.0f;

    float Kp_posSlider          = 0.0f;
    float Ki_posSlider          = 0.0f;
    float Kd_posSlider          = 0.0f;
    float integralMax_posSlider = 0.0f;

    float Kp_impSlider = 0.0f;
    float Kd_impSlider = 0.0f;

    float targetVelocitySlider     = 0.0f;
    float targetPositionSlider     = 0.0f;
    float targetTorqueSlider       = 0.0f;
    float targetAccelerationSlider = 0.0f;
    float targetDecelerationSlider = 0.0f;

    // MAB
    mab::MdMode_E             currentMode = mab::MdMode_E::IDLE;
    std::vector<mab::canId_t> mdIDs;
    mab::canId_t              chosenID = 0;

    // Logic
    std::atomic<bool> testStarted{false};

    std::atomic<bool> buttonDiscoverMdPressed{false};
    std::atomic<bool> buttonSelectMdPressed{false};
    std::atomic<bool> buttonManualTestPressed{false};
    std::atomic<bool> buttonAutomaticTestPressed{false};

    std::atomic<bool> updateParametersTest{false};
    std::atomic<bool> selectedMD{false};
    std::atomic<bool> selectedMode{false};
    std::atomic<bool> testOngoing{false};
    std::atomic<bool> candleAvailable{false};
    std::atomic<bool> updatedVersion{true};
    std::atomic<bool> discoverOngoing{false};
    std::atomic<bool> errorOccured{false};

    // Plots
    static const uint32_t PLOT_BUFFER_SIZE = 100000;

    float plotTime[PLOT_BUFFER_SIZE];
    float plotVelocity[PLOT_BUFFER_SIZE];
    float plotPosition[PLOT_BUFFER_SIZE];
    float plotTorque[PLOT_BUFFER_SIZE];
    float plotTargetVelocity[PLOT_BUFFER_SIZE];
    float plotTargetPosition[PLOT_BUFFER_SIZE];
    float plotTargetTorque[PLOT_BUFFER_SIZE];

    std::atomic<uint32_t> plotWriteData{0};

    float guiElapsedTime = 0.0f;

    uint32_t offset   = 0;
    uint32_t readData = 0;

    float minVel = 0.0f, maxVel = 0.0f;
    float minPos = 0.0f, maxPos = 0.0f;
    float minTrq = 0.0f, maxTrq = 0.0f;

    void reset()
    {
        offset = 0;
        maxVel = 0.0f;
        minVel = 0.0f;
        maxTrq = 0.0f;
        minTrq = 0.0f;
        maxPos = 0.0f;
        minPos = 0.0f;

        for (uint32_t i = 0; i < PLOT_BUFFER_SIZE; ++i)
        {
            plotTime[i]           = 0.0f;
            plotVelocity[i]       = 0.0f;
            plotPosition[i]       = 0.0f;
            plotTorque[i]         = 0.0f;
            plotTargetVelocity[i] = 0.0f;
            plotTargetPosition[i] = 0.0f;
            plotTargetTorque[i]   = 0.0f;
        }
    }
};