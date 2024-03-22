#include "Candle.hpp"

int main(int argc, char** argv)
{
	const std::array<uint8_t, 2> ids{1, 2};

	const float kp = 0.5f;
	const float kd = 0.02f;

	Candle candle;

	if (!candle.init(Candle::Baud::BAUD_1M))
	{
		std::cout << "CANdle init failed!" << std::endl;
		return -1;
	}

	std::cout << "CANdle init successful!" << std::endl;

	for (auto id : ids)
	{
		candle.addMd80(id);
		auto md80 = candle.getMd80(id);
		md80->setupPDO(CanopenStack::PDO::TPDO1, {{0x2009, 0x01}, {0x2009, 0x03}, {0x2009, 0x02}});
		md80->setupPDO(CanopenStack::PDO::RPDO1, {{0x2008, 0x09}, {0x2008, 0x0B}, {0x2008, 0x0A}});
		md80->setModeOfOperation(MD80::ModesOfOperation::CYCLIC_SYNC_POSITION);
		candle.writeSDO(id, 0x2003, 0x05, true);
		candle.writeSDO(id, 0x200C, 0x01, kp);
		candle.writeSDO(id, 0x200C, 0x02, kd);
		md80->runRoutine(MD80::RoutineID::SET_ZERO, true);
		md80->enterOperational();
	}

	auto md80_1 = candle.getMd80(ids[0]);
	auto md80_2 = candle.getMd80(ids[1]);

	candle.setSendSync(true, 3000);

	float x = 0.0f;

	while (1)
	{
		float target = 5.0f * sin(x);
		md80_1->setPositionTarget(target);
		md80_2->setPositionTarget(target);

		std::cout << md80_1->getOutputPosition() << " " << md80_1->getOutputVelocity() << " " << md80_1->getOutputTorque() << std::endl;

		x += 0.01f;
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	return 0;
}
