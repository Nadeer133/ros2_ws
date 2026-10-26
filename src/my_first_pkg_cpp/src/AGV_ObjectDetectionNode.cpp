#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/image.hpp"
#include <functional> //std::bind. std::placeholders
#include <memory> //std::make_shared
#include <string> //std::string

using namespace std::chrono_literals;

class ObjectDetectionNode : public rclcpp::Node
{
public:
    ObjectDetectionNode() : Node("objectdetection_node")
    {
        this->declare_parameter("camera_topic","/camera/image_raw");
        topic_name = this->get_parameter("camera_topic").as_string();
        fActv_publisher_=0;
        RCLCPP_INFO(this->get_logger(),"ObjectDetectionNode is up and running");
        timer_=this->create_wall_timer((3000ms),[this](){
            if(this->count_publishers("/camera/image_raw")==0){
                RCLCPP_WARN(this->get_logger(),"No active publishers on the topic:%s",topic_name.c_str());
                fActv_publisher_=0;
            }
            else{
                if(!fActv_publisher_){
                RCLCPP_WARN(this->get_logger(),"Active publishers available");
                fActv_publisher_=1;}
            }
        });
        Subscriber_=this->create_subscription<sensor_msgs::msg::Image>(topic_name,10,std::bind(&ObjectDetectionNode::ImageCallback,this,std::placeholders::_1));
    }
private:
    void ImageCallback(const sensor_msgs::msg::Image::SharedPtr Image_){
        RCLCPP_INFO(this->get_logger(),"Received Image: Width=%u, Hight=%u, Encoding=%s", Image_->width, Image_->height, Image_->encoding.c_str());
    }
std::int8_t fActv_publisher_;
rclcpp::TimerBase::SharedPtr timer_;
std::string topic_name;
rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr Subscriber_;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<ObjectDetectionNode>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}