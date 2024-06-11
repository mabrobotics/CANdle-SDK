#include "mdtool.hpp"
#include "ui.hpp"
#include "CLI11.hpp"

int main(int argc, char** argv)
{
	CLI::App app{"mdtool"};
	app.fallthrough();
	app.ignore_case();
	UserCommand cmd;

	auto* blink	  = app.add_subcommand("blink", "Blink LEDs on MD drive.");
	auto* bus	  = app.add_subcommand("bus", "Set CANdle bus parameters.");
	auto* clear	  = app.add_subcommand("clear", "Clear Errors and Warnings.");
	auto* config  = app.add_subcommand("config", "Configure CAN parameters of MD drive.");
	auto* encoder = app.add_subcommand("encoder", "Display MD rotor position.");
	auto* ping	  = app.add_subcommand("ping", "Discover MD drives on CAN bus.");
	auto* registr = app.add_subcommand("register", "Access MD drive via register read/write.");
	auto* reset	  = app.add_subcommand("reset", "Reset MD drive.");
	auto* setup	  = app.add_subcommand("setup", "Setup MD via config files, and calibrate.");
	auto* test	  = app.add_subcommand("test", "Basic MD drive testing routines.");

	// BLINK
	blink->add_option("<CAN ID>", cmd.id, "CAN ID of the MD to interact with.")->required();

	// BUS
	bus->add_option("<BUS>", cmd.bus, "Can be USB/SPI/UART. Only for CANdleHAT.")->required();
	bus->add_option("<DEVICE>", cmd.variant, "SPI/UART device to use, ie: /dev/spidev1.1.");

	// CLEAR
	clear->add_option("<CAN ID>", cmd.id, "CAN ID of the MD to interact with.")->required();
	clear->add_option("<LEVEL>", cmd.variant, "Can be: warning, error, all")->required();

	// CONFIG
	auto* configBand = config->add_subcommand(
		"bandwidth", "Set the torque bandwidth without recalibrating the actuator");
	auto* configCan		= config->add_subcommand("can", "Set CAN parameters of MD drive.");
	auto* configClear	= config->add_subcommand("clear");
	auto* configCurrent = config->add_subcommand(
		"current", "Set the maximum phase current that is allowed to flow through the motor.");
	auto* configSave = config->add_subcommand("save", "Save current config to MD flash memory.");
	auto* configZero = config->add_subcommand("zero", "Set MD zero position at current position.");

	configBand->add_option("<CAN ID>", cmd.id, "CAN ID of the MD to interact with.")->required();
	configBand->add_option("<HZ>", cmd.bandwidth, "Desired torque bandwidth.")->required();
	configCan->add_option("<CAN ID>", cmd.id, "Current CAN ID.")->required();
	configCan->add_option("<NEW CAN ID>", cmd.newId, "New CAN ID to set.")->required();
	configCan->add_option("<CAN BAUD>", cmd.baud, "New CAN baudrate to set: 1M, 2M, 5M or 8M")
		->required();
	configCan->add_option("<CAN WDG>", cmd.canWatchdog, "New CAN watchdog timeout (in ms) to set.")
		->required();
	configClear->add_option("<CAN ID>", cmd.id, "CAN ID of the MD to interact with.")->required();
	configCurrent->add_option("<CAN ID>", cmd.id, "CAN ID of the MD to interact with.")->required();
	configCurrent->add_option("<AMPS>", cmd.current, "Max current in Amps.")->required();
	configSave->add_option("<CAN ID>", cmd.id, "CAN ID of the MD to interact with.")->required();
	configZero->add_option("<CAN ID>", cmd.id, "CAN ID of the MD to interact with.")->required();

	// ENCODER
	encoder->add_option("<CAN ID>", cmd.id, "CAN ID of the MD to interact with.")->required();

	// PING
	ping->add_option("<CAN BAUD>", cmd.variant, "Can be one of: 1M, 2M, 5M, 8M, all. Default: all");

	// REGISTER
	auto* regRead  = registr->add_subcommand("read", "Read MD register.");
	auto* regWrite = registr->add_subcommand("write", "Write MD register.");

	regRead->add_option("<CAN ID>", cmd.id, "CAN ID of the MD to interact with.")->required();
	regRead->add_option("<REG ID>", cmd.reg, "Register ID (offset) to read data from.")->required();
	regWrite->add_option("<CAN ID>", cmd.id, "CAN ID of the MD to interact with.")->required();
	regWrite->add_option("<REG ID>", cmd.reg, "Register ID (offset) to write data to.")->required();
	regWrite->add_option("<VALUE>", cmd.value, "Value to write")->required();

	// RESET
	reset->add_option("<CAN ID>", cmd.id, "CAN ID of the MD to interact with.")->required();

	// SETUP
	auto* setupCalib	= setup->add_subcommand("calibration", "Calibrate main MD encoder.");
	auto* setupCalibOut = setup->add_subcommand("calibration_out", "Calibrate output encoder.");
	auto* setupHoming	= setup->add_subcommand("homing", "Begin homing procedure.");
	auto* setupInfo		= setup->add_subcommand("info", "Display info about the MD drive.");
	auto* setupMotor	= setup->add_subcommand("motor", "Upload actuator config from .cfg file.");
	auto* setupReadCfg =
		setup->add_subcommand("read_config", "Download actuator config from MD to .cfg file.");
	auto* setupInfoAllFlag = setupInfo->add_flag("-a", "Print ALL available info.");

	setupCalib->add_option("<CAN ID>", cmd.id, "CAN ID of the MD to interact with.")->required();
	setupCalibOut->add_option("<CAN ID>", cmd.id, "CAN ID of the MD to interact with.")->required();
	setupHoming->add_option("<CAN ID>", cmd.id, "CAN ID of the MD to interact with.")->required();
	setupInfo->add_option("<CAN ID>", cmd.id, "CAN ID of the MD to interact with.")->required();
	setupMotor->add_option("<CAN ID>", cmd.id, "CAN ID of the MD to interact with.")->required();
	setupMotor->add_option(
		"<.cfg FILENAME>",
		cmd.cfgPath,
		"Filename of motor config. By default, searches `~/.config/mdtool/mdtool_motors/`.");
	setupReadCfg->add_option("<CAN ID>", cmd.id, "CAN ID of the MD to interact with.")->required();
	setupReadCfg->add_option("<FILE>", cmd.value, "File to save config to.");

	// TEST
	auto* testEncoder = test->add_subcommand("encoder", "Perform accuracy test on encoder.");
	auto* testEncoderMain =
		testEncoder->add_subcommand("main", "Perfrom test routine on main encoder.");
	auto* testEncoderOut =
		testEncoder->add_subcommand("output", "Perfrom test routine on output encoder.");
	auto* testLatency = test->add_subcommand(
		"latency",
		"Test max data exchange rate between your computer and all MD connected  drives.");
	auto* testMove	  = test->add_subcommand("move", "Validate if motor can move.");
	auto* testMoveAbs = testMove->add_subcommand("absolute", "Move motor to absolute position.");
	auto* testMoveRel = testMove->add_subcommand("relative", "Move motor to relative position.");

	testEncoderMain->add_option("<CAN ID>", cmd.id, "CAN ID of the MD to interact with.")
		->required();
	testEncoderOut->add_option("<CAN ID>", cmd.id, "CAN ID of the MD to interact with.")
		->required();
	testLatency->add_option("<CAN BAUD>", cmd.baud, "New CAN baudrate to set: 1M, 2M, 5M or 8M")
		->required();
	testMoveAbs->add_option("<CAN ID>", cmd.id, "CAN ID of the MD to interact with.")->required();
	testMoveAbs->add_option("<ABS POS>", cmd.pos, "Absolute position to reach [rad].")->required();
	testMoveAbs->add_option("<MAX VEL>", cmd.vel, "Profile max velocity [rad/s].");
	testMoveAbs->add_option("<MAX ACC>", cmd.acc, "Profile max acceleration [rad/s^2].");
	testMoveAbs->add_option("<MAX DCC>", cmd.dcc, "Profile max deceleration [rad/s^2].");
	testMoveRel->add_option("<CAN ID>", cmd.id, "CAN ID of the MD to interact with.")->required();
	testMoveRel->add_option("<REL POS>", cmd.pos, "Relative position to reach. <-10, 10>[rad]");

	CLI11_PARSE(app, argc, argv);

	MDtool mdtool;
	if (app.count_all() == 1)
		std::cerr << app.help() << std::endl;

	if (blink->parsed())
		mdtool.blink(cmd.id);
	if (bus->parsed())
		mdtool.bus(cmd.bus, cmd.variant);
	if (clear->parsed())
		mdtool.clearErrors(cmd.id, cmd.variant);
	if (config->parsed())
	{
		if (configBand->parsed())
			mdtool.configBandwidth(cmd.id, cmd.bandwidth);
		if (configCan->parsed())
			mdtool.configCan(cmd.id, cmd.newId, cmd.baud, cmd.canWatchdog);
		if (configClear->parsed())
			mdtool.configClear(cmd.id);
		if (configCurrent->parsed())
			mdtool.configCurrent(cmd.id, cmd.current);
		if (configSave->parsed())
			mdtool.configSave(cmd.id);
		if (configZero->parsed())
			mdtool.configZero(cmd.id);
	}
	if (encoder->parsed())
		mdtool.encoder(cmd.id);
	if (ping->parsed())
		mdtool.ping(cmd.variant);
	if (registr->parsed())
	{
		u16 reg = strtoul(cmd.reg.c_str(), nullptr, 16);
		if (regRead->parsed())
			mdtool.registerRead(cmd.id, reg);
		if (regWrite->parsed())
			mdtool.registerWrite(cmd.id, reg, cmd.value);
	}
	if (reset->parsed())
		mdtool.reset(cmd.id);
	if (setup->parsed())
	{
		if (setupCalib->parsed())
			mdtool.setupCalibration(cmd.id);
		if (setupCalibOut->parsed())
			mdtool.setupCalibrationOutput(cmd.id);
		if (setupHoming->parsed())
			mdtool.setupHoming(cmd.id);
		if (setupInfo->parsed())
			mdtool.setupInfo(cmd.id, (setupInfoAllFlag->count() > 0 ? true : false));
		if (setupMotor->parsed())
			mdtool.setupMotor(cmd.id, cmd.cfgPath);
		if (setupReadCfg->parsed())
			mdtool.setupReadConfig(cmd.id, cmd.value);
	}
	if (test->parsed())
	{
		if (testEncoderMain->parsed())
			mdtool.testEncoderMain(cmd.id);
		if (testEncoderOut->parsed())
			mdtool.testEncoderOutput(cmd.id);
		if (testLatency->parsed())
			mdtool.testLatency(cmd.baud);
		if (testMoveAbs->parsed())
			mdtool.testMoveAbsolute(cmd.id, cmd.pos, cmd.vel, cmd.acc, cmd.dcc);
		if (testMoveRel->parsed())
			mdtool.testMove(cmd.id, cmd.pos);
	}

	return EXIT_SUCCESS;
}
