#include "rclcpp/rclcpp.hpp"
#include "example_interfaces/srv/add_two_ints.hpp"

using namespace std::placeholders;
using namespace std::chrono_literals;
class AddTwoIntsClient : public rclcpp::Node
{
public:
    AddTwoIntsClient() : Node("add_two_ints_client")
    {
        client_=this->create_client<example_interfaces::srv::AddTwoInts>("add_two_ints");
        RCLCPP_INFO(this->get_logger(), "Client is alive and ready to request!!!");
    }

    void addTwoInts(int a, int b)
    {
        while(!client_->wait_for_service(1s))
        {
            RCLCPP_WARN(this->get_logger(), "Waiting for server!!!");
        }
        auto request = std::make_shared<example_interfaces::srv::AddTwoInts::Request>();
        request->a = a;
        request->b = b;
        client_->async_send_request(request, std::bind(&AddTwoIntsClient::callbackaddTwoInts, this, _1));
        RCLCPP_INFO(this->get_logger(),"Served discovered, Request Sent!!!");
    }
private:
    void callbackaddTwoInts(rclcpp::Client<example_interfaces::srv::AddTwoInts>::SharedFuture future_)
    {
        auto response_ = future_.get();
        RCLCPP_INFO(this->get_logger(),"Response recieved : Sum = %ld",  (long int)response_->sum);
    }

    rclcpp::Client<example_interfaces::srv::AddTwoInts>::SharedPtr client_;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<AddTwoIntsClient>();
    node->addTwoInts(5,6);
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}