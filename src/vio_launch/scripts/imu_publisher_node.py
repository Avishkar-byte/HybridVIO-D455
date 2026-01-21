#!/usr/bin/env python3
import rospy
import pyrealsense2 as rs
from sensor_msgs.msg import Imu
import time
import numpy as np

def main():
    rospy.init_node('d455_imu_publisher', anonymous=True)
    pub = rospy.Publisher('/camera/imu', Imu, queue_size=200)
    
    pipeline = rs.pipeline()
    config = rs.config()
    
    # Configure IMU Streams (High Rate)
    config.enable_stream(rs.stream.accel, rs.format.motion_xyz32f, 200)
    config.enable_stream(rs.stream.gyro, rs.format.motion_xyz32f, 400)
    
    try:
        pipeline.start(config)
        rospy.loginfo("IMU Pipeline started")
    except Exception as e:
        rospy.logerr(f"Failed to start pipeline: {e}")
        return

    time.sleep(0.5) # Warmup
    
    # Timestamp alignment variables
    ts_offset = None

    try:
        while not rospy.is_shutdown():
            try:
                # Wait for frames
                frames = pipeline.wait_for_frames(timeout_ms=1000)
            except RuntimeError:
                continue

            accel = frames.first_or_default(rs.stream.accel)
            gyro = frames.first_or_default(rs.stream.gyro)

            if accel and gyro:
                # Synchronization Handling
                # Used to align RealSense Device Time (ms) to ROS Time (sec)
                # We assume the drift between Host and Device clock is negligible over short runs
                device_ts_sec = accel.get_timestamp() / 1000.0
                ros_now = rospy.Time.now().to_sec()
                
                if ts_offset is None:
                    ts_offset = ros_now - device_ts_sec
                    rospy.loginfo(f"IMU Timestamp Offset Synchronized: {ts_offset:.6f}")
                
                # Calculate corrected timestamp
                corrected_ts = device_ts_sec + ts_offset
                
                accel_data = accel.as_motion_frame().get_motion_data()
                gyro_data = gyro.as_motion_frame().get_motion_data()

                msg = Imu()
                # Use the corrected timestamp (Device Capture Time aligned to ROS Base)
                msg.header.stamp = rospy.Time.from_sec(corrected_ts)
                msg.header.frame_id = "camera_imu_optical_frame"
                
                msg.linear_acceleration.x = accel_data.x
                msg.linear_acceleration.y = accel_data.y
                msg.linear_acceleration.z = accel_data.z
                
                msg.angular_velocity.x = gyro_data.x
                msg.angular_velocity.y = gyro_data.y
                msg.angular_velocity.z = gyro_data.z
                
                msg.orientation_covariance[0] = -1
                pub.publish(msg)
                
    finally:
        pipeline.stop()

if __name__ == '__main__':
    main()
