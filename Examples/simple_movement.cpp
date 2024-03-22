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

	auto md80 = candle.getMd80(id);
	md80->setModeOfOperation(MD80::ModesOfOperation::CYCLIC_SYNC_POSITION);
	md80->runRoutine(MD80::RoutineID::SET_ZERO, true);
	md80->enterOperational();

	float x = 0.0f;

	while (1)
	{
		md80->setPositionTarget(6.0f * sin(x));
		std::cout << "Current position: " << md80->getOutputPosition() << std::endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		x += 0.01;
	}

	return 0;
}
