#include "rclcpp/rclcpp.hpp"
#include "my_ros2_interfaces/srv/led_state.hpp"

using namespace std::chrono_literals;
using namespace std::placeholders;

class BatteryNode : public rclcpp::Node
{
public:
    BatteryNode() : Node("battery_node"), counter_(0)
    {
        client_=this->create_client<my_ros2_interfaces::srv::LedState>("set_led");
        timer_=this->create_wall_timer((1s), std::bind(&BatteryNode::sendledrequest,this));
        RCLCPP_INFO(this->get_logger(),"Client is Up and ready to request");        
    }
private:
    int counter_=0;
    void sendledrequest()
    {
        counter_++;
        while(!client_->wait_for_service(2s))
        {
            RCLCPP_WARN(this->get_logger(),"Waiting for Server!");
        }
        auto battery_req_ = std::make_shared<my_ros2_interfaces::srv::LedState::Request>();
        battery_req_->ledno = 2;
        if(counter_%4==0)
            {
                battery_req_->ledstate = 1;
                RCLCPP_INFO(this->get_logger(),"Battery Low, Sending LED Warning ON request");
                client_->async_send_request(battery_req_, std::bind(&BatteryNode::callbacksendledrequest,this,_1));
            }   
        if(counter_%10==0)
            {
                battery_req_->ledstate = 0;
                RCLCPP_INFO(this->get_logger(),"Battery Full, Sending LED Warning OFF request");
                client_->async_send_request(battery_req_, std::bind(&BatteryNode::callbacksendledrequest,this,_1));
                counter_=0;
            }

    }
        
    void callbacksendledrequest(rclcpp::Client<my_ros2_interfaces::srv::LedState>::SharedFuture future_)
    {
        auto response_=future_.get();
        RCLCPP_INFO(this->get_logger(),"Status: %s", (response_->status).c_str());
        
    }


rclcpp::TimerBase::SharedPtr timer_;
rclcpp::Client<my_ros2_interfaces::srv::LedState>::SharedPtr client_;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<BatteryNode>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}