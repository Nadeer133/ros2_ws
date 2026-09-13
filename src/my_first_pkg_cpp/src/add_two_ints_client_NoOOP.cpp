#include "rclcpp/rclcpp.hpp"
#include "example_interfaces/srv/add_two_ints.hpp"

using namespace std::chrono_literals;
int main(int argc, char **argv)
{
    rclcpp::init(argc, argv); // initialising ROS Communication
    auto node = std::make_shared<rclcpp::Node>("add_two_ints_Client");// creating a sharedptr of the type rclcpp::Node with the name: add_two_ints_Client
    auto client = node->create_client<example_interfaces::srv::AddTwoInts>("add_two_ints"); //creating a client with the interface type:AddTwoInts with the service name add_two_ints.
    while (!client->wait_for_service(1s)) // there are chances were it takes some time for the server to come up, so including a delay otherwise, the client may not find the service server it is trying to talk to.
    {
        RCLCPP_WARN(node->get_logger(),"Waiting for server...");
    }
    auto request = std::make_shared<example_interfaces::srv::AddTwoInts::Request>();
    request->a = 6;
    request->b = 5;
    auto future = client->async_send_request(request);
    rclcpp::spin_until_future_complete(node, future);
    auto response = future.get();
    RCLCPP_INFO(node->get_logger(),"%d + %d = %d",int(request->a), int(request->b), int(response->sum));
    rclcpp::shutdown();
    return 0;
}
/* line no 17-19 can also be written like this:
    client->async_send_request(request);
    rclcpp::spin_until_future_complete(node, client->async_send_request(request));
    auto response = client->async_send_request(request).get();
    But here the problem is, the client call to the server happens 3 times..
    which is unneccessary and also in the 3rd line where the call happens, the call
    as well as the fetch for result happens at the very same instant. The server may
    not even get the time to process the request. As a result that client call will fail
    and we wont get any output in the client side terminal.
    */