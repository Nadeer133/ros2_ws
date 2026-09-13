#include "rclcpp/rclcpp.hpp"
#include "example_interfaces/msg/string.hpp"
using namespace std::chrono_literals;
class RobotNewsStation : public rclcpp::Node 
{
public:
    RobotNewsStation() : Node("robot_news_station"), robot_name("R2D2")
    {
        publisher_ = this->create_publisher<example_interfaces::msg::String>("robot_news_ChannelA", 10); // creating the actual publisher using create_publisher API. And also the API works only with objects. this-> points to the current object of RobotNewsStation class.
        timer_= this->create_wall_timer((3s), std::bind(&RobotNewsStation::PublishNews, this));
        RCLCPP_INFO(this->get_logger(), "Robot News Station have been started");
    }   
private:
    void PublishNews()
    {
    auto msg = example_interfaces::msg::String();
    msg.data = std::string("Hi, this is ") + robot_name + std::string(" from the robot news station.");
    publisher_->publish(msg);
    }

    rclcpp::TimerBase::SharedPtr timer_;
    rclcpp::Publisher<example_interfaces::msg::String>::SharedPtr publisher_; // publisher_ is a shared ptr variable that points to a publisher that publishes string messages.
    std::string robot_name;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<RobotNewsStation>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}