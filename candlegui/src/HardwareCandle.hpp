#pragma once
#include <memory>

#include "commonMemory.hpp"
#include "MD.hpp"

class HardwareCandle
{
  public:
    HardwareCandle(std::shared_ptr<commonMemory_S> commonMemory);

    void init();

    void candleLoop(std::atomic<bool>& isRunning);

  private:
    std::shared_ptr<commonMemory_S> m_data;

    mab::Candle* candle = nullptr;

    mab::MDStatus statuses;
    mab::canId_t  chosenID;

    int timeoutCounter = 0;

    mab::canId_t min = 0;
    mab::canId_t max = 100;

    const mab::canId_t MAX_VALID_ID        = 0x7FF;
    const int          MAX_CONNECT_RETRIES = 100;

    void testMD(mab::MD& md);
    void checkConnectionStatus(mab::MD& md, mab::canId_t chosenID);
    void checkQuickStatus(mab::MD& md);
    void downloadParameters(mab::MD& md);

    void updateVelParameters();
    void updatePosParameters();
    void updateImpParameters();

    const char* errorToString(mab::candleTypes::Error_t error);
};