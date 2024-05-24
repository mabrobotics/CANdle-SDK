#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/joint_state.hpp>

#include "candle.hpp"
#include "candle_ros2/msg/impedance_command.hpp"
#include "candle_ros2/msg/motion_command.hpp"
#include "candle_ros2/msg/position_pid_command.hpp"
#include "candle_ros2/msg/velocity_pid_command.hpp"
#include "candle_ros2/srv/add_md80s.hpp"
#include "candle_ros2/srv/generic_md80_msg.hpp"
#include "candle_ros2/srv/set_limits_md80.hpp"
#include "candle_ros2/srv/set_mode_md80s.hpp"

class Md80Node : public rclcpp::Node
{
   public:
	Md80Node(int argc, char** argv);
	~Md80Node();

   private:
	std::vector<mab::Candle*> candleInstances;
	rclcpp::TimerBase::SharedPtr pubTimer;

	rclcpp::Service<candle_ros2::srv::AddMd80s>::SharedPtr addMd80Service;
	rclcpp::Service<candle_ros2::srv::GenericMd80Msg>::SharedPtr zeroMd80Service;
	rclcpp::Service<candle_ros2::srv::SetModeMd80s>::SharedPtr setModeMd80Service;
	rclcpp::Service<candle_ros2::srv::GenericMd80Msg>::SharedPtr enableMd80Service;
	rclcpp::Service<candle_ros2::srv::GenericMd80Msg>::SharedPtr disableMd80Service;
	rclcpp::Service<candle_ros2::srv::SetLimitsMd80>::SharedPtr setLimitsMd80Service;

	rclcpp::Publisher<sensor_msgs::msg::JointState>::SharedPtr jointStatePub;
	rclcpp::Subscription<candle_ros2::msg::MotionCommand>::SharedPtr motionCommandSub;
	rclcpp::Subscription<candle_ros2::msg::ImpedanceCommand>::SharedPtr impedanceCommandSub;
	rclcpp::Subscription<candle_ros2::msg::VelocityPidCommand>::SharedPtr velocityCommandSub;
	rclcpp::Subscription<candle_ros2::msg::PositionPidCommand>::SharedPtr positionCommandSub;

	void service_addMd80(const std::shared_ptr<candle_ros2::srv::AddMd80s::Request> request,
						 std::shared_ptr<candle_ros2::srv::AddMd80s::Response> response);
	void service_zeroMd80(const std::shared_ptr<candle_ros2::srv::GenericMd80Msg::Request> request,
						  std::shared_ptr<candle_ros2::srv::GenericMd80Msg::Response> response);
	void service_setModeMd80(const std::shared_ptr<candle_ros2::srv::SetModeMd80s::Request> request,
							 std::shared_ptr<candle_ros2::srv::SetModeMd80s::Response> response);
	void service_enableMd80(const std::shared_ptr<candle_ros2::srv::GenericMd80Msg::Request> request,
							std::shared_ptr<candle_ros2::srv::GenericMd80Msg::Response> response);
	void service_disableMd80(const std::shared_ptr<candle_ros2::srv::GenericMd80Msg::Request> request,
							 std::shared_ptr<candle_ros2::srv::GenericMd80Msg::Response> response);

	mab::Candle* findCandleByMd80Id(uint16_t md80Id);
	void publishJointStates();
	void motionCommandCallback(const std::shared_ptr<candle_ros2::msg::MotionCommand> msg);
	void impedanceCommandCallback(const std::shared_ptr<candle_ros2::msg::ImpedanceCommand> msg);
	void velocityCommandCallback(const std::shared_ptr<candle_ros2::msg::VelocityPidCommand> msg);
	void positionCommandCallback(const std::shared_ptr<candle_ros2::msg::PositionPidCommand> msg);
};
