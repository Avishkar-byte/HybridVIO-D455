#include <ros/ros.h>
#include <sensor_msgs/Image.h>
#include <sensor_msgs/Imu.h>
#include <cv_bridge/cv_bridge.h>
#include <librealsense2/rs.hpp>
#include <mutex>
#include <iostream>

using namespace std;

class D455Driver {
public:
    D455Driver() : pnh_("~") {
        // Publishers
        pub_left_ = nh_.advertise<sensor_msgs::Image>("/camera/infra1/image_rect_raw", 10);
        pub_right_ = nh_.advertise<sensor_msgs::Image>("/camera/infra2/image_rect_raw", 10);
        pub_imu_ = nh_.advertise<sensor_msgs::Imu>("/camera/imu", 200);

        // Parameters
        setup_pipeline();
    }

    void run() {
        ROS_INFO("Starting D455 Custom Driver (Callback Mode)...");
        try {
            // Start with callback for high-rate IMU
            pipe_.start(cfg_, [&](const rs2::frame& frame) {
                try {
                    handle_frame(frame);
                } catch (const std::exception& e) {
                    ROS_WARN_THROTTLE(1.0, "Callback Exception: %s", e.what());
                }
            });
            
            ROS_INFO("Pipeline started. Streaming data...");
            ros::spin(); // Blocking main thread, callbacks run in background
            
        } catch (const rs2::error& e) {
            ROS_ERROR("RealSense Error: %s", e.what());
        } catch (const std::exception& e) {
            ROS_ERROR("Exception: %s", e.what());
        }
        
        pipe_.stop();
    }

private:
    ros::NodeHandle nh_, pnh_;
    ros::Publisher pub_left_, pub_right_, pub_imu_;
    
    rs2::pipeline pipe_;
    rs2::config cfg_;

    // IMU State
    rs2_vector last_accel_ = {0, 0, 0};
    bool has_accel_ = false;
    std::mutex imu_mutex_;

    void setup_pipeline() {
        // Enable Infra 1 & 2 (Left & Right)
        cfg_.enable_stream(RS2_STREAM_INFRARED, 1, 848, 480, RS2_FORMAT_Y8, 30);
        cfg_.enable_stream(RS2_STREAM_INFRARED, 2, 848, 480, RS2_FORMAT_Y8, 30);
        
        // Enable IMU
        cfg_.enable_stream(RS2_STREAM_ACCEL, RS2_FORMAT_MOTION_XYZ32F, 200);
        cfg_.enable_stream(RS2_STREAM_GYRO, RS2_FORMAT_MOTION_XYZ32F, 400);

        // Disable others
        cfg_.disable_stream(RS2_STREAM_COLOR);
        cfg_.disable_stream(RS2_STREAM_DEPTH);
    }

    void handle_frame(const rs2::frame& frame) {
        // RS2 callbacks can return individual frames (IMU) or composite framesets (Images)
        if (auto fs = frame.as<rs2::frameset>()) {
            handle_images(fs);
        } else if (auto mf = frame.as<rs2::motion_frame>()) {
            handle_imu(mf);
        }
    }

    void handle_images(const rs2::frameset& fs) {
        auto ir1 = fs.get_infrared_frame(1);
        auto ir2 = fs.get_infrared_frame(2);

        if (ir1) publish_image(ir1, pub_left_, "camera_infra1_optical_frame");
        if (ir2) publish_image(ir2, pub_right_, "camera_infra1_optical_frame"); 
    }

    void publish_image(const rs2::video_frame& frame, ros::Publisher& pub, const std::string& frame_id) {
        if (pub.getNumSubscribers() == 0) return;

        sensor_msgs::ImagePtr msg = cv_bridge::CvImage(
            std_msgs::Header(), 
            "mono8", 
            cv::Mat(cv::Size(frame.get_width(), frame.get_height()), CV_8UC1, (void*)frame.get_data())
        ).toImageMsg();

        msg->header.stamp = ros::Time(frame.get_timestamp() / 1000.0); 
        msg->header.frame_id = frame_id;
        pub.publish(msg);
    }

    void handle_imu(const rs2::motion_frame& mf) {
        std::lock_guard<std::mutex> lock(imu_mutex_);
        
        auto data = mf.get_motion_data();
        
        if (mf.get_profile().stream_type() == RS2_STREAM_ACCEL) {
            last_accel_ = data;
            has_accel_ = true;
        } else if (mf.get_profile().stream_type() == RS2_STREAM_GYRO) {
            // Publish IMU message on Gyro update (approx 400Hz)
            // Note: If using strict VIO, we might want to check dt.
            
            if (!has_accel_) return; 

            sensor_msgs::Imu msg;
            msg.header.stamp = ros::Time(mf.get_timestamp() / 1000.0);
            msg.header.frame_id = "camera_imu_optical_frame";
            
            msg.linear_acceleration.x = last_accel_.x;
            msg.linear_acceleration.y = last_accel_.y;
            msg.linear_acceleration.z = last_accel_.z;
            
            msg.angular_velocity.x = data.x;
            msg.angular_velocity.y = data.y;
            msg.angular_velocity.z = data.z;

            msg.orientation_covariance[0] = -1;
            
            pub_imu_.publish(msg);
        }
    }
};

int main(int argc, char** argv) {
    ros::init(argc, argv, "d455_driver");
    D455Driver driver;
    driver.run();
    return 0;
}
