#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"

int main(){
    std::string path ="/home/nadeer133/Pictures/AGV_Camera.png";
    cv::Mat image = cv::imread(path);
    cv::imshow("image",image);
    cv::waitKey(0);
}