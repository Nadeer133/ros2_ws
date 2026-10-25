#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/image.hpp"


class ObjectDetectionNode : public rclcpp::Node
{
public:
    ObjectDetectionNode() : Node("objectdetection_node")
    {
        this->declare_parameter("camera_topic","/camera/image_raw");
        topic_name = this->get_parameter("camera_topic").as_string();
        Subscriber_=this->create_subscription<sensor_msgs::msg::Image>(topic_name,10,std::bind(&ObjectDetectionNode::Obj_Detct,this,std::placeholders::_1));
    }
private:
    void Obj_Detct(const sensor_msgs::msg::Image::SharedPtr Image_){

    }
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