#include "Candle.hpp"

int main(int argc, char** argv)
{
	const std::array<uint8_t, 2> ids{1, 2};

	Candle candle;
	Candle candle1;

	if (!candle.init())
	{
		std::cout << "CANdle init failed!" << std::endl;
		return -1;
	}
	if (!candle1.init())
	{
		std::cout << "CANdle1 init failed!" << std::endl;
		return -1;
	}

	std::cout << "CANdle init successful!" << std::endl;

	auto id = ids[0];
	candle.addMd80(id);
	candle.setupPDO(id, CanopenStack::PDO::TPDO1, {{0x2009, 0x01}, {0x2009, 0x03}});
	candle.setupPDO(id, CanopenStack::PDO::RPDO1, {{0x2008, 0x09}, {0x2008, 0x0B}});
	candle.setModeOfOperation(id, Candle::ModesOfOperation::IMPEDANCE);
	candle.setZeroPosition(id);
	candle.enterOperational(id);

	id = ids[1];
	candle1.addMd80(id);
	candle1.setupPDO(id, CanopenStack::PDO::TPDO1, {{0x2009, 0x01}, {0x2009, 0x03}});
	candle1.setupPDO(id, CanopenStack::PDO::RPDO1, {{0x2008, 0x09}, {0x2008, 0x0B}});
	candle1.setModeOfOperation(id, Candle::ModesOfOperation::IMPEDANCE);
	candle1.setZeroPosition(id);
	candle1.enterOperational(id);

	auto md80_1 = candle.getMd80(ids[0]);
	auto md80_2 = candle1.getMd80(ids[1]);

	candle.setSendSync(true, 1000);
	candle1.setSendSync(true, 1000);

	while (1)
	{
		md80_1->setPositionTarget(md80_2->getOutputPosition());
		md80_2->setPositionTarget(md80_1->getOutputPosition());
	}

	return 0;
}
