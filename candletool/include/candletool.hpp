#pragma once

#include <string>
#include "candle.hpp"
#include "mini/ini.h"
#include "logger.hpp"
#include "pds.hpp"

struct UserCommand
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
};

class CandleTool
{
  public:
    explicit CandleTool(mab::CANdleBaudrate_E baud);
    ~CandleTool();
    void ping(const std::string& variant);
    void configCan(u16 id, u16 newId, const std::string& baud, u16 timeout, bool termination = 0);
    void configSave(u16 id);
    void configZero(u16 id);
    void configCurrent(u16 id, f32 current);
    void configBandwidth(u16 id, f32 bandwidth);
    void configClear(u16 id);

    void setupCalibration(u16 id);
    void setupCalibrationOutput(u16 id);
    void setupMotor(u16 id, const std::string& cfgPath, bool force);
    void setupInfo(u16 id, bool printAll);
    void setupReadConfig(u16 id, const std::string& cfgName);

    void testMove(u16 id, f32 targetPosition);
    void testMoveAbsolute(u16 id, f32 targetPos, f32 velLimit, f32 accLimit, f32 dccLimit);
    void testLatency(const std::string& canBaudrate, std::string busString);
    void testEncoderOutput(u16 id);
    void testEncoderMain(u16 id);
    void blink(u16 id);
    void encoder(u16 id);
    void bus(const std::string& bus, const std::string& device);
    void clearErrors(u16 id, const std::string& level);
    void reset(u16 id);
    void registerWrite(u16 id, u16 reg, const std::string& value);
    void registerRead(u16 id, u16 reg);

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
