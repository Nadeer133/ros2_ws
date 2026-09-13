#include "rclcpp/rclcpp.hpp"
#include "my_ros2_interfaces/srv/spawn_chase.hpp"
#include "turtlesim/srv/spawn.hpp"
#include "turtlesim/srv/kill.hpp"
#include "turtlesim/srv/teleport_absolute.hpp"
#include "turtlesim/msg/pose.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include <map>

using namespace std::placeholders;
using namespace std::chrono_literals;

class HuntKill : public rclcpp::Node
{
public:
    HuntKill() : Node("hunt_kill")
    {
        turtle1posSub_=this->create_subscription<turtlesim::msg::Pose>("/turtle1/pose",10,std::bind(&HuntKill::MainTurtlePos,this,_1));
        Pubcmdvel_=this->create_publisher<geometry_msgs::msg::Twist>("/turtle1/cmd_vel",10);
        killclient_=this->create_client<turtlesim::srv::Kill>("/kill");
        huntservice_=this->create_service<my_ros2_interfaces::srv::SpawnChase>("HuntRequest",std::bind(&HuntKill::huntdown,this,_1,_2));
        RCLCPP_INFO(this->get_logger(),"Hunt Service is online!!!");
        timer_=this->create_wall_timer((0.01s),std::bind(&HuntKill::catchturtle,this)); 
    
    }
private:
    void huntdown(const my_ros2_interfaces::srv::SpawnChase::Request::SharedPtr huntreq_,
                    my_ros2_interfaces::srv::SpawnChase::Response::SharedPtr huntresp_)
    {
        turtlesim::msg::Pose catch_pose;
        catch_pose.x = huntreq_->x;
        catch_pose.y = huntreq_->y;    
        catch_pose.theta = huntreq_->theta;
        RCLCPP_INFO(this->get_logger(),"catch_pose.x: %f, catch_pose.y: %f, catch_pose.theta: %f",catch_pose.x,catch_pose.y,catch_pose.theta);
        catch_registry[huntreq_->turtlename] = catch_pose;
        RCLCPP_INFO(this->get_logger()," Going to hunt %s ",huntreq_->turtlename.c_str());
        huntresp_->succees = true;
        huntresp_->msg ="Hunt request received";
        
        //dist_err = 1.0;
        //run_catchturtle = true;
     
        /*for(auto it = catch_registry.begin(); it!=catch_registry.end();++it)
        {

        }
        int huntsuccess_f=catchturtle();
        if(huntsuccess_f==1)
        {
            huntresp_->succees = true;
            huntresp_->msg = "Hunt Success!";
        }
        else
        {
            huntresp_->succees = true;
            huntresp_->msg = "Hunt failed";
        }*/
    }
    void MainTurtlePos(const turtlesim::msg::Pose position_)
    {
        turtle1pos_ = position_;
        RCLCPP_INFO(this->get_logger(),"Turtle1 Pose fetched, X:%f, Y:%f, Z:%f", turtle1pos_.x, turtle1pos_.y, turtle1pos_.theta);
    }
    void catchturtle()
    {
        auto it = catch_registry.begin();
        if(it != catch_registry.end())
        {
            float delta_x = it->second.x - turtle1pos_.x;
            float delta_y = it->second.y - turtle1pos_.y;
            float dist_err = std::sqrt((delta_x * delta_x) + (delta_y*delta_y));
            RCLCPP_INFO(this->get_logger(),"turtle1pos_.x: %f, turtle1pos_.y: %f, it->second.x: %f, it->second.y: %f, dist_err: %f"
                                            ,turtle1pos_.x,turtle1pos_.y, it->second.x,it->second.y,dist_err);
            float angle_to_target = std::atan2(delta_y,delta_x);
            float angle_error = angle_to_target - turtle1pos_.theta;
            if (angle_error > M_PI) angle_error -= 2 * M_PI;
            if (angle_error < -M_PI) angle_error += 2 * M_PI;
            
            if(dist_err>0.1)
            {
                cmdvel_.linear.x = dist_err*1.5;
                cmdvel_.angular.z = angle_error*2.0;
                Pubcmdvel_->publish(cmdvel_);
                RCLCPP_INFO(this->get_logger()," Turtle moving ");
            }
            else
            {
                RCLCPP_INFO(this->get_logger()," Gonna kIll %s",it->first.c_str());
                auto Killturtle_ = std::make_shared<turtlesim::srv::Kill::Request>();
                Killturtle_->name = it->first;
                RCLCPP_INFO(this->get_logger(),"%s ", Killturtle_->name.c_str());
                cmdvel_.linear.x = 0.0;
                cmdvel_.angular.z = 0.0;
                Pubcmdvel_->publish(cmdvel_);
                killclient_->async_send_request(Killturtle_,std::bind(&HuntKill::killcallback,this,_1));
                catch_registry.erase(it->first);
                RCLCPP_INFO(this->get_logger(),"%s : Kill Success", Killturtle_->name.c_str());
                //++it;
            }
        }
        //return 0;
    }
        /*float tar_a=catchreq_->x;
        float tar_b=catchreq_->y;
        float tar_c=catchreq_->theta;
        float delta_a = 5;
        float delta_b = 5;
        float delta_c = 5;
        float k = 0.1;

        delta_a = tar_a - curr_a;
        delta_b = tar_b - curr_b;
        delta_c = tar_c - curr_c;

        float new_a = curr_a + (k*delta_a);
        float new_b = curr_b + (k*delta_b);
        float new_c = curr_c + (k*delta_c);

        delta_a = tar_a - new_a;
        delta_b = tar_b - new_b;
        delta_c = tar_c - new_c;

        auto teleportreq_=std::make_shared<turtlesim::srv::TeleportAbsolute::Request>();
        
        teleportreq_->x = new_a;
        teleportreq_->y = new_b;
        teleportreq_->theta = new_c;
        teleportclient_->async_send_request(teleportreq_);
        
        RCLCPP_INFO(this->get_logger(),"Turtle1 Moving");
        
        curr_a=new_a;
        curr_b=new_b;
        curr_c=new_c;
        
        int huntsuccess_f = 1;
        auto killname_= std::make_shared<turtlesim::srv::Kill::Request>();
        killname_->name = catchreq_->turtlename;
        killclient_->async_send_request(killname_,std::bind(&HuntKill::killcallback,this,_1));
        RCLCPP_INFO(this->get_logger(),"%s Killed",killname_->name.c_str());
        return huntsuccess_f;

    }*/
    void killcallback(rclcpp::Client<turtlesim::srv::Kill>::SharedFuture future)
    {
       auto result = future.get();
    }


float dist_err;
//static bool run_catchturtle;
std::map<std::string, turtlesim::msg::Pose> catch_registry; 
turtlesim::msg::Pose turtle1pos_;
geometry_msgs::msg::Twist cmdvel_;
rclcpp::Service<my_ros2_interfaces::srv::SpawnChase>::SharedPtr huntservice_;
rclcpp::Client<turtlesim::srv::TeleportAbsolute>::SharedPtr teleportclient_;
rclcpp::Client<turtlesim::srv::Kill>::SharedPtr killclient_;
rclcpp::TimerBase::SharedPtr timer_;
rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr Pubcmdvel_;
rclcpp::Subscription<turtlesim::msg::Pose>::SharedPtr turtle1posSub_;
my_ros2_interfaces::srv::SpawnChase::Request::SharedPtr catchreq_;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<HuntKill>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}