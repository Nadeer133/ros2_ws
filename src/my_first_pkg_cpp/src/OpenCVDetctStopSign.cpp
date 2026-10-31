#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/dnn.hpp>
#include <iostream>

using namespace std;

int main(){
    std::string path ="/home/nadeer133/Pictures/StopSign.png";
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
    
    /// ============================================================
// STEP 1: CONFIGURATION
// ============================================================

// COCO class ID for a stop sign is 11.
// Our YOLOv5 model is trained to recognize 80 COCO classes.
int STOP_SIGN_CLASS_ID = 11;

// Minimum combined confidence required to accept a detection.
// For example, 0.5 means the confidence must exceed 50%.
float CONDIFENCE_THRESHOLD = 0.5;

// IoU threshold used by Non-Maximum Suppression (NMS).
// If two boxes overlap significantly (IoU > 0.4),
// NMS suppresses the lower-confidence box.
float NMS_CONFIDENCE_THRESHOLD = 0.4;

// Vector to store bounding boxes of stop-sign candidates.
// cv::Rect stores (top-left X, top-left Y, width, height).
std::vector<cv::Rect> boxes;

// Vector to store the confidence score of each bounding box.
// boxes[i] and confidences[i] refer to the same candidate.
std::vector<float> confidences;


// ============================================================
// STEP 2: LOOP THROUGH ALL YOLO CANDIDATE PREDICTIONS
// ============================================================

// Our YOLOv5 output has the shape:
// [1, 25200, 85]
//
// Dimension 0: Number of images (1)
// Dimension 1: Number of candidate predictions (25200)
// Dimension 2: Values per candidate (85)
//
// We loop through all 25200 candidates.
// Using output.size[1] avoids hardcoding the candidate count.

for (int i = 0; i < output.size[1]; i++)
{
    // Access the 85 floating-point values of candidate i.
    //
    // First index (0) = First image in the batch.
    // Second index (i) = Current candidate prediction.
    //
    // ptr<float>() returns a pointer to the candidate's
    // first value, allowing access using prediction[index].

    const float* prediction = output.ptr<float>(0, i);


    // --------------------------------------------------------
    // STEP 3: EXTRACT BOUNDING BOX INFORMATION
    // --------------------------------------------------------

    // The first four values describe the predicted bounding box.
    //
    // YOLOv5 represents bounding boxes using:
    // [center_x, center_y, width, height]
    //
    // These coordinates refer to the model's input image
    // coordinate system (640 x 640 in our case).

    float x_center = prediction[0];
    float y_center = prediction[1];
    float width    = prediction[2];
    float height   = prediction[3];


    // --------------------------------------------------------
    // STEP 4: EXTRACT OBJECTNESS SCORE
    // --------------------------------------------------------

    // prediction[4] contains the objectness score.
    //
    // Objectness indicates how strongly YOLO predicts
    // that this candidate bounding box contains an object.
    //
    // It does NOT tell us which object is present.

    float objectness = prediction[4];


    // --------------------------------------------------------
    // STEP 5: IDENTIFY THE MOST LIKELY OBJECT CLASS
    // --------------------------------------------------------

    // Each candidate contains 80 class scores.
    //
    // prediction[5]  = Class 0 score  (Person)
    // prediction[6]  = Class 1 score  (Bicycle)
    // prediction[7]  = Class 2 score  (Car)
    // ...
    // prediction[16] = Class 11 score (Stop sign)
    // ...
    // prediction[84] = Class 79 score
    //
    // We examine all 80 class scores and find the highest.

    // Initially, no class has been selected.
    int best_class_id = -1;

    // Stores the highest class score found so far.
    float best_class_score = 0;

    for (int j = 0; j < 80; j++)
    {
        // The first five values are reserved for:
        // x, y, width, height, and objectness.
        //
        // Therefore, class j is located at prediction[j + 5].

        if (prediction[j + 5] > best_class_score)
        {
            // Update the highest class score.
            best_class_score = prediction[j + 5];

            // Store the corresponding class ID.
            best_class_id = j;
        }
    }


    // --------------------------------------------------------
    // STEP 6: CALCULATE COMBINED DETECTION CONFIDENCE
    // --------------------------------------------------------

    // Combined confidence = Objectness * Best class score
    //
    // Example:
    // Objectness       = 0.90
    // Best class score = 0.95
    //
    // Confidence = 0.90 * 0.95 = 0.855

    float confidence = best_class_score * objectness;


    // --------------------------------------------------------
    // STEP 7: FILTER STOP-SIGN CANDIDATES
    // --------------------------------------------------------

    // Keep this candidate only if:
    //
    // 1. Its highest-scoring class is Stop sign (class ID 11).
    // 2. Its combined confidence exceeds our threshold (0.5).
    //
    // Candidates that fail either condition are ignored.

    if (best_class_id == STOP_SIGN_CLASS_ID &&
        confidence > CONDIFENCE_THRESHOLD)
    {
        // ----------------------------------------------------
        // STEP 8: CONVERT BOUNDING BOX COORDINATES
        // ----------------------------------------------------

        // YOLOv5 provides:
        // (center_x, center_y, width, height)
        //
        // OpenCV's cv::Rect expects:
        // (top_left_x, top_left_y, width, height)
        //
        // Therefore:
        //
        // top_left_x = center_x - width / 2
        // top_left_y = center_y - height / 2

        int top = static_cast<int>(y_center - height / 2);
        int left = static_cast<int>(x_center - width / 2);

        // static_cast<int>() converts floating-point values
        // into integers by truncating the decimal part.


        // ----------------------------------------------------
        // STEP 9: STORE BOUNDING BOX AND CONFIDENCE
        // ----------------------------------------------------

        // Create a cv::Rect object directly inside the vector.
        //
        // emplace_back() constructs and appends a new rectangle
        // using (left, top, width, height).

        boxes.emplace_back(
            left,
            top,
            static_cast<int>(width),
            static_cast<int>(height)
        );

        // Store the confidence of the same candidate.
        //
        // Example:
        // boxes[0]       = Bounding box A
        // confidences[0] = Confidence of bounding box A

        confidences.push_back(confidence);
    }
}


// ============================================================
// STEP 10: NON-MAXIMUM SUPPRESSION (NMS)
// ============================================================

// Multiple candidates may predict overlapping bounding boxes
// around the SAME stop sign.
//
// NMS removes redundant overlapping boxes and keeps
// the strongest detections.
//
// selected_indices stores the indices of the boxes
// that survive NMS.
//
// These indices refer to positions in our boxes vector,
// NOT the original 25200 YOLO candidate indices.

std::vector<int> selected_indices;

cv::dnn::NMSBoxes(
    boxes,                      // Candidate bounding boxes
    confidences,                // Corresponding confidence scores
    CONDIFENCE_THRESHOLD,       // Minimum confidence score
    NMS_CONFIDENCE_THRESHOLD,   // Maximum allowed overlap (IoU)
    selected_indices            // Indices of surviving boxes
);


// ============================================================
// STEP 11: FINAL STOP-SIGN DETECTION RESULT
// ============================================================

// selected_indices.empty() returns:
// true  -> No bounding boxes survived NMS.
// false -> At least one bounding box survived NMS.
//
// Therefore, if selected_indices is NOT empty,
// at least one stop sign has been detected.

    if (!selected_indices.empty())
    {
        cout << "Stop Sign detected" << endl;
    }   
    else
    {
        cout << "Stop sign not detected" << endl;
    }

    
    cv::imshow("image",image);
    cv::waitKey(0);
}