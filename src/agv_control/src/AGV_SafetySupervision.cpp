#include "rclcpp/rclcpp.hpp"
#include "my_ros2_interfaces/msg/agv_health_state.hpp"
#include "sensor_msgs/msg/laser_scan.hpp"
#include "geometry_msgs/msg/twist_stamped.hpp"

using namespace std::chrono_literals;

class SafetySupervisionNode : public rclcpp::Node
{
public:
    SafetySupervisionNode() : Node("safety_supervision"),last_command_time(std::chrono::steady_clock::now())
    {
        scansub_=this->create_subscription<sensor_msgs::msg::LaserScan>("/scan",1,std::bind(&SafetySupervisionNode::scanCallBack,this,std::placeholders::_1));
        watchdogtimer_=this->create_wall_timer(250ms,std::bind(&SafetySupervisionNode::watchdogCallBack,this));
        velpub_=this->create_publisher<geometry_msgs::msg::TwistStamped>("/ackermann_steering_controller/reference",1);
        healthstpub_=this->create_publisher<my_ros2_interfaces::msg::AgvHealthState>("/agv_healthstate",10);
        RCLCPP_INFO(this->get_logger(),"Safety Supervision Node Active!");
    }
private:
    void scanCallBack(const sensor_msgs::msg::LaserScan::SharedPtr){
        last_command_time=std::chrono::steady_clock::now();
    }
    void watchdogCallBack(){
        auto current_time=std::chrono::steady_clock::now();
        auto elapsedtime_=std::chrono::duration<double>(current_time-last_command_time).count();

        if(elapsedtime_>0.5)
        {
            geometry_msgs::msg::TwistStamped stopcommand_;
            stopcommand_.header.stamp=this->now();
            stopcommand_.header.frame_id="base_footprint_link";
            
            agvhealthst_.data=1;
            agvhealthst_.msg="FailOp";
            healthstpub_->publish(agvhealthst_);

            velpub_->publish(stopcommand_);

            RCLCPP_WARN_THROTTLE(
                this->get_logger(),
                *this->get_clock(),
                2000,
                "Nav2 command timeout! Publishing STOP.");
        }
        else{
        agvhealthst_.data=0;
        agvhealthst_.msg="Normal";
        healthstpub_->publish(agvhealthst_);
    }
}

rclcpp::Subscription<sensor_msgs::msg::LaserScan>::SharedPtr scansub_;
rclcpp::Publisher<my_ros2_interfaces::msg::AgvHealthState>::SharedPtr healthstpub_;
my_ros2_interfaces::msg::AgvHealthState agvhealthst_;
std::chrono::steady_clock::time_point last_command_time;
rclcpp::TimerBase::SharedPtr watchdogtimer_;
rclcpp::Publisher<geometry_msgs::msg::TwistStamped>::SharedPtr velpub_;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<SafetySupervisionNode>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}