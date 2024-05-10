#include "Candle.hpp"

int main(int argc, char** argv)
{
	Candle candle;
	Candle candle1;

	if (!candle.init(Candle::Baud::BAUD_8M))
	{
		std::cout << "CANdle init failed!" << std::endl;
		return -1;
	}
	if (!candle1.init(Candle::Baud::BAUD_8M))
	{
		std::cout << "CANdle1 init failed!" << std::endl;
		return -1;
	}

	std::cout << "CANdle init successful!" << std::endl;

	auto ids = candle.ping();

	for (auto& id : ids)
	{
		candle.addMd80(id);
		auto md80 = candle.getMd80(id);
		md80->setupPDO(CanopenStack::PDO::TPDO1, {{0x2009, 0x01}, {0x2009, 0x03}});
		md80->setupPDO(CanopenStack::PDO::RPDO1, {{0x2008, 0x09}, {0x2008, 0x0B}});
		md80->setModeOfOperation(MD80::ModesOfOperation::IMPEDANCE);
		md80->runRoutine(MD80::RoutineID::SET_ZERO, true);
		md80->enterOperational();
	}

	auto ids1 = candle1.ping();

	for (auto& id : ids1)
	{
		candle.addMd80(id);
		auto md80 = candle.getMd80(id);
		md80->setupPDO(CanopenStack::PDO::TPDO1, {{0x2009, 0x01}, {0x2009, 0x03}});
		md80->setupPDO(CanopenStack::PDO::RPDO1, {{0x2008, 0x09}, {0x2008, 0x0B}});
		md80->setModeOfOperation(MD80::ModesOfOperation::IMPEDANCE);
		md80->runRoutine(MD80::RoutineID::SET_ZERO, true);
		md80->enterOperational();
	}

	auto md80_1 = candle.getMd80(ids[0]);
	auto md80_2 = candle1.getMd80(ids1[0]);

	candle.setSendSync(true, 2000);
	candle1.setSendSync(true, 2000);

	while (1)
	{
		md80_1->setPositionTarget(md80_2->getOutputPosition());
		md80_2->setPositionTarget(md80_1->getOutputPosition());
	}

	return 0;
}
