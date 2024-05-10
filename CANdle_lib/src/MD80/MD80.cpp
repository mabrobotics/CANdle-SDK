#include "MD80/MD80.hpp"

MD80::MD80(uint32_t id, std::shared_ptr<CanopenStack> canopenStack) : id(id), canopenStack(canopenStack) {}

float MD80::getOutputPosition(bool useSDO)
{
	return readAccessHelper<float>(0x2009, 0x01, useSDO);
}
float MD80::getOutputVelocity(bool useSDO)
{
	return readAccessHelper<float>(0x2009, 0x02, useSDO);
}
float MD80::getOutputTorque(bool useSDO)
{
	return readAccessHelper<float>(0x2009, 0x03, useSDO);
}
bool MD80::setPositionTarget(float position, bool useSDO)
{
	return writeAccessHelper(0x2008, 0x09, position, useSDO);
}
bool MD80::setVelocityTarget(float velocity, bool useSDO)
{
	return writeAccessHelper(0x2008, 0x0A, velocity, useSDO);
}
bool MD80::setTorqueTarget(float torque, bool useSDO)
{
	return writeAccessHelper(0x2008, 0x0B, torque, useSDO);
}
uint32_t MD80::getQuickStatus(bool useSDO)
{
	return readAccessHelper<uint32_t>(0x2004, 0x0C, useSDO);
}
bool MD80::isTargetReached(bool useSDO)
{
	return readAccessHelper<uint16_t>(0x6041, 0x00, useSDO) & (1 << 10);
}

bool MD80::setVelocityPidGains(float kp, float ki, float kd, float intLimit, bool useSDO)
{
	return writeAccessHelper(0x2001, 0x01, kp, useSDO) &&
		   writeAccessHelper(0x2001, 0x02, ki, useSDO) &&
		   writeAccessHelper(0x2001, 0x03, kd, useSDO) &&
		   writeAccessHelper(0x2001, 0x04, intLimit, useSDO);
}

bool MD80::setPositionPidGains(float kp, float ki, float kd, float intLimit, bool useSDO)
{
	return writeAccessHelper(0x2002, 0x01, kp, useSDO) &&
		   writeAccessHelper(0x2002, 0x02, ki, useSDO) &&
		   writeAccessHelper(0x2002, 0x03, kd, useSDO) &&
		   writeAccessHelper(0x2002, 0x04, intLimit, useSDO);
}

bool MD80::setImpedancePdGains(float kp, float kd, bool useSDO)
{
	return writeAccessHelper(0x200C, 0x01, kp, useSDO) &&
		   writeAccessHelper(0x200C, 0x02, kd, useSDO);
}

bool MD80::runRoutine(RoutineID routineId, bool shouldWaitForCompletion)
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

bool MD80::enterOperational()
{
	return writeAccessHelper(0x6040, 0x00, static_cast<uint16_t>(0x0080), true) &&
		   writeAccessHelper(0x6040, 0x00, static_cast<uint16_t>(0x0006), true) &&
		   writeAccessHelper(0x6040, 0x00, static_cast<uint16_t>(0x000f), true);
}

bool MD80::enterSwitchOnDisabled()
{
	return writeAccessHelper(0x6040, 0x00, static_cast<uint16_t>(0x0008), true);
}

bool MD80::setModeOfOperation(ModesOfOperation mode)
{
	return writeAccessHelper(0x6060, 0x00, static_cast<int8_t>(mode), true);
}

bool MD80::setupPDO(CanopenStack::PDO pdoID, const std::vector<std::pair<uint16_t, uint8_t>>& fields)
{
	return canopenStack->setupPDO(id, pdoID, fields);
}