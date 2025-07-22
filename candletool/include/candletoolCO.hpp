#pragma once

#include <string>
#include "candle.hpp"
#include "mini/ini.h"
#include "logger.hpp"
#include "pds.hpp"
#include "MDCO.hpp"
// #include "../../candlelib/include/MDCO.hpp"

struct UserCommandCO
{
    std::string variant     = "";
    u32         id          = 0;
    u32         newId       = 0;
    std::string baud        = "1M";
    u32         canWatchdog = 100;
    f32         current     = 1.f;
    f32         bandwidth   = 100.f;
    std::string cfgPath     = "";
    f32         pos = 0.f, vel = 10.f, acc = 5.f, dcc = 5.f;
    bool        infoAll          = false;
    std::string bus              = "USB";
    std::string reg              = "0x0000";
    std::string value            = "";
    bool        force            = false;
    std::string firmwareFileName = "";
    bool        noReset          = false;
    bool        canOpen          = false;
    u16         maxCurrent       = 500;
    u16         maxTorque        = 200;
    u32         ratedCurrent     = 1000;
    u32         ratedTorque      = 1000;
    u32         MaxSpeed         = 1000;
    i32         desiredSpeed     = 50;
    u8          subReg           = 0;
    u8          dataSize         = 0;
    u32         HeartbeatTimeout = 1000;
    f32         kp               = 1;
    f32         kd               = 0;
    u16         torque           = 0;
};

uint64_t get_time_us();

class CandleToolCO
{
  public:
    explicit CandleToolCO(mab::CANdleBaudrate_E baud);
    ~CandleToolCO();
    void ping(const std::string& variant);
    void configCan(u16 id, u16 newId, const std::string& baud, u16 timeout, bool termination = 0);
    void configSave(u16 id);
    void configZero(u16 id);
    void configCurrent(u16 id, f32 current);
    void configBandwidth(u16 id, f32 bandwidth);
    void configClear(u16 id);
    void sendPdoSpeed(u16 id, i32 desiredSpeed);
    void sendPdoPosition(u16 id, i32 desiredSpeed);
    void setupCalibration(u16 id);
    void setupCalibrationOutput(u16 id);
    void setupMotor(u16 id, const std::string& cfgPath, bool force);
    void setupInfo(u16 id, bool printAll);
    void setupReadConfig(u16 id, const std::string& cfgName);
    void testMove(u16 id,
                  f32 targetPosition,
                  u32 MaxSpeed     = 200,
                  u16 MaxCurrent   = 500,
                  u32 RatedCurrent = 1000,
                  u16 MaxTorque    = 200,
                  u32 RatedTorque  = 1000);
    void testMoveAbsolute(u16 id,
                          f32 targetPos,
                          f32 velLimit,
                          f32 accLimit,
                          f32 dccLimit,
                          u16 MaxCurrent,
                          u32 RatedCurrent,
                          u16 MaxTorque,
                          u32 RatedTorque);
    void testMoveSpeed(u16 id,
                       u16 MaxCurrent,
                       u32 RatedCurrent,
                       u16 MaxTorque,
                       u32 RatedTorque,
                       u32 MaxSpeed,
                       i32 DesiredSpeed);
    void testMoveImpedance(u16 id,
                           i32 desiredSpeed,
                           f32 targetPos,
                           f32 kp,
                           f32 kd,
                           i16 torque,
                           u32 MaxSpeed,
                           u16 MaxCurrent,
                           u32 RatedCurrent,
                           u16 MaxTorque,
                           u32 RatedTorque);
    void testLatency(u16 id);
    void testEncoderOutput(u16 id);
    void testEncoderMain(u16 id);
    void blink(u16 id);
    void encoder(u16 id);
    void clearErrors(u16 id, const std::string& level);
    void reset(u16 id);
    void registerWrite(u16 id, u16 reg, const std::string& value, u8 subIndex, u8 dataSize);
    void SDOsegmentedRead(u16 id, u16 reg, u8 subIndex);
    void SDOsegmentedWrite(u16 id, u16 reg, u8 subIndex, std::string& data);

    void               registerRead(u16 id, u16 reg, u8 subIndex);
    static std::string clean(std::string s);
    void               heartbeatTest(u32 MasterId, u32 SlaveId, u32 HeartbeatTimeout);

    // Retrieve the Candle object
    mab::Candle* getCandle()
    {
        return m_candle;
    }

    /**
     * @brief Update firmware on Candle device
     *
     * @param firmwareFile path to firmware file (.mab)
     */
    void updateCandle(const std::string& mabFilePath);

    /**
     * @brief Update firmware on Motor Driver
     *
     * @param firmwareFile path to firmware file (.mab)
     * @param canId CAN ID of the motor driver to be updated
     */
    void updateMd(const std::string& mabFilePath, uint16_t canId, bool noReset = false);

    /**
     * @brief Update firmware on PDS
     *
     * @param firmwareFile path to firmware file (.mab)
     * @param canId CAN ID of the PDS to be updated
     */
    void updatePds(mab::Pds&          pds,
                   const std::string& mabFilePath,
                   uint16_t           canId,
                   bool               noReset = false);

  private:
    Logger       log;
    mab::Candle* m_candle;

    std::string busString;

    std::string validateAndGetFinalConfigPath(const std::string& cfgPath);

    u8 getNumericParamFromList(std::string& param, const std::vector<std::string>& list);

    template <class T>
    bool getField(mINI::INIStructure& cfg,
                  mINI::INIStructure& ini,
                  std::string         category,
                  std::string         field,
                  T&                  value);

    bool checkSetupError(u16 id);
};
