#!/usr/bin/env python3
import rospy
import yaml
from sensor_msgs.msg import CameraInfo
import sys

# Usage: rosrun vio_project create_calibration_files.py

class CalibrationDumper:
    def __init__(self):
        rospy.init_node('calibration_dumper', anonymous=True)
        self.left_inf = None
        self.right_info = None

        rospy.Subscriber("/camera/infra1/camera_info", CameraInfo, self.left_cb)
        rospy.Subscriber("/camera/infra2/camera_info", CameraInfo, self.right_cb)
        
        rospy.loginfo("Waiting for Camera Info topics...")

    def left_cb(self, msg):
        if not self.left_inf:
            self.left_inf = msg
            self.dump_yaml("left_d455.yaml", msg)

    def right_cb(self, msg):
        if not self.right_info:
            self.right_info = msg
            self.dump_yaml("right_d455.yaml", msg)

    def dump_yaml(self, filename, msg):
        data = {
            "model_type": "PINHOLE",
            "camera_name": "camera",
            "image_width": msg.width,
            "image_height": msg.height,
            "distortion_parameters": {
                "k1": msg.D[0],
                "k2": msg.D[1],
                "p1": msg.D[2],
                "p2": msg.D[3]
            },
            "projection_parameters": {
                "fx": msg.K[0],
                "fy": msg.K[4],
                "cx": msg.K[2],
                "cy": msg.K[5]
            }
        }
        
        # Check if D5 exists (k3)
        if len(msg.D) > 4:
             data["distortion_parameters"]["k3"] = msg.D[4]
             
        full_path = "/home/avishkar/VIO/config/" + filename
        with open(full_path, 'w') as outfile:
            outfile.write("%YAML:1.0\n")
            outfile.write("---\n")
            for key, value in data.items():
                if key in ["distortion_parameters", "projection_parameters"]:
                     outfile.write(f"{key}:\n")
                     for subkey, subval in value.items():
                         outfile.write(f"   {subkey}: {subval}\n")
                else:
                    outfile.write(f"{key}: {value}\n")
        
        rospy.loginfo(f"Saved {filename} to {full_path}")
        
        if self.left_inf and self.right_info:
            rospy.loginfo("Both files saved. You can kill this node.")
            rospy.signal_shutdown("Done")

if __name__ == '__main__':
    dumper = CalibrationDumper()
    rospy.spin()
