#include "Candle.hpp"

int main(int argc, char** argv)
{
	const uint32_t id = 1;

	Candle candle;

	if (!candle.init())
		return -1;

	std::cout << "CANdle init successful!" << std::endl;

	if (!candle.addMd80(id))
		return -1;

	auto md80 = candle.getMd80(id);
	md80->setupPDO(CanopenStack::PDO::TPDO1, {{0x2009, 0x01}, {0x2009, 0x02}});
	md80->setupPDO(CanopenStack::PDO::RPDO1, {{0x2008, 0x09}, {0x2008, 0x0A}});
	md80->setModeOfOperation(MD80::ModesOfOperation::IMPEDANCE);
	md80->runRoutine(MD80::RoutineID::SET_ZERO, true);
	md80->enterOperational();

	candle.setSendSync(true, 2000);

	float x = 0;

	while (1)
	{
		md80->setPositionTarget(5 * sin(x), false);
		std::cout << md80->getOutputPosition(false) << "   " << md80->getOutputVelocity(false) << " setpoint pos: " << 20 * sin(x) << std::endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		x += 0.01;
	}

	return 0;
}
