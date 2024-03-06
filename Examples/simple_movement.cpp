#include "Candle.hpp"

int main(int argc, char** argv)
{
	const uint32_t id = 2;

	Candle candle;

	if (!candle.init())
		return -1;

	std::cout << "CANdle init successful!" << std::endl;

	if (!candle.addMd80(id))
		return -1;

	std::cout << "Added MD80 succesfully!" << std::endl;

	candle.setModeOfOperation(id, Candle::ModesOfOperation::CYCLIC_SYNC_POSITION);
	candle.setZeroPosition(id);
	candle.enterOperational(id);

	auto md80 = candle.getMd80(id);

	float x = 0.0f;
	float readPos = 0.0f;

	while (1)
	{
		candle.writeSDO(id, 0x607A, 0x0, (int32_t)(10000.0f * sin(x)));
		candle.readSDO(id, 0x2009, 0x01, readPos);
		std::cout << "Current position: " << readPos << std::endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		x += 0.01;
	}

	return 0;
}
