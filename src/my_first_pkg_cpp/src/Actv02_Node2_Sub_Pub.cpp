#include "rclcpp/rclcpp.hpp"
#include "example_interfaces/msg/int64.hpp"

using namespace std::chrono_literals;

class Num_Count : public rclcpp::Node
{
public:
    Num_Count() : Node("number_counter")
    {
        counter_.data=0;
        subscriber_ = this->create_subscription<example_interfaces::msg::Int64>("number_upd",10, 
        std::bind(&Num_Count::updatedata, this, std::placeholders:: _1));
        RCLCPP_INFO(this->get_logger(),"Subscribing to the topic :number_upd...");
        publisher_= this->create_publisher<example_interfaces::msg::Int64>("number_count",10);
        timer_=this->create_wall_timer((3s), std::bind(&Num_Count::publishdata, this));
    }
private:
    void updatedata(example_interfaces::msg::Int64 msg)
    {
        counter_.data = counter_.data + msg.data;
    }
    void publishdata()
    {
        publisher_->publish(counter_);
        //RCLCPP_INFO(this->get_logger(), "Data: %ld", counter_.data);
        //RCLCPP_INFO(this->get_logger(), "----");
    }
example_interfaces::msg::Int64 counter_;
rclcpp::TimerBase::SharedPtr timer_;
rclcpp::Subscription<example_interfaces::msg::Int64>::SharedPtr subscriber_;
rclcpp::Publisher<example_interfaces::msg::Int64>::SharedPtr publisher_;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<Num_Count>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}