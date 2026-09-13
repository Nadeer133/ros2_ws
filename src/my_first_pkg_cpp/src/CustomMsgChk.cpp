#include "rclcpp/rclcpp.hpp"
#include "my_ros2_interfaces/msg/hardware_status.hpp"

using namespace std;
using namespace std::chrono_literals;
class HardwareStatusPublisherNode : public rclcpp::Node
{
public:
    HardwareStatusPublisherNode() : Node("hardware_status_publisher")
    {
        publisher_=this->create_publisher<my_ros2_interfaces::msg::HardwareStatus>("Status_Hardware",10);
        timer_=this->create_wall_timer((3s),std::bind(&HardwareStatusPublisherNode::publishHwStatus, this));
        RCLCPP_INFO(this->get_logger(),"Publishing Hw Status");
    }
private:
void publishHwStatus()
{
    my_ros2_interfaces::msg::HardwareStatus HwStatus_;
    HwStatus_.temperature = 40.0;
    HwStatus_.are_motors_ready = true;
    HwStatus_.debug_message = "HwStatus is Normal";
    publisher_->publish(HwStatus_);
}

rclcpp::TimerBase::SharedPtr timer_;
rclcpp::Publisher<my_ros2_interfaces::msg::HardwareStatus>::SharedPtr publisher_;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<HardwareStatusPublisherNode>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}