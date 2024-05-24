#include "mainWorker.hpp"
#include "ui.hpp"

int main(int argc, char** argv)
{
	std::vector<std::string> args;
	for (int i = 0; i < argc; i++)
		args.emplace_back(argv[i]);

	MainWorker program(args);

	return EXIT_SUCCESS;
}