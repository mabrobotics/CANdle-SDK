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
		md80s.push_back(candle.getMd80(id));
	}

	candle.setSendSync(true, 3000);

	while (1)
	{
		for (auto& md80 : md80s)
			auto a = md80->getOutputPosition();
	}

	return 0;
}
