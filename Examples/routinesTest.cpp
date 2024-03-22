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

	candle.enterOperational(id);

	auto md80 = candle.getMd80(id);

	md80->runRoutine(MD80::RoutineID::BLINK, true);

	return 0;
}
