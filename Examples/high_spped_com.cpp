#include "Candle.hpp"

int main(int argc, char** argv)
{
	const uint32_t id = 1;

	Candle candle;

	if (!candle.init())
		return -1;

	candle.addMd80(id);
	// candle.addMd80(2);

	std::vector<std::pair<uint16_t, uint8_t>> TPDO{{0x2009, 0x01}, {0x2009, 0x02}};
	candle.setupResponse(id, CanopenStack::PDO::TPDO1, TPDO);
	// candle.setupResponse(2, CanopenStack::PDO::TPDO1, TPDO);

	std::vector<std::pair<uint16_t, uint8_t>> RPDO{{0x2008, 0x09}, {0x2008, 0x0A}};
	candle.setupResponse(id, CanopenStack::PDO::RPDO1, RPDO);
	// candle.setupResponse(2, CanopenStack::PDO::RPDO1, RPDO);

	candle.setModeOfOperation(id, Candle::ModesOfOperation::CYCLIC_SYNC_POSITION);
	candle.enterOperational(id);

	auto md80 = candle.getMd80(id);

	candle.setSendSync(true, 2000);

	float x = 0;

	while (1)
	{
		md80->setPositionTarget(5 * sin(x));
		std::cout << md80->getOutputPosition() << "   " << md80->getOutputVelocity() << " setpoint pos: " << 20 * sin(x) << std::endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(10));

		x += 0.01;
	}

	return 0;
}
