cmake_minimum_required(VERSION 3.0.2)
project(d455_driver)

find_package(catkin REQUIRED COMPONENTS
  roscpp
  sensor_msgs
  cv_bridge
  image_transport
  std_msgs
)

find_package(realsense2 REQUIRED)

catkin_package(
  CATKIN_DEPENDS roscpp sensor_msgs cv_bridge image_transport std_msgs
)

include_directories(
  ${catkin_INCLUDE_DIRS}
  ${realsense2_INCLUDE_DIRS}
)

add_executable(d455_node src/d455_node.cpp)
target_link_libraries(d455_node
  ${catkin_LIBRARIES}
  ${realsense2_LIBRARY}
)
