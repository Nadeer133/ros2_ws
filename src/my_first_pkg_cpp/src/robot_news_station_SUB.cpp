#include "rclcpp/rclcpp.hpp"
#include "example_interfaces/msg/string.hpp"

class SmartPhone : public rclcpp::Node
{
public:
    SmartPhone() : Node("Smart_Phone")
    {
        subscriber_ =  this->create_subscription<example_interfaces::msg::String>("robot_news_ChannelA",
            10, 
            std::bind(&SmartPhone::subsrobotnews, this, std::placeholders::_1));
            RCLCPP_INFO(this->get_logger(),"Subscribing to Robot News ChannelA");
        timer_ = this->create_wall_timer(std::chrono::seconds(1), [this]() {  // <--- You MUST have both the capture [this] and the args ()
        if (this->count_publishers("robot_news_ChannelA") == 0) {
            RCLCPP_WARN(this->get_logger(), "No publishers are currently connected!");
        }
        else{
            RCLCPP_WARN(this->get_logger(), "Publishers are avaialable");
        }
        this->timer_->cancel();
    });
    }
private:
    void subsrobotnews(const example_interfaces::msg::String::SharedPtr msg)
    {
        
        RCLCPP_INFO(this->get_logger(),"the recieved message is: %s", msg->data.c_str());
        
    }

    rclcpp::TimerBase::SharedPtr timer_;
    rclcpp::Subscription<example_interfaces::msg::String>::SharedPtr subscriber_;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    printf("DEBUG: Starting the main function...\n");
    auto node = std::make_shared<SmartPhone>();
    printf("DEBUG: Node created! Spinning now...\n");
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}
