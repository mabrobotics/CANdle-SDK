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

	std::cout << "Added MD80 succesfully!" << std::endl;

	candle.addMd80(id);
	auto md80 = candle.getMd80(id);
	md80->setupPDO(CanopenStack::PDO::TPDO1, {{0x2009, 0x01}, {0x2009, 0x02}});
	md80->setupPDO(CanopenStack::PDO::RPDO1, {{0x2008, 0x09}, {0x2008, 0x0A}});
	md80->setModeOfOperation(MD80::ModesOfOperation::CYCLIC_SYNC_VELOCTIY);
	md80->runRoutine(MD80::RoutineID::SET_ZERO, true);
	md80->enterOperational();

	candle.setSendSync(true, 2000);

	float x = 0;

	while (1)
	{
		md80->setVelocityTarget(50 * sin(x));
		std::cout << md80->getOutputVelocity() << " setpoint velocity: " << 50 * sin(x) << std::endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		x += 0.01;
	}

	return 0;
}
