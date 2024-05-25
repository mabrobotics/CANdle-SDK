#pragma once

#include "bus.hpp"
#include "candle.hpp"
#include "mini/ini.h"

enum class toolsCmd_E
{
	NONE,
	PING,
	CONFIG,
	SETUP,
	TEST,
	BLINK,
	ENCODER,
	BUS,
	REGISTER,
	CLEAR,
	RESET,
};
enum class toolsOptions_E
{
	NONE,
	CURRENT,
	CAN,
	SAVE,
	ZERO,
	BANDWIDTH,
	CALIBRATION,
	DIAGNOSTIC,
	MOTOR,
	INFO,
	MOVE,
	LATENCY,
	CALIBRATION_OUTPUT,
	ENCODER,
	MAIN,
	OUTPUT,
	HOMING,
	ABSOLUTE,
	READ,
	WRITE,
	ERROR,
	WARNING,
	CLEAR,
};
struct UserCommand
{
	toolsCmd_E	   cmd		   = toolsCmd_E::NONE;
	toolsOptions_E option	   = toolsOptions_E::NONE;
	std::string	   variant	   = "all";
	u32			   id		   = 0;
	u32			   newId	   = 0;
	std::string	   baud		   = "1M";
	u32			   canWatchdog = 100;
	u32			   current	   = 1.f;
	u32			   bandwidth   = 100.f;
	std::string	   cfgPath	   = "~/.config/mdtool/mdtool_motors/";
	f32			   pos = 0.f, vel = 10.f, acc = 5.f, dcc = 5.f;
	bool		   infoAll = false;
};
class MDtool
{
  public:
	MDtool(const UserCommand& cmd);
	void ping(const std::string& variant);
	void configCan(u16 id, u16 newId, const std::string& baud, u16 timeout, bool termination = 0);
	void configSave(u16 id);
	void configZero(u16 id);
	void configCurrent(u16 id, f32 current);
	void configBandwidth(u16 id, f32 bandwidth);

	void setupCalibration(u16 id);
	void setupCalibrationOutput(u16 id);
	void setupMotor(u16 id, const std::string& cfgPath);
	void setupInfo(u16 id, bool printAll);
	void setupHoming(u16 id);

  private:
	const std::string mdtoolHomeConfigDirName = ".config";
	const std::string mdtoolDirName			  = "mdtool";
	const std::string mdtoolMotorCfgDirName	  = "mdtool_motors";
	const std::string mdtoolIniFileName		  = "mdtool.ini";

	const std::string mdtoolConfigPath = "/etc/";

	std::string mdtoolBaseDir;
	std::string mdtoolIniFilePath;

	std::unique_ptr<mab::Candle> candle;

	std::string busString;

	bool printVerbose = true;

	void configClear(std::vector<std::string>& args);

	void testMove(std::vector<std::string>& args);
	void testMoveAbsolute(std::vector<std::string>& args);
	void testLatency(std::vector<std::string>& args);
	void testEncoderOutput(std::vector<std::string>& args);
	void testEncoderMain(std::vector<std::string>& args);
	void registerWrite(std::vector<std::string>& args);
	void registerRead(std::vector<std::string>& args);
	void blink(std::vector<std::string>& args);
	void encoder(std::vector<std::string>& args);
	void bus(std::vector<std::string>& args);

	void clearErrors(std::vector<std::string>& args);
	void clearWarnings(std::vector<std::string>& args);
	void reset(std::vector<std::string>& args);

	void				  changeDefaultConfig(std::string bus, std::string device);
	mab::CANdleBaudrate_E checkSpeedForId(uint16_t id);

	uint8_t getNumericParamFromList(std::string& param, const std::vector<std::string>& list);

	bool checkErrors(uint16_t canId);

	template <class T>
	bool getField(mINI::INIStructure& cfg,
				  mINI::INIStructure& ini,
				  std::string		  category,
				  std::string		  field,
				  T&				  value);

	template <typename T>
	bool readRegisterToString(uint16_t id, mab::Md80Reg_E regId, std::string& str)
	{
		T	 value	= 0;
		bool status = candle->readMd80Register(id, regId, value);
		str			= std::to_string(value);
		return status;
	}

	bool checkArgs(std::vector<std::string>& args, uint32_t size);
	bool tryAddMD80(uint16_t id);
	int	 checkArgsAndGetId(std::vector<std::string>& args, uint32_t size, uint32_t idPos);
	bool checkSetupError(uint16_t id);
};
