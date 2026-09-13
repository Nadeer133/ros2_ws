#include "rclcpp/rclcpp.hpp"
#include "my_ros2_interfaces/srv/spawn_chase.hpp"
#include "turtlesim/srv/spawn.hpp"
#include "turtlesim/srv/kill.hpp"
#include "turtlesim/srv/teleport_relative.hpp"
#include "turtlesim/msg/pose.hpp"
#include <random> // required for generating random values for spawning
#include <map> //for storing values in key:value pair, map datatype
#include <algorithm> // for using some of the built in algorithms

using namespace std::chrono_literals;
using namespace std::placeholders;

class SpawnChase : public rclcpp::Node
{
public:
    SpawnChase() : Node("spawn_chase")
    {
        flag = true;
        spawnclient_=this->create_client<turtlesim::srv::Spawn>("/spawn");
        huntclient_=this->create_client<my_ros2_interfaces::srv::SpawnChase>("HuntRequest");
        //spawn();
        timer_=this->create_wall_timer((1.5s),std::bind(&SpawnChase::spawn,this));
        //timer2_=this->create_wall_timer((2s),std::bind(&SpawnChase::Update_AliveTurtle_Registry,this));
        timer3_=this->create_wall_timer((2.5s),std::bind(&SpawnChase::HuntReq,this));
        RCLCPP_INFO(this->get_logger(),"SpawnClient & HuntClient Up and Running!!!");
        turtle1posSub_=this->create_subscription<turtlesim::msg::Pose>("/turtle1/pose",10,std::bind(&SpawnChase::MainTurtlePos,this,_1));

    }
private:
    std::string last_spawned_turtle;
    void spawn()
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(1.0,10.0);
        std::uniform_real_distribution<> dis_theta(-3.14,3.14);
        while(!spawnclient_->wait_for_service(2s))
        {
            RCLCPP_WARN(this->get_logger(),"Waiting for Server!");
        }
        auto spawnreq_ = std::make_shared<turtlesim::srv::Spawn::Request>();
        spawnreq_->x = dis(gen);
        spawnreq_->y = dis(gen);
        spawnreq_->theta = dis_theta(gen);
        spawnclient_->async_send_request(spawnreq_,std::bind(&SpawnChase::spawn_callback,this,_1));
        RCLCPP_INFO(this->get_logger(),"Sending Spawn request");
    }
    void spawn_callback(rclcpp::Client<turtlesim::srv::Spawn>::SharedFuture spawnfuture)
    {
        auto spawnresponse_=spawnfuture.get();
        last_spawned_turtle = spawnresponse_->name;
        RCLCPP_INFO(this->get_logger(),"%s spawned",last_spawned_turtle.c_str());
        posesub_ = this->create_subscription<turtlesim::msg::Pose>("/"+last_spawned_turtle+"/pose",10,
                                                                   std::bind(&SpawnChase::Update_AliveTurtle_Registry,this,_1));
    }
    void MainTurtlePos(const turtlesim::msg::Pose position_)
    {
        turtle1pos_ = position_;
    }
    void Update_AliveTurtle_Registry(const turtlesim::msg::Pose position_)
    {
        turtle_registry[last_spawned_turtle] = position_;  
        /*posesub_ = this->create_subscription<turtlesim::msg::Pose>("/"+SpawnedTurtlename+"/pose",10,
                                                                        [this, SpawnedTurtlename] (const turtlesim::msg::Pose::SharedPtr position_){
                                                                         this->turtle_registry[SpawnedTurtlename] = *position_; 
                                                                         RCLCPP_INFO(this->get_logger(), "Received data for %s: x=%f, y=%f", SpawnedTurtlename.c_str(), position_->x,position_->y);
                                                                        });*/
        RCLCPP_INFO(this->get_logger(),"spawned turtle position, x:%f, y:%f, theta:%f ",turtle_registry[last_spawned_turtle].x,turtle_registry[last_spawned_turtle].y,turtle_registry[last_spawned_turtle].theta);                                                                
    }
    void HuntReq()
    {
        auto huntreq_=std::make_shared<my_ros2_interfaces::srv::SpawnChase::Request>();
        auto it = turtle_registry.begin();
        /*while(turtle_registry[it->first].x ==0.0 && turtle_registry[it->first].y==0){
            RCLCPP_INFO(this->get_logger(),"Waiting for registry update");
        }*/
        if(flag==true && it!=turtle_registry.end() && turtle_registry[it->first].x >0.0 && turtle_registry[it->first].y>0.0)
        {
            
            RCLCPP_INFO(this->get_logger(),"Turtle Registry Updated");
            auto turtleposnear_ = this->get_nearest_turtle();
            huntreq_->x = turtleposnear_.x;
            huntreq_->y = turtleposnear_.y;
            huntreq_->theta = turtleposnear_.theta;
            huntreq_->turtlename = turtlenearest_;
            RCLCPP_INFO(this->get_logger(),"huntreq_->x: %f, huntreq_->y: %f, huntreq_->theta: %f",huntreq_->x,huntreq_->y,huntreq_->theta);
            huntreq_->hunt_f = 1;
            
            huntclient_->async_send_request(huntreq_,std::bind(&SpawnChase::huntstatus,this,_1));
            RCLCPP_INFO(this->get_logger(),"Sending Hunt request");
            flag = false;
            
        }
        else
        {
            RCLCPP_INFO(this->get_logger(),"Turtle Registry yet to be updated");
        }

    }

    void huntstatus(rclcpp::Client<my_ros2_interfaces::srv::SpawnChase>::SharedFuture huntfuture)
    {
        auto status = huntfuture.get();
        if(status->succees)
        {
            RCLCPP_INFO(this->get_logger(),"%s",status->msg.c_str());
            turtle_registry.erase(turtlenearest_);
            flag=true;

        }
        else
        {
            RCLCPP_INFO(this->get_logger(),"%s",status->msg.c_str());

        }
    }
    turtlesim::msg::Pose get_nearest_turtle()
    {
        std::map<std::string, turtlesim::msg::Pose> allturtleposes_;
        std::map<std::string, float> euclid_;
        for(const auto& [key,value]:turtle_registry){
            euclid_[key]=0.0;
        }
        //int i=0;
        for(auto it = turtle_registry.begin(); it != turtle_registry.end(); ++it)
        {      
            euclid_[it->first] = std::sqrt((turtle1pos_.x - turtle_registry[it->first].x)*(turtle1pos_.x - turtle_registry[it->first].x) + 
                                    (turtle1pos_.y - turtle_registry[it->first].y)*(turtle1pos_.y - turtle_registry[it->first].y));
            //++i; 
        }
        auto min_it = std::min_element(euclid_.begin(),euclid_.end());
        
        if (min_it!=euclid_.end())
        {
            turtlenearest_ = min_it->first;
        }
        return turtle_registry[turtlenearest_];

        
    }

bool flag;
std::map<std::string, turtlesim::msg::Pose> turtle_registry;
std::string turtlenearest_;
rclcpp::TimerBase::SharedPtr timer_;
rclcpp::TimerBase::SharedPtr timer2_;
rclcpp::TimerBase::SharedPtr timer3_;
rclcpp::Client<turtlesim::srv::Spawn>::SharedPtr spawnclient_;
rclcpp::Client<my_ros2_interfaces::srv::SpawnChase>::SharedPtr huntclient_;
rclcpp::Subscription<turtlesim::msg::Pose>::SharedPtr turtle1posSub_;
rclcpp::Subscription<turtlesim::msg::Pose>::SharedPtr posesub_;
turtlesim::msg::Pose turtle1pos_;
};  

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<SpawnChase>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}