#include "mdtool.hpp"
#include "ui.hpp"
#include "CLI11.hpp"

int main(int argc, char** argv)
{
	CLI::App app{"mdtool"};
	app.fallthrough();
	app.ignore_case();

	auto* ping	 = app.add_subcommand("ping", "Discover MD drives on CAN bus.");
	auto* config = app.add_subcommand("config", "Configure CAN parameters of MD drive.");
	auto* setup	 = app.add_subcommand("setup", "Setup MD via config files, and calibrate.");
	// auto* test	  = app.add_subcommand("test", "Basic MD drive testing routines.");
	// auto* blink	  = app.add_subcommand("blink", "Blink LEDs on MD drive.");
	// auto* encoder = app.add_subcommand("encoder", "Display MD rotor position.");
	// auto* bus	  = app.add_subcommand("bus", "Set CANdle bus parameters.");
	// auto* registe = app.add_subcommand("register", "Access MD drive via register read/write.");
	// auto* clear	  = app.add_subcommand("clear", "Clear Errors and Warnings.");
	// auto* reset	  = app.add_subcommand("reset", "Reset MD drive.");

	// PING
	std::string pingVariant = "all";
	ping->add_option("<CAN BAUD>", pingVariant, "Can be one of: 1M, 2M, 5M, 8M, all. Default: all");

	// CONFIG
	u32			id			= 0;
	u32			newId		= 0;
	std::string canBaud		= "DC";
	u32			canWatchdog = 100;
	f32			current		= 1.f;
	f32			bandwidth	= 100.f;
	auto* configZero = config->add_subcommand("zero", "Set MD zero position at current position.");
	configZero->add_option("<CAN ID>", id, "CAN ID of the MD to interact with.")->required();
	auto* configCan = config->add_subcommand("can", "Set CAN parameters of MD drive.");
	configCan->add_option("<CAN ID>", id, "Current CAN ID.")->required();
	configCan->add_option("<NEW CAN ID>", newId, "New CAN ID to set.")->required();
	configCan->add_option("<CAN BAUD>", canBaud, "New CAN baudrate to set.")->required();
	configCan->add_option("<CAN WDG>", canWatchdog, "New CAN watchdog timeout (in ms) to set.")
		->required();
	auto* configSave = config->add_subcommand("save", "Save current config to MD flash memory.");
	configSave->add_option("<CAN ID>", id, "CAN ID of the MD to interact with.")->required();
	auto* configCurrent = config->add_subcommand("current");
	configCurrent->add_option("<CAN ID>", id, "CAN ID of the MD to interact with.")->required();
	configCurrent->add_option("<AMPS>", current, "Max current in Amps.")->required();
	auto* configBandwidth = config->add_subcommand("bandwidth");
	configBandwidth->add_option("<CAN ID>", id, "CAN ID of the MD to interact with.")->required();
	configBandwidth->add_option("<HZ>", bandwidth, "Desired torque bandwidth.")->required();

	// SETUP
	std::string configPath;
	auto* setupInfo		= setup->add_subcommand("info", "Display info about the MD drive.");
	auto* setupCalib	= setup->add_subcommand("calibrate", "Calibrate main MD encoder.");
	auto* setupCalibOut = setup->add_subcommand("calibrate_out", "Calibrate output encoder.");
	auto* setupMotor	= setup->add_subcommand("motor", "Upload actuator config from .cfg file.");
	auto* setupHoming	= setup->add_subcommand("homing", "Begin homing procedure.");
	setupInfo->add_option("<CAN ID>", id, "CAN ID of the MD to interact with.")->required();
	setupCalib->add_option("<CAN ID>", id, "CAN ID of the MD to interact with.")->required();
	setupCalibOut->add_option("<CAN ID>", id, "CAN ID of the MD to interact with.")->required();
	setupMotor->add_option("<CAN ID>", id, "CAN ID of the MD to interact with.")->required();
	setupHoming->add_option("<CAN ID>", id, "CAN ID of the MD to interact with.")->required();
	setupMotor->add_option(
		"<.cfg PATH>",
		configPath,
		"Path to motor config. By default, searches ~/.config/mdtool/mdtool_motors/")

		CLI11_PARSE(app, argc, argv);

	return 0;

	std::vector<std::string> args;
	for (int i = 0; i < argc; i++)
		args.emplace_back(argv[i]);

	MDtool program(args);

	return EXIT_SUCCESS;
}
