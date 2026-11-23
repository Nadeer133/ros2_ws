#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/laser_scan.hpp"
#include "std_msgs/msg/u_int8.hpp"

using namespace std;

class FaultInjector : public rclcpp::Node
{
public:
    FaultInjector() : Node("fault_injector"),lidarfaultcmd_(0)
    {
        faultcmdsub_=this->create_subscription<std_msgs::msg::UInt8>("fault_command",10,std::bind(&FaultInjector::faultcmdCallBack,this,placeholders::_1));
        lidarsub_=this->create_subscription<sensor_msgs::msg::LaserScan>("/scan_raw",10,std::bind(&FaultInjector::faultinjCallBack,this,placeholders::_1));
        publisher_=this->create_publisher<sensor_msgs::msg::LaserScan>("/scan",10);

    }
private:
    void faultcmdCallBack(const std_msgs::msg::UInt8::SharedPtr faultcmd_){
        lidarfaultcmd_=faultcmd_->data;
    }
    void faultinjCallBack(const sensor_msgs::msg::LaserScan::SharedPtr scanrawdata_){
        if(lidarfaultcmd_==0){
            auto scandata_= *scanrawdata_;
            publisher_->publish(scandata_);
        }
        else if(lidarfaultcmd_==1)
        {
            //simulate lidar communication fault
            return;
        }

    }


uint8_t lidarfaultcmd_;
rclcpp::TimerBase::SharedPtr timer_;
rclcpp::Subscription<sensor_msgs::msg::LaserScan>::SharedPtr lidarsub_;
rclcpp::Subscription<std_msgs::msg::UInt8>::SharedPtr faultcmdsub_;
rclcpp::Publisher<sensor_msgs::msg::LaserScan>::SharedPtr publisher_;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<FaultInjector>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}