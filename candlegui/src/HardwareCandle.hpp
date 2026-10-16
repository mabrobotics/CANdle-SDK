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

    mab::Candle*                          candle = nullptr;
    std::vector<std::unique_ptr<mab::MD>> mds;
    std::vector<MDStatus_S>               mdStatusLocal;

    mab::MDStatus statuses;
    mab::canId_t  chosenID;

    bool                                       mdListChanged = true;
    std::chrono::steady_clock::time_point      lastStatusTime{};
    std::chrono::steady_clock::time_point      monitorStartTime = std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point      lastSlowTime{};
    static constexpr std::chrono::milliseconds STATUS_PERIOD{100};
    static constexpr std::chrono::milliseconds SLOW_PERIOD{100};

    bool quickStatusOk(uint16_t raw);

    int timeoutCounter = 0;
    inline static std::map<mab::canId_t, std::chrono::time_point<std::chrono::steady_clock>>
          errorStartTimes;
    float beginStepTime = 0.025f;

    mab::canId_t min = 0;
    mab::canId_t max = 100;

    const mab::canId_t MAX_VALID_ID        = 0x7FF;
    const int          MAX_CONNECT_RETRIES = 100;

    mab::MD::Error_t communicationCheck(mab::MD& md);

    float readDriverTemp(mab::MD& md);
    void  copyLiveValues(mab::MD& md, MDStatus_S& status);
    bool  readLiveValues(mab::MD& md, MDStatus_S& status);

    void testMD(mab::MD& md);
    void checkConnectionStatus(mab::MD& md, mab::canId_t chosenID);
    void checkQuickStatus(mab::MD& md);
    void downloadParameters(mab::MD& md);

    void updateVelParameters();
    void updatePosParameters();
    void updateImpParameters();

    const char* errorToString(mab::candleTypes::Error_t error);
};