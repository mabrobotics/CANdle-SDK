#include "Candle.hpp"

int main(int argc, char** argv)
{
	const float kp = 0.5f;
	const float kd = 0.02f;
	std::vector<std::shared_ptr<MD80>> md80s;

	Candle candle;

	if (!candle.init(Candle::Baud::BAUD_1M))
	{
		std::cout << "CANdle init failed!" << std::endl;
		return -1;
	}

	auto ids = candle.ping();

	std::cout << "CANdle init successful!" << std::endl;

	for (auto id : ids)
	{
		candle.addMd80(id);
		auto md80 = candle.getMd80(id);
		md80->setupPDO(CanopenStack::PDO::TPDO1, {{0x2009, 0x01}, {0x2009, 0x03}, {0x2009, 0x02}});
		md80->setupPDO(CanopenStack::PDO::RPDO1, {{0x2008, 0x09}, {0x2008, 0x0B}, {0x2008, 0x0A}});
		md80->setModeOfOperation(MD80::ModesOfOperation::CYCLIC_SYNC_POSITION);
		md80->setImpedancePdGains(kp, kd);
		md80->runRoutine(MD80::RoutineID::SET_ZERO, true);
		md80->enterOperational();
		md80s.push_back(md80);
	}

	candle.setSendSync(true, 3000);

	float x = 0.0f;

	while (1)
	{
		float target = 5.0f * sin(x);

		for (auto& md80 : md80s)
			md80->setPositionTarget(target, false);

		std::cout << md80s[0]->getOutputPosition() << " " << md80s[0]->getOutputVelocity() << " " << md80s[0]->getOutputTorque() << std::endl;

		x += 0.01f;
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	return 0;
}
