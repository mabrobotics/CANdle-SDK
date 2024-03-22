#ifndef MD80_HPP
#define MD80_HPP

#include <cstdint>
#include <map>
#include <memory>

#include "CanopenStack/CANopen/CanopenStack.hpp"
#include "IObjectDictionaryParser.hpp"
#include "ObjectDictionaryParserEDS.hpp"

/**
 * @brief implements general methods for reading and writing data to MD80 drivers
 *
 */
class MD80
{
   public:
	enum RoutineID
	{
		BLINK = 1,
		RESET,
		CALIBRATION,
		CALIBRATION_AUX,
		SET_ZERO,
		CALIBRATE_PI_GAINS,
		TEST_OUTPUT_ENCODER,
		TEST_MAIN_ENCODER,
		SAVE,
		REVERT_FACTORY_SETTINGS,
		CAN_REINIT,
		RUN_HOMING,
	};

	MD80(uint32_t id, std::shared_ptr<CanopenStack> canopenStack) : id(id), canopenStack(canopenStack) {}

	float getOutputPosition(bool useSDO = true) { return readAccessHelper<float>(0x2009, 0x01, useSDO); }
	float getOutputVelocity(bool useSDO = true) { return readAccessHelper<float>(0x2009, 0x02, useSDO); }
	float getOutputTorque(bool useSDO = true) { return readAccessHelper<float>(0x2009, 0x03, useSDO); }

	uint32_t getQuickStatus(bool useSDO = true) { return readAccessHelper<uint32_t>(0x2004, 0x0C, useSDO); }

	bool setPositionTarget(float position, bool useSDO = true) { return writeAccessHelper(0x2008, 0x09, position, useSDO); }
	bool setVelocityTarget(float velocity, bool useSDO = true) { return writeAccessHelper(0x2008, 0x0A, velocity, useSDO); }
	bool setTorqueTarget(float torque, bool useSDO = true) { return writeAccessHelper(0x2008, 0x0B, torque, useSDO); }

	bool isTargetReached(bool useSDO = true) { return readAccessHelper<uint16_t>(0x6041, 0x00, useSDO) & (1 << 10); }

	bool setVelocityPidGains(float kp, float ki, float kd, float intLimit, bool useSDO = true)
	{
		return writeAccessHelper(0x2001, 0x01, kp, useSDO) ||
			   writeAccessHelper(0x2001, 0x02, ki, useSDO) ||
			   writeAccessHelper(0x2001, 0x03, kd, useSDO) ||
			   writeAccessHelper(0x2001, 0x04, intLimit, useSDO);
	}

	bool setPositionPidGains(float kp, float ki, float kd, float intLimit, bool useSDO = true)
	{
		return writeAccessHelper(0x2002, 0x01, kp, useSDO) ||
			   writeAccessHelper(0x2002, 0x02, ki, useSDO) ||
			   writeAccessHelper(0x2002, 0x03, kd, useSDO) ||
			   writeAccessHelper(0x2002, 0x04, intLimit, useSDO);
	}

	bool runRoutine(RoutineID routineId, bool shouldWaitForCompletion)
	{
		bool inProgress = true;
		uint8_t routineSubindex = static_cast<uint8_t>(routineId);

		if (!writeAccessHelper(0x2003, routineSubindex, inProgress, true))
			return false;

		while (inProgress && shouldWaitForCompletion)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			inProgress = readAccessHelper<bool>(0x2003, routineSubindex, true);
		}
		return true;
	}

   public:
	IODParser::ODType OD;

   private:
	template <typename T>
	T readAccessHelper(uint16_t index, uint8_t subindex, bool useSDO)
	{
		if (!useSDO)
			return std::get<T>(OD.at(index)->subEntries.at(subindex)->value);

		T value = 0;
		uint32_t errorCode = 0;
		if (!canopenStack->readSDO(id, index, subindex, value, errorCode))
			std::cout << "Error reading SDO!" << std::endl;
		return value;
	}

	template <typename T>
	bool writeAccessHelper(uint16_t index, uint8_t subindex, T value, bool useSDO)
	{
		if (!useSDO)
		{
			OD.at(index)->subEntries.at(subindex)->value = value;
			return true;
		}

		uint32_t errorCode = 0;
		if (!canopenStack->writeSDO(id, index, subindex, value, errorCode))
			return false;
		return true;
	}

   private:
	uint32_t id;
	std::shared_ptr<CanopenStack> canopenStack;
};

#endif