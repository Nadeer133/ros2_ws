#include "rclcpp/rclcpp.hpp" //including rclcpp lib and pointing the actual header file

int main(int arg_count, char **arg_value) //defining main function with necessary args
{
    rclcpp::init(arg_count,arg_value);// FIRST THING WE DO: initialising ros2 communication, from rclcpp lib we are calling init function with the same args
    auto node = std::make_shared<rclcpp::Node>("My_CNode");
    RCLCPP_INFO(node->get_logger(),"Hello World");
    rclcpp::spin(node);
    rclcpp::shutdown(); // LAST THING WE DO: finally shutting down the node communication
    return 0;
}