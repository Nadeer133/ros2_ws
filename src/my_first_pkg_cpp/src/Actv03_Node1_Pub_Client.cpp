#include "rclcpp/rclcpp.hpp"
#include "example_interfaces/msg/int64.hpp"
#include "example_interfaces/srv/set_bool.hpp"

using namespace std::placeholders;
using namespace std::chrono_literals;

class Num_Pub : public rclcpp::Node
{
public:
    //using SetBoolFuture = rclcpp::Client<example_interfaces::srv::SetBool>::SharedFuture;
    Num_Pub() : Node("number_publisher")
    {
        msg.data = 10;
        publisher_= this->create_publisher<example_interfaces::msg::Int64>("number", 10);
        timer_=this->create_wall_timer((1s), std::bind(&Num_Pub::publishnumber, this));
        RCLCPP_INFO(this->get_logger(),"Publisher 'number' has started publishing");
        client_=this->create_client<example_interfaces::srv::SetBool>("reset_counter");
        timer2_=this->create_wall_timer((5s), std::bind(&Num_Pub::reset_counter, this));
    }
private:
    void publishnumber()
    {
        publisher_->publish(msg);
    }
    void reset_counter()
    {
        RCLCPP_INFO(this->get_logger(),"Client call made to RESET counter");
        auto request_= std::make_shared<example_interfaces::srv::SetBool::Request>();
        request_->data = true;
        client_->async_send_request(
            request_, 
            std::bind(&Num_Pub::callbackreset_counter, this, _1));
    }
    void callbackreset_counter(rclcpp::Client<example_interfaces::srv::SetBool>::SharedFuture future)
    {
        auto response_ = future.get();
        if(response_->success ==true)
        {
            RCLCPP_INFO(this->get_logger(),"%s, Counter Reset Succesfull", response_->message.c_str());
        }
        else
        {
            RCLCPP_INFO(this->get_logger(),"%s, Counter Reset Unsuccesfull", response_->message.c_str());
        }
    }
example_interfaces::msg::Int64 msg;
rclcpp::TimerBase::SharedPtr timer_,timer2_;
//rclcpp::Client<example_interfaces::srv::SetBool::Request>::SharedPtr request_;
rclcpp::Client<example_interfaces::srv::SetBool>::SharedPtr client_; 
rclcpp::Publisher<example_interfaces::msg::Int64>::SharedPtr publisher_;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<Num_Pub>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}