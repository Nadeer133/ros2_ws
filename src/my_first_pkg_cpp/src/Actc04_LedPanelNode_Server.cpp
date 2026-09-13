#include "rclcpp/rclcpp.hpp"
#include "my_ros2_interfaces/srv/led_state.hpp"
#include "my_ros2_interfaces/msg/state_led.hpp"

using namespace std::chrono_literals;
using namespace std::placeholders;

class LedPanelNode : public rclcpp::Node
{
public:
    LedPanelNode() : Node("led_panel")
    {
        led_panel[3]={0};
        service_=this->create_service<my_ros2_interfaces::srv::LedState>("set_led",std::bind(&LedPanelNode::ledpanelaction,this,_1,_2));
        RCLCPP_INFO(this->get_logger(),"Server is UP and Running");
        publisher_=this->create_publisher<my_ros2_interfaces::msg::StateLed>("led_panel_state",10);
    }
private:
    void ledpanelaction(my_ros2_interfaces::srv::LedState::Request::SharedPtr request_,
                        my_ros2_interfaces::srv::LedState::Response::SharedPtr response_)
    {
        my_ros2_interfaces::msg::StateLed msg;
        if(request_->ledstate==1)
        {
            response_->status = "Warning LED Turned ON";
            led_panel[request_->ledno]=100;
            RCLCPP_INFO(this->get_logger(),"[%d,%d,%d]",led_panel[0],led_panel[1],led_panel[2]);
            msg.msg="led:" + std::to_string((request_->ledno)+1) +" is turned ON:["+ std::to_string(led_panel[0]) + "," + std::to_string(led_panel[1]) + "," + std::to_string(led_panel[2]) + "]";
            publisher_->publish(msg);

            
        }
        else
        {
            response_->status = "Warning LED Turned OFF";
            msg.msg="led: %d" + std::to_string(request_->ledno) +" is turned OFF";
            RCLCPP_INFO(this->get_logger(),"[0,0,0]");
            msg.msg="led is turned OFF, [0,0,0]";
            publisher_->publish(msg);
        }
    }
int led_panel[3];
rclcpp::TimerBase::SharedPtr timer_;
rclcpp::Publisher<my_ros2_interfaces::msg::StateLed>::SharedPtr publisher_;
rclcpp::Service<my_ros2_interfaces::srv::LedState>::SharedPtr service_;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<LedPanelNode>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}