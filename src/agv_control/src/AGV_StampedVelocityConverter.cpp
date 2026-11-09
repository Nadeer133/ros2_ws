#include <rclcpp/rclcpp.hpp>
#include <functional>
#include <memory>
#include <geometry_msgs/msg/twist_stamped.hpp>
#include <geometry_msgs/msg/twist.hpp>

using namespace std::chrono_literals;

class StampedVelocityConverter : public rclcpp::Node
{
public:
    StampedVelocityConverter() : Node("stamped_velocity_converter"),last_command_time(std::chrono::steady_clock::now())
    {
        fActv_publisher_=0;
        RCLCPP_INFO(this->get_logger(),"StampedVelocityConverter is up and running");
        timer_=this->create_wall_timer((3s),[this](){
            if(this->count_publishers("/cmd_vel")==0){
                RCLCPP_WARN(this->get_logger(),"No active publishers on /cmd_vel");
                fActv_publisher_=0;
            }
            else{
                if(!fActv_publisher_){
                RCLCPP_WARN(this->get_logger(),"Active publishers available");
                fActv_publisher_=1;}
        
            }
        });
        watchdogtimer_=this->create_wall_timer(100ms,std::bind(&StampedVelocityConverter::watchdogCallBack,this));
        subscriber_=this->create_subscription<geometry_msgs::msg::Twist>("/cmd_vel",1,std::bind(&StampedVelocityConverter::velocityCallBack,this,std::placeholders::_1));
        publisher_=this->create_publisher<geometry_msgs::msg::TwistStamped>("/ackermann_steering_controller/reference",1);
    }
private:
    void velocityCallBack(const geometry_msgs::msg::Twist::SharedPtr velocity_){
        last_command_time=std::chrono::steady_clock::now();

        outputvelocity_.twist = *velocity_;
        outputvelocity_.header.stamp = this->now();
        outputvelocity_.header.frame_id = "base_footprint_link";

        publisher_->publish(outputvelocity_);
    }
    void watchdogCallBack(){
        auto current_time=std::chrono::steady_clock::now();
        auto elapsedtime_=std::chrono::duration<double>(current_time-last_command_time).count();

        if(elapsedtime_>0.5)
        {
            geometry_msgs::msg::TwistStamped stopcommand_;
            stopcommand_.header.stamp=this->now();
            stopcommand_.header.frame_id="base_footprint_link";
            
            publisher_->publish(stopcommand_);

            RCLCPP_WARN_THROTTLE(
                this->get_logger(),
                *this->get_clock(),
                2000,
                "Nav2 command timeout! Publishing STOP.");
        }
    }

geometry_msgs::msg::TwistStamped outputvelocity_;
std::int8_t fActv_publisher_;
rclcpp::TimerBase::SharedPtr timer_;
rclcpp::TimerBase::SharedPtr watchdogtimer_;
std::chrono::steady_clock::time_point last_command_time;
rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr subscriber_;
rclcpp::Publisher<geometry_msgs::msg::TwistStamped>::SharedPtr publisher_;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<StampedVelocityConverter>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}