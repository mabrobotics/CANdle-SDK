#include "Candle.hpp"

int main(int argc, char** argv)
{
	Candle candle;

	if (!candle.init(Candle::Baud::BAUD_8M))
	{
		std::cout << "CANdle init failed!" << std::endl;
		return -1;
	}

	std::cout << "CANdle init successful!" << std::endl;

	auto ids = candle.ping();

	std::vector<std::shared_ptr<MD80>> md80s;

	for (auto id : ids)
	{
		candle.addMd80(id);
		candle.setupPDO(id, CanopenStack::PDO::TPDO1, {{0x2009, 0x01}, {0x2009, 0x03}});
		candle.setupPDO(id, CanopenStack::PDO::RPDO1, {{0x2008, 0x09}, {0x2008, 0x0B}});
		candle.setModeOfOperation(id, Candle::ModesOfOperation::IMPEDANCE);
		candle.writeSDO(id, 0x2003, 0x05, true);
		candle.setZeroPosition(id);
		candle.enterOperational(id);
	}

	for (int i = 1; i < ids.size() / 2; i++)
	{
		md80s.push_back(candle.getMd80(i));
		md80s.push_back(candle.getMd80(i + 6));
	}
	// auto md80_1 = candle.getMd80(ids[0]);
	// auto md80_2 = candle.getMd80(ids[1]);

	candle.setSendSync(true, 1500);

	while (1)
	{
		// md80_1->setPositionTarget(md80_2->getOutputPosition());
		// md80_2->setPositionTarget(md80_1->getOutputPosition());

		for (auto& md80 : md80s)
		{
			auto a = md80->getOutputPosition();
		}
	}

	return 0;
}
