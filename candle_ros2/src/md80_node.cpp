#include "md80_node.hpp"

const std::string version = "v1.3.2";

Md80Node::Md80Node(int argc, char** argv) : Node("candle_ros2_node")
{
	if (argc < 2 || argc > 4)
	{
		std::cout << "Wrong arguments specified, please see candle_ros candle_ros_node --help" << std::endl;
		return;
	}

	if (strcmp(argv[1], "--help") == 0)
	{
		std::cout << "usage: candle_ros candle_ros_node <bus> <baud> [<device>] [--help]" << std::endl;
		std::cout << "<bus> can be SPI/USB/UART" << std::endl;
		std::cout << "<baud> can be 1M/2M/5M/8M" << std::endl;
		std::cout << "<device> SPI or UART device if default is not suitable" << std::endl;
		std::cout << "[--help] - displays help message" << std::endl;
		return;
	}

	mab::BusType_E bus = mab::BusType_E::USB;
	mab::CANdleBaudrate_E baud = mab::CAN_BAUD_1M;

	if (strcmp(argv[1], "SPI") == 0)
		bus = mab::BusType_E::SPI;
	else if (strcmp(argv[1], "USB") == 0)
		bus = mab::BusType_E::USB;
	else if (strcmp(argv[1], "UART") == 0)
		bus = mab::BusType_E::UART;
	else
	{
		std::cout << "bus parameter not recognised!" << std::endl;
		return;
	}

	if (strcmp(argv[2], "1M") == 0)
		baud = mab::CAN_BAUD_1M;
	else if (strcmp(argv[2], "2M") == 0)
		baud = mab::CAN_BAUD_2M;
	else if (strcmp(argv[2], "5M") == 0)
		baud = mab::CAN_BAUD_5M;
	else if (strcmp(argv[2], "8M") == 0)
		baud = mab::CAN_BAUD_8M;
	else
	{
		std::cout << "baud parameter not recognised!" << std::endl;
		return;
	}

	while (bus == mab::BusType_E::USB)
	{
		try
		{
			auto candle = new mab::Candle(baud, true, bus);
			std::cout << "[CANDLE] Found CANdle with ID: " << candle->getDeviceId() << std::endl;
			candleInstances.push_back(candle);
		}
		catch (const char* eMsg)
		{
			break;
		}
	}

	if (bus != mab::BusType_E::USB)
	{
		mab::Candle* candle = nullptr;

		if (argc == 4 && argv[3] != 0)
			candle = new mab::Candle(baud, true, bus, argv[3]);
		else
			candle = new mab::Candle(baud, true, bus);

		candleInstances.push_back(candle);
	}

	addMd80Service = this->create_service<candle_ros2::srv::AddMd80s>(this->get_name() + std::string("/add_md80s"),
																	  std::bind(&Md80Node::service_addMd80, this, std::placeholders::_1, std::placeholders::_2));
	zeroMd80Service = this->create_service<candle_ros2::srv::GenericMd80Msg>(this->get_name() + std::string("/zero_md80s"),
																			 std::bind(&Md80Node::service_zeroMd80, this, std::placeholders::_1, std::placeholders::_2));
	setModeMd80Service = this->create_service<candle_ros2::srv::SetModeMd80s>(this->get_name() + std::string("/set_mode_md80s"),
																			  std::bind(&Md80Node::service_setModeMd80, this, std::placeholders::_1, std::placeholders::_2));
	enableMd80Service = this->create_service<candle_ros2::srv::GenericMd80Msg>(this->get_name() + std::string("/enable_md80s"),
																			   std::bind(&Md80Node::service_enableMd80, this, std::placeholders::_1, std::placeholders::_2));
	disableMd80Service = this->create_service<candle_ros2::srv::GenericMd80Msg>(this->get_name() + std::string("/disable_md80s"),
																				std::bind(&Md80Node::service_disableMd80, this, std::placeholders::_1, std::placeholders::_2));

	motionCommandSub = this->create_subscription<candle_ros2::msg::MotionCommand>("md80/motion_command", 10,
																				  std::bind(&Md80Node::motionCommandCallback, this, std::placeholders::_1));
	impedanceCommandSub = this->create_subscription<candle_ros2::msg::ImpedanceCommand>("md80/impedance_command", 10,
																						std::bind(&Md80Node::impedanceCommandCallback, this, std::placeholders::_1));
	velocityCommandSub = this->create_subscription<candle_ros2::msg::VelocityPidCommand>("md80/velocity_pid_command", 10,
																						 std::bind(&Md80Node::velocityCommandCallback, this, std::placeholders::_1));
	positionCommandSub = this->create_subscription<candle_ros2::msg::PositionPidCommand>("md80/position_pid_command", 10,
																						 std::bind(&Md80Node::positionCommandCallback, this, std::placeholders::_1));

	jointStatePub = this->create_publisher<sensor_msgs::msg::JointState>("md80/joint_states", 10);
	pubTimer = this->create_wall_timer(std::chrono::milliseconds(10), std::bind(&Md80Node::publishJointStates, this));
	pubTimer->cancel();
	RCLCPP_INFO(this->get_logger(), "candle_ros2_node %s has started.", version.c_str());
}
Md80Node::~Md80Node()
{
	for (auto candle : candleInstances)
		delete candle;
	RCLCPP_INFO(this->get_logger(), "candle_ros2_node finished.");
}

mab::Candle* Md80Node::findCandleByMd80Id(uint16_t md80Id)
{
	for (auto candle : candleInstances)
	{
		for (auto id : candle->md80s)
		{
			if (id.getId() == md80Id) return candle;
		}
	}
	return NULL;
}

void Md80Node::service_addMd80(const std::shared_ptr<candle_ros2::srv::AddMd80s::Request> request,
							   std::shared_ptr<candle_ros2::srv::AddMd80s::Response> response)
{
	for (auto& id : request->drive_ids)
	{
		unsigned int md80NotFound = 0;
		unsigned int md80Found = 0;

		for (auto candle : candleInstances)
		{
			if (candle->addMd80(id, false) == true)
			{
				response->drives_success.push_back(true);
				md80Found++;
			}
			else
				md80NotFound++;
		}
		/* if the id was found on multiple CANdle devices */
		if (md80Found > 1)
			RCLCPP_WARN(this->get_logger(), "Drive with ID %d seem to be duplicated", id);
		/* if the drive was not found on any of CANdle devices */
		else if (md80NotFound == candleInstances.size())
			response->drives_success.push_back(false);
	}

	int totalNumberOfDrives = 0;

	/* collect total number of drives from all CANdle devices */
	for (auto candle : candleInstances)
		totalNumberOfDrives += candle->md80s.size();

	response->total_number_of_drives = totalNumberOfDrives;
}
void Md80Node::service_zeroMd80(const std::shared_ptr<candle_ros2::srv::GenericMd80Msg::Request> request,
								std::shared_ptr<candle_ros2::srv::GenericMd80Msg::Response> response)
{
	for (auto& id : request->drive_ids)
	{
		auto candle = findCandleByMd80Id(id);
		if (candle != NULL)
			response->drives_success.push_back(candle->controlMd80SetEncoderZero(id));
		else
		{
			response->drives_success.push_back(false);
			RCLCPP_WARN(this->get_logger(), "Drive with ID: %d is not added!", id);
		}
	}
}
void Md80Node::service_setModeMd80(const std::shared_ptr<candle_ros2::srv::SetModeMd80s::Request> request,
								   std::shared_ptr<candle_ros2::srv::SetModeMd80s::Response> response)
{
	if (request->drive_ids.size() != request->mode.size())
	{
		for (auto& id : request->drive_ids)
		{
			(void)id;
			response->drives_success.push_back(false);
		}
		RCLCPP_WARN(this->get_logger(), "SetMode request incomplete. Sizes of arrays do not match!");
		return;
	}
	for (int i = 0; i < (int)request->drive_ids.size(); i++)
	{
		mab::Md80Mode_E mode;

		if (request->mode[i] == "IMPEDANCE")
			mode = mab::Md80Mode_E::IMPEDANCE;
		else if (request->mode[i] == "POSITION_PID")
			mode = mab::Md80Mode_E::POSITION_PID;
		else if (request->mode[i] == "VELOCITY_PID")
			mode = mab::Md80Mode_E::VELOCITY_PID;
		else if (request->mode[i] == "RAW_TORQUE")
			mode = mab::Md80Mode_E::RAW_TORQUE;
		else
		{
			RCLCPP_WARN(this->get_logger(), "MODE %s not recognized, setting IDLE for driveID = %d", request->mode[i].c_str(), request->drive_ids[i]);
			mode = mab::Md80Mode_E::IDLE;
		}
		auto candle = findCandleByMd80Id(request->drive_ids[i]);
		if (candle != NULL)
			response->drives_success.push_back(candle->controlMd80Mode(request->drive_ids[i], mode));
		else
		{
			response->drives_success.push_back(false);
			RCLCPP_WARN(this->get_logger(), "Drive with ID: %d is not added!", request->drive_ids[i]);
		}
	}
}
void Md80Node::service_enableMd80(const std::shared_ptr<candle_ros2::srv::GenericMd80Msg::Request> request,
								  std::shared_ptr<candle_ros2::srv::GenericMd80Msg::Response> response)
{
	std::vector<mab::Candle*> candlesToBegin;

	for (auto& id : request->drive_ids)
	{
		auto candle = findCandleByMd80Id(id);
		if (candle != NULL)
		{
			response->drives_success.push_back(candle->controlMd80Enable(id, true));
			/* this is to ensure only one copy of CANdle object is present for the begin procedure */
			if (std::find(candlesToBegin.begin(), candlesToBegin.end(), candle) == candlesToBegin.end())
				candlesToBegin.push_back(candle);
		}
		else
		{
			response->drives_success.push_back(false);
			RCLCPP_WARN(this->get_logger(), "Drive with ID: %d is not added!", id);
		}
	}

	/* begin the communication on each CANdle */
	for (auto candle : candlesToBegin)
	{
		candle->begin();
	}

	pubTimer->reset();
}
void Md80Node::service_disableMd80(const std::shared_ptr<candle_ros2::srv::GenericMd80Msg::Request> request,
								   std::shared_ptr<candle_ros2::srv::GenericMd80Msg::Response> response)
{
	std::vector<mab::Candle*> candlesToEnd;

	/* just to find which CANdle devices are commanded to be disabled */
	for (auto& id : request->drive_ids)
	{
		auto candle = findCandleByMd80Id(id);
		if (candle != NULL)
		{
			if (std::find(candlesToEnd.begin(), candlesToEnd.end(), candle) == candlesToEnd.end())
				candlesToEnd.push_back(candle);
		}
	}

	/* ending CANdles */
	for (auto candle : candlesToEnd)
	{
		candle->end();
	}
	pubTimer->cancel();

	/* Actually disabling individual MD80s */
	for (auto& id : request->drive_ids)
	{
		auto candle = findCandleByMd80Id(id);
		if (candle != NULL)
			response->drives_success.push_back(candle->controlMd80Enable(id, false));
		else
		{
			response->drives_success.push_back(false);
			RCLCPP_WARN(this->get_logger(), "Drive with ID: %d is not added!", id);
		}
	}
}
void Md80Node::publishJointStates()
{
	sensor_msgs::msg::JointState jointStateMsg;
	jointStateMsg.header.stamp = rclcpp::Clock().now();
	for (auto candle : candleInstances)
	{
		for (auto& md : candle->md80s)
		{
			jointStateMsg.name.push_back(std::string("Joint " + std::to_string(md.getId())));
			jointStateMsg.position.push_back(md.getPosition());
			jointStateMsg.velocity.push_back(md.getVelocity());
			jointStateMsg.effort.push_back(md.getTorque());
		}
	}

	this->jointStatePub->publish(jointStateMsg);
}
void Md80Node::motionCommandCallback(const std::shared_ptr<candle_ros2::msg::MotionCommand> msg)
{
	if (msg->drive_ids.size() != msg->target_position.size() || msg->drive_ids.size() != msg->target_velocity.size() ||
		msg->drive_ids.size() != msg->target_torque.size())
	{
		RCLCPP_WARN(this->get_logger(), "Motion Command message incomplete. Sizes of arrays do not match! Ignoring message.");
		return;
	}
	for (int i = 0; i < (int)msg->drive_ids.size(); i++)
	{
		try
		{
			auto candle = findCandleByMd80Id(msg->drive_ids[i]);
			if (candle != NULL)
			{
				auto& md = candle->getMd80FromList(msg->drive_ids[i]);
				md.setTargetPosition(msg->target_position[i]);
				md.setTargetVelocity(msg->target_velocity[i]);
				md.setTargetTorque(msg->target_torque[i]);
			}
			else
				RCLCPP_WARN(this->get_logger(), "Drive with ID: %d is not added!", msg->drive_ids[i]);
		}
		catch (const char* eMsg)
		{
			RCLCPP_WARN(this->get_logger(), eMsg);
		}
	}
}
void Md80Node::impedanceCommandCallback(const std::shared_ptr<candle_ros2::msg::ImpedanceCommand> msg)
{
	if (msg->drive_ids.size() != msg->kp.size() || msg->drive_ids.size() != msg->kd.size())
	{
		RCLCPP_WARN(this->get_logger(), "Impedance Command message incomplete. Sizes of arrays do not match! Ignoring message.");
		return;
	}
	for (int i = 0; i < (int)msg->drive_ids.size(); i++)
	{
		try
		{
			auto candle = findCandleByMd80Id(msg->drive_ids[i]);
			if (candle != NULL)
			{
				auto& md = candle->getMd80FromList(msg->drive_ids[i]);
				md.setImpedanceControllerParams(msg->kp[i], msg->kd[i]);
				md.setMaxTorque(msg->max_output[i]);
			}
			else
				RCLCPP_WARN(this->get_logger(), "Drive with ID: %d is not added!", msg->drive_ids[i]);
		}
		catch (const char* eMsg)
		{
			RCLCPP_WARN(this->get_logger(), eMsg);
		}
	}
}
void Md80Node::velocityCommandCallback(const std::shared_ptr<candle_ros2::msg::VelocityPidCommand> msg)
{
	if (msg->drive_ids.size() != msg->velocity_pid.size())
	{
		RCLCPP_WARN(this->get_logger(), "Velocity Command message incomplete. Sizes of arrays do not match! Ignoring message.");
		return;
	}
	for (int i = 0; i < (int)msg->drive_ids.size(); i++)
	{
		try
		{
			auto candle = findCandleByMd80Id(msg->drive_ids[i]);
			if (candle != NULL)
			{
				auto& md = candle->getMd80FromList(msg->drive_ids[i]);
				md.setVelocityControllerParams(msg->velocity_pid[i].kp, msg->velocity_pid[i].ki, msg->velocity_pid[i].kd, msg->velocity_pid[i].i_windup);
				md.setMaxTorque(msg->velocity_pid[i].max_output);
			}
			else
				RCLCPP_WARN(this->get_logger(), "Drive with ID: %d is not added!", msg->drive_ids[i]);
		}
		catch (const char* eMsg)
		{
			RCLCPP_WARN(this->get_logger(), eMsg);
		}
	}
}
void Md80Node::positionCommandCallback(const std::shared_ptr<candle_ros2::msg::PositionPidCommand> msg)
{
	if (msg->drive_ids.size() != msg->position_pid.size())
	{
		RCLCPP_WARN(this->get_logger(), "Position Command message incomplete. Sizes of arrays do not match! Ignoring message.");
		return;
	}
	for (int i = 0; i < (int)msg->drive_ids.size(); i++)
	{
		try
		{
			auto candle = findCandleByMd80Id(msg->drive_ids[i]);
			if (candle != NULL)
			{
				auto& md = candle->getMd80FromList(msg->drive_ids[i]);
				md.setPositionControllerParams(msg->position_pid[i].kp, msg->position_pid[i].ki, msg->position_pid[i].kd, msg->position_pid[i].i_windup);
				md.setProfileVelocity(msg->position_pid[i].max_output);
				if (i < (int)msg->velocity_pid.size())
				{
					md.setVelocityControllerParams(msg->velocity_pid[i].kp, msg->velocity_pid[i].ki, msg->velocity_pid[i].kd, msg->velocity_pid[i].i_windup);
					md.setMaxTorque(msg->velocity_pid[i].max_output);
				}
			}
			else
				RCLCPP_WARN(this->get_logger(), "Drive with ID: %d is not added!", msg->drive_ids[i]);
		}
		catch (const char* eMsg)
		{
			RCLCPP_WARN(this->get_logger(), eMsg);
		}
	}
}
int main(int argc, char** argv)
{
	rclcpp::init(argc, argv);
	auto node = std::make_shared<Md80Node>(argc, argv);
	rclcpp::spin(node);
	rclcpp::shutdown();
	return 0;
}
