#include "rclcpp/rclcpp.hpp" //including rclcpp lib and pointing the actual header file

class MyNode : public rclcpp::Node
{
public:
    MyNode() : Node("cpp_node"), counter_(0) /*here MyNode() is a constructor and the syntx here is to create parent class Node obj(means another node) with name cpp_node, create and init immd the variable counter_before entering the constructor body*/
    {
        RCLCPP_INFO(this->get_logger(),"Hello World");
        timer_= this->create_wall_timer(std::chrono::seconds(1), std::bind(&MyNode::timerCallBack, this));
    }
private:
    void timerCallBack()
    {
        RCLCPP_INFO(this->get_logger(),"Hello %d", counter_);
        counter_++;
    }
    rclcpp::TimerBase::SharedPtr timer_;
    int counter_;
};

int main(int arg_count, char **arg_value) //defining main function with necessary args
{
    rclcpp::init(arg_count,arg_value);// FIRST THING WE DO: initialising ros2 communication, from rclcpp lib we are calling init function with the same args
    auto node = std::make_shared<MyNode>();
    rclcpp::spin(node);
    rclcpp::shutdown(); // LAST THING WE DO: finally shutting down the node communication
    return 0;
}