cmake_minimum_required(VERSION 3.8)
project(agv_control)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(OpenCV REQUIRED)
find_package(cv_bridge REQUIRED)

add_executable(AGV_ObjectDetectionNode src/AGV_ObjectDetectionNode.cpp)
ament_target_dependencies(AGV_ObjectDetectionNode rclcpp sensor_msgs OpenCV cv_bridge)
add_executable(OpenCVDetctStopSign src/OpenCVDetctStopSign.cpp)
ament_target_dependencies(OpenCVDetctStopSign OpenCV)
add_executable(AGV_StampedVelocityConverter src/AGV_StampedVelocityConverter.cpp)
ament_target_dependencies(AGV_StampedVelocityConverter rclcpp geometry_msgs)


install(TARGETS
  AGV_ObjectDetectionNode
  OpenCVDetctStopSign
  AGV_StampedVelocityConverter
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()
