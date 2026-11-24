#include "rclcpp/rclcpp.hpp"
#include "my_ros2_interfaces/msg/agv_health_state.hpp"
#include "sensor_msgs/msg/laser_scan.hpp"
#include "geometry_msgs/msg/twist_stamped.hpp"
#include "geometry_msgs/msg/pose_stamped.hpp" //goal position msg type
#include "action_msgs/msg/goal_status_array.hpp" //bt_navigatore msg type to see the goal progress:- 1:accepted, 2:executing, 4:succeeded, 6:Aborted

using namespace std::chrono_literals;

class SafetySupervisionNode : public rclcpp::Node
{
public:
    SafetySupervisionNode() : Node("safety_supervision"),last_command_time(std::chrono::steady_clock::now()),glstatus_(0)
    {
        scansub_=this->create_subscription<sensor_msgs::msg::LaserScan>("/scan",1,std::bind(&SafetySupervisionNode::scanCallBack,this,std::placeholders::_1));
        watchdogtimer_=this->create_wall_timer(250ms,std::bind(&SafetySupervisionNode::watchdogCallBack,this));
        velpub_=this->create_publisher<geometry_msgs::msg::TwistStamped>("/ackermann_steering_controller/reference",1);
        healthstpub_=this->create_publisher<my_ros2_interfaces::msg::AgvHealthState>("/agv_healthstate",10);
        goalstatussub_=this->create_subscription<action_msgs::msg::GoalStatusArray>("/navigate_to_pose/_action/status",1,std::bind(&SafetySupervisionNode::goalstatusCallBack,this,std::placeholders::_1));
        goalposesub_=this->create_subscription<geometry_msgs::msg::PoseStamped>("/goal_pose",1,std::bind(&SafetySupervisionNode::goalposeCallBack,this,std::placeholders::_1));
        goalposepub_=this->create_publisher<geometry_msgs::msg::PoseStamped>("/goal_pose",1);
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
            fault_occured_=true;
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
        else
        {
            agvhealthst_.data=0;
            agvhealthst_.msg="Normal";
            healthstpub_->publish(agvhealthst_);
            
            if(fault_occured_&& goal_received_ && glstatus_==6){
            fault_occured_=false;
            lastgoal_.header.stamp=this->now();

            goalposepub_->publish(lastgoal_);

            RCLCPP_INFO(
            this->get_logger(),
            "Sensor recovered. Republishing last navigation goal.");
            }
        }
    }
    void goalstatusCallBack(const action_msgs::msg::GoalStatusArray::SharedPtr goalstatus_){
        glstatus_= goalstatus_->status_list.back().status; // getting the goal status of the latest given goal position; exepcting aborted due to sensor fault.
    }
    void goalposeCallBack(const geometry_msgs::msg::PoseStamped::SharedPtr goalpose_){
        lastgoal_=*goalpose_;
        goal_received_ =true;
    }

geometry_msgs::msg::PoseStamped lastgoal_;
bool goal_received_ = false;
bool fault_occured_ = false;
std::chrono::steady_clock::time_point last_command_time;
uint8_t glstatus_;
rclcpp::Subscription<sensor_msgs::msg::LaserScan>::SharedPtr scansub_;
rclcpp::Publisher<my_ros2_interfaces::msg::AgvHealthState>::SharedPtr healthstpub_;
my_ros2_interfaces::msg::AgvHealthState agvhealthst_;
rclcpp::TimerBase::SharedPtr watchdogtimer_;
rclcpp::Publisher<geometry_msgs::msg::TwistStamped>::SharedPtr velpub_;
rclcpp::Subscription<geometry_msgs::msg::PoseStamped>::SharedPtr goalposesub_;
rclcpp::Publisher<geometry_msgs::msg::PoseStamped>::SharedPtr goalposepub_;
rclcpp::Subscription<action_msgs::msg::GoalStatusArray>::SharedPtr goalstatussub_;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<SafetySupervisionNode>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}