#include "Candle.hpp"

int main(int argc, char** argv)
{
	const std::array<uint8_t, 2> ids{1, 2};

	const float kp = 0.5f;
	const float kd = 0.02f;

	Candle candle;

	if (!candle.init())
	{
		std::cout << "CANdle init failed!" << std::endl;
		return -1;
	}

	std::cout << "CANdle init successful!" << std::endl;

	for (auto id : ids)
	{
		candle.addMd80(id);
		candle.setupPDO(id, CanopenStack::PDO::TPDO1, {{0x2009, 0x01}, {0x2009, 0x03}});
		candle.setupPDO(id, CanopenStack::PDO::RPDO1, {{0x2008, 0x09}, {0x2008, 0x0B}});
		candle.setModeOfOperation(id, Candle::ModesOfOperation::IMPEDANCE);
		candle.writeSDO(id, 0x2003, 0x05, true);
		candle.writeSDO(id, 0x200C, 0x01, kp);
		candle.writeSDO(id, 0x200C, 0x02, kd);
		candle.enterOperational(id);
	}

	auto md80_1 = candle.getMd80(ids[0]);
	auto md80_2 = candle.getMd80(ids[1]);

	candle.setSendSync(true, 1000);

	while (1)
	{
		md80_1->setPositionTarget(0.0f);
		md80_2->setPositionTarget(0.0f);
	}

	return 0;
}
