#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/dnn.hpp>
#include <iostream>

int main(){
    std::string path ="/home/nadeer133/Pictures/AGV_Camera.png";
    cv::Mat image = cv::imread(path);
    std::string model_path = "TrainedModels/yolov5n_opencv_dnn.onnx";
    cv::Mat input_tensor_=cv::dnn::blobFromImage(
        image,
        1.0/255.0, //scaling down 0-255 to 0-1
        cv::Size(640,640), //resizing the image height and width
        cv::Scalar(0), //mean value to be subtracted
        true,  //swap red and blue channels(bgr->rgb)
        false  // do not crop
    );

    cv::dnn::Net net;
    try
    {
        net = cv::dnn::readNetFromONNX(model_path); //Net is an opencv class representing a deep neural network

        if(net.empty()){
            std::cerr<<"failed to load YOLO model"<<std::endl;
        }

        std::cout<<"YOLO model loaded succesfully"<<std::endl;
    }
    catch(const cv::Exception& e)
    {
        std::cerr<<"OpenCV Error: "<< e.what() << std::endl; //e.what() returns a description of the error
        return 1;
    }

    net.setInput(input_tensor_); //providing input tensor to yolo model.
    cv::Mat output=net.forward(); //execute the model and obtain predictions.
    std::cout << "Inference completed!" << std::endl;
    std::cout << "Output dimensions: " << output.dims << std::endl;

    for (int i = 0; i < output.dims; i++)
    {
        std::cout << "Dimension " << i << ": "
                  << output.size[i] << std::endl;
    }

    cv::imshow("image",image);
    cv::waitKey(0);
}