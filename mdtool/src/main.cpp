#include "mdtool.hpp"
#include "ui.hpp"
#include "CLI11.hpp"

int main(int argc, char** argv)
{
	CLI::App app{"mdtool"};
	app.fallthrough();
	app.ignore_case();
	UserCommand cmd;

	auto* ping	  = app.add_subcommand("ping", "Discover MD drives on CAN bus.");
	auto* config  = app.add_subcommand("config", "Configure CAN parameters of MD drive.");
	auto* setup	  = app.add_subcommand("setup", "Setup MD via config files, and calibrate.");
	auto* test	  = app.add_subcommand("test", "Basic MD drive testing routines.");
	auto* blink	  = app.add_subcommand("blink", "Blink LEDs on MD drive.");
	auto* encoder = app.add_subcommand("encoder", "Display MD rotor position.");
	auto* bus	  = app.add_subcommand("bus", "Set CANdle bus parameters.");
	// auto* registe = app.add_subcommand("register", "Access MD drive via register read/write.");
	auto* clear	  = app.add_subcommand("clear", "Clear Errors and Warnings.");
	auto* reset	  = app.add_subcommand("reset", "Reset MD drive.");

	// PING
	ping->add_option("<CAN BAUD>", cmd.variant, "Can be one of: 1M, 2M, 5M, 8M, all. Default: all");

	// CONFIG
	auto* configZero = config->add_subcommand("zero", "Set MD zero position at current position.");
	auto* configCan	 = config->add_subcommand("can", "Set CAN parameters of MD drive.");
	auto* configSave = config->add_subcommand("save", "Save current config to MD flash memory.");
	auto* configCurrent = config->add_subcommand("current");
	auto* configBand	= config->add_subcommand("bandwidth");
	configZero->add_option("<CAN ID>", cmd.id, "CAN ID of the MD to interact with.")->required();
	configCan->add_option("<CAN ID>", cmd.id, "Current CAN ID.")->required();
	configSave->add_option("<CAN ID>", cmd.id, "CAN ID of the MD to interact with.")->required();
	configCurrent->add_option("<CAN ID>", cmd.id, "CAN ID of the MD to interact with.")->required();
	configBand->add_option("<CAN ID>", cmd.id, "CAN ID of the MD to interact with.")->required();
	configCan->add_option("<NEW CAN ID>", cmd.newId, "New CAN ID to set.")->required();
	configCan->add_option("<CAN BAUD>", cmd.baud, "New CAN baudrate to set: 1M, 2M, 5M or 8M")
		->required();
	configCan->add_option("<CAN WDG>", cmd.canWatchdog, "New CAN watchdog timeout (in ms) to set.")
		->required();
	configCurrent->add_option("<AMPS>", cmd.current, "Max current in Amps.")->required();
	configBand->add_option("<HZ>", cmd.bandwidth, "Desired torque bandwidth.")->required();

	// SETUP
	auto* setupInfo		= setup->add_subcommand("info", "Display info about the MD drive.");
	auto* setupCalib	= setup->add_subcommand("calibration", "Calibrate main MD encoder.");
	auto* setupCalibOut = setup->add_subcommand("calibration_out", "Calibrate output encoder.");
	auto* setupHoming	= setup->add_subcommand("homing", "Begin homing procedure.");
	auto* setupMotor	= setup->add_subcommand("motor", "Upload actuator config from .cfg file.");
	setupInfo->add_option("<CAN ID>", cmd.id, "CAN ID of the MD to interact with.")->required();
	setupInfo->add_option("all", cmd.infoAll, "Print ALL available info.");
	setupCalib->add_option("<CAN ID>", cmd.id, "CAN ID of the MD to interact with.")->required();
	setupCalibOut->add_option("<CAN ID>", cmd.id, "CAN ID of the MD to interact with.")->required();
	setupHoming->add_option("<CAN ID>", cmd.id, "CAN ID of the MD to interact with.")->required();
	setupMotor->add_option("<CAN ID>", cmd.id, "CAN ID of the MD to interact with.")->required();
	setupMotor->add_option(
		"<.cfg FILENAME>",
		cmd.cfgPath,
		"Filename of motor config. By default, searches `~/.config/mdtool/mdtool_motors/`.");

	// TEST
	auto* testMove	  = test->add_subcommand("move", "Move motor relatie to current position.");
	auto* testLatency = test->add_subcommand(
		"latency",
		"Test max data exchange rate between your computer and all MD connected  drives.");
	auto* testEncoder = test->add_subcommand("encoder", "Perform accuracy test on encoder.");

	auto* testMoveAbs = testMove->add_subcommand("absolute", "Move motor to absolute position.");
	auto* testEncoderMain =
		testEncoder->add_subcommand("main", "Perfrom test routine on main encoder.");
	auto* testEncoderOut =
		testEncoder->add_subcommand("output", "Perfrom test routine on output encoder.");

	testEncoderMain->add_option("<CAN ID>", cmd.id, "CAN ID of the MD to interact with.")
		->required();
	testEncoderOut->add_option("<CAN ID>", cmd.id, "CAN ID of the MD to interact with.")
		->required();
	testLatency->add_option("<CAN BAUD>", cmd.baud, "New CAN baudrate to set: 1M, 2M, 5M or 8M")
		->required();
	testMove->add_option("<CAN ID>", cmd.id, "CAN ID of the MD to interact with.")->required();
	testMove->add_option("<REL POS>", cmd.pos, "Relative position to reach. Range: <-10, 10> [rad]")
		->required();
	testMoveAbs->add_option("<CAN ID>", cmd.id, "CAN ID of the MD to interact with.")->required();
	testMoveAbs->add_option("<ABS POS>", cmd.pos, "Absolute position to reach [rad].")->required();
	testMoveAbs->add_option("<MAX VEL>", cmd.vel, "Profile max velocity [rad/s].");
	testMoveAbs->add_option("<MAX ACC>", cmd.acc, "Profile max acceleration [rad/s^2].");
	testMoveAbs->add_option("<MAX DCC>", cmd.dcc, "Profile max deceleration [rad/s^2].");

	// BLINK
	blink->add_option("<CAN ID>", cmd.id, "CAN ID of the MD to interact with.")->required();

	// ENCODER
	encoder->add_option("<CAN ID>", cmd.id, "CAN ID of the MD to interact with.")->required();

	// BUS
	bus->add_option("<BUS>", cmd.bus, "Can be USB/SPI/UART. Only for CANdleHAT.")->required();
	bus->add_option("<DEVICE>", cmd.variant, "SPI/UART device to use, ie: /dev/spidev1.1.");

	// CLEAR 
	clear->add_option("<CAN ID>", cmd.id, "CAN ID of the MD to interact with.")->required();
	clear->add_option("<LEVEL>", cmd.variant, "Can be: warning, error, all")->required();
	
	// RESET 
	reset->add_option("<CAN ID>", cmd.id, "CAN ID of the MD to interact with.")->required();

	CLI11_PARSE(app, argc, argv);

	MDtool mdtool(cmd);
	if (app.count_all() == 1)
		std::cerr << app.help() << std::endl;

	if (ping->parsed())
		mdtool.ping(cmd.variant);
	if (config->parsed())
	{
		if (configCan->parsed())
			mdtool.configCan(cmd.id, cmd.newId, cmd.baud, cmd.canWatchdog);
		if (configZero->parsed())
			mdtool.configZero(cmd.id);
		if (configSave->parsed())
			mdtool.configSave(cmd.id);
		if (configCurrent->parsed())
			mdtool.configCurrent(cmd.id, cmd.current);
		if (configBand->parsed())
			mdtool.configBandwidth(cmd.id, cmd.bandwidth);
	}
	if (setup->parsed())
	{
		if (setupInfo->parsed())
			mdtool.setupInfo(cmd.id, cmd.infoAll);
		if (setupCalib->parsed())
			mdtool.setupCalibration(cmd.id);
		if (setupCalibOut->parsed())
			mdtool.setupCalibrationOutput(cmd.id);
		if (setupHoming->parsed())
			mdtool.setupHoming(cmd.id);
		if (setupMotor->parsed())
			mdtool.setupMotor(cmd.id, cmd.cfgPath);
	}
	if(test->parsed())
	{
		if(testMove->parsed())
			mdtool.testMove(cmd.id, cmd.pos);
	}
	if (blink->parsed())
		mdtool.blink(cmd.id);
	if (encoder->parsed())
		mdtool.encoder(cmd.id);
	if (bus->parsed())
		mdtool.bus(cmd.bus, cmd.busDevice);
	if (clear->parsed())
		mdtool.clearErrors(cmd.id, cmd.variant);
	if(reset->parsed())
		mdtool.reset(cmd.id);

	return EXIT_SUCCESS;
}
