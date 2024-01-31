#ifndef _MDTOOL_HPP
#define _MDTOOL_HPP

#include <optional>
#include <unordered_map>

#include "Candle.hpp"
#include "UsbHandler.hpp"

class Mdtool
{
   public:
	Mdtool() = delete;
	Mdtool(spdlog::logger* logger);
	~Mdtool();

	bool init(ICommunication* interface, Candle::Baud baud);
	void ping();
	bool updateMd80(std::string& filePath, uint32_t id, bool recover, bool all);
	bool updateBootloader(std::string& filePath, uint32_t id, bool recover);
	bool updateCANdle(std::string& filePath, bool recover);
	bool readSDO(uint32_t id, uint16_t index, uint8_t subindex);
	bool writeSDO(uint32_t id, uint16_t index, uint8_t subindex, const IODParser::ValueType& value);
	bool calibrate(uint32_t id);
	bool home(uint32_t id);
	bool save(uint32_t id);
	bool status(uint32_t id);
	bool changeId(uint32_t id, uint32_t newId);
	bool changeBaud(uint32_t id, uint32_t newBaud);
	bool clearError(uint32_t id);
	bool clearWarning(uint32_t id);

   private:
	static constexpr uint32_t secondaryBootloaderAddress = 0x8005000;
	static constexpr uint32_t primaryBootloaderAddress = 0x8000000;
	std::unique_ptr<Candle> candle;
	ICommunication* interface;
	spdlog::logger* logger;

	Candle::Baud baudrate = Candle::Baud::BAUD_1M;

   private:
	IODParser::ValueType getTypeBasedOnTag(IODParser::DataType tag);
	std::optional<IODParser::Entry*> checkEntryExists(MD80* md80, uint16_t index, uint8_t subindex);

	using errorMapType = const std::unordered_map<std::string, uint32_t>;

	errorMapType encoderErrorList = {{"ERROR_COMMUNICATION", (1 << 0)},
									 {"ERROR_WRONG_DIRECTION", (1 << 1)},
									 {"ERROR_EMPTY_LUT", (1 << 2)},
									 {"ERROR_FAULTY_LUT", (1 << 3)},
									 {"ERROR_CALIBRATION_FAILED", (1 << 4)},
									 {"ERROR_POSITON_INVALID", (1 << 5)},
									 {"ERROR_INIT", (1 << 6)},
									 {"WARNING_LOW_ACCURACY", (1 << 30)}};

	errorMapType calibrationErrorList = {{"ERROR_OFFSET_CAL", (1 << 0)},
										 {"ERROR_RESISTANCE_IDENT", (1 << 1)},
										 {"ERROR_INDUCTANCE_IDENT", (1 << 2)},
										 {"ERROR_POLE_PAIR_CAL", (1 << 3)},
										 {"ERROR_SETUP", (1 << 4)}};

	errorMapType bridgeErrorList = {{"ERROR_BRIDGE_COM", (1 << 0)},
									{"ERROR_BRIDGE_OC", (1 << 1)},
									{"ERROR_BRIDGE_GENERAL_FAULT", (1 << 2)}};

	errorMapType hardwareErrorList = {{"ERROR_OVER_CURRENT", (1 << 0)},
									  {"ERROR_OVER_VOLTAGE", (1 << 1)},
									  {"ERROR_UNDER_VOLTAGE", (1 << 2)},
									  {"ERROR_MOTOR_TEMP", (1 << 3)},
									  {"ERROR_MOSFET_TEMP", (1 << 4)},
									  {"ERROR_ADC_CURRENT_OFFSETS", (1 << 5)}};

	errorMapType communicationErrorList = {{"WARNING_CAN_WD", (1 << 30)}};

	errorMapType homingErrorList = {{"ERROR_HOMING_LIMIT_REACHED", (1 << 0)},
									{"ERROR_HOMING_SEQUENCE", (1 << 1)},
									{"ERROR_HOMING_REQUIRED", (1 << 2)},
									{"ERROR_HOMING_SETUP", (1 << 3)},
									{"ERROR_HOMING_ABORTED", (1 << 4)}};

	errorMapType motionErrorList = {{"ERROR_POSITION_OUTSIDE_LIMITS", (1 << 0)},
									{"ERROR_VELOCITY_OUTSIDE_LIMITS", (1 << 1)},
									{"WARNING_ACCELERATION_CLIPPED", (1 << 24)},
									{"WARNING_TORQUE_CLIPPED", (1 << 25)},
									{"WARNING_VELOCITY_CLIPPED", (1 << 26)},
									{"WARNING_POSITION_CLIPPED", (1 << 27)}};
};

#endif