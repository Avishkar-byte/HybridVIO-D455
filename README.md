# HybridVIO-D455 🚀

![ROS Noetic](https://img.shields.io/badge/ROS-Noetic-blue)
![License](https://img.shields.io/badge/License-MIT-green)
![Platform](https://img.shields.io/badge/Platform-Intel%20RealSense%20D455-orange)
![Python](https://img.shields.io/badge/Python-3.8+-yellow)

**A Robust Visual-Inertial Odometry (VIO) Pipeline for Intel RealSense D455 on Linux.**

This project implements a **Hybrid Architecture** to overcome common hardware synchronization issues in the D455 ROS drivers. It fuses high-rate IMU data (accessed via Python/LibRealSense) with Stereo/RGB/Depth streams (via ROS) to drive **VINS-Fusion** for centimeter-level state estimation.

![VIO Tracking Demo](docs/demo_tracking.jpg)
*Real-time flight path tracking in VINS-Fusion.*

---

## 🌟 Key Features

*   **Hybrid Driver Architecture**: Bypasses the "Resource Busy" / IMU timeout bugs common in standard ROS wrappers by decoupling the IMU and Camera streams.
*   **Hardware-Time Synchronization**: Custom logic aligns Realsense hardware timestamps with ROS time to prevent VIO drift.
*   **Full Sensor Suite**: Enables **RGB + Depth + Stereo IR + IMU** simultaneously.
*   **Production Ready**: Includes separate launch files for Hybrid mode (Operations) and Stereo-Only mode (Fallback).

---

## 🛠️ Installation

### Prerequisites
*   Ubuntu 20.04 (ROS Noetic)
*   Intel RealSense SDK 2.0 (`librealsense2`)
*   `realsense-ros` wrapper
*   `ceres-solver`

### Build
```bash
# Clone this repository
git clone https://github.com/YOUR_USERNAME/HybridVIO-D455.git
cd HybridVIO-D455

# Build the workspace
catkin_make -j2
source devel/setup.bash
```

---

## 🚀 Usage

### 1. Preferred Method: Hybrid Launch
This launches the custom Python IMU node + standard ROS camera driver + VINS-Fusion.

```bash
roslaunch vio_launch rs_d455_hybrid.launch
```

### 2. Visualization
To see the path tracking, point clouds, and covariance:
```bash
rosrun rviz rviz -d src/VINS-Fusion/config/vins_rviz_config.rviz
```

### 3. Verification
Verify the distinct streams are active:
*   **Odometry (30Hz):** `rostopic hz /vins_estimator/odometry`
*   **IMU (200Hz):** `rostopic hz /camera/imu`
*   **RGB Stream:** `rostopic hz /camera/color/image_raw`

---

## 📂 Architecture Overview

| Component | Responsibility | Technology |
|-----------|----------------|------------|
| **Camera Driver** | Publishes Images (Infra, Color, Depth) | `realsense2_camera` (C++) |
| **IMU Driver** | Publishes Accel/Gyro at 200/400Hz | Custom `imu_publisher_node.py` (Python) |
| **State Estimator** | Fuses Visual + Inertial Data | `VINS-Fusion` (Optimization) |

---

## ⚠️ Troubleshooting

**"Resource temporarily unavailable"**
> You may see these warnings in the logs during startup. This is normal for the hybrid architecture as the two drivers negotiate USB endpoints. As long as you see "Initialization finish!", the system is working.

**Drift while stationary**
> Ensure the camera is static during the first 2 seconds of launch for IMU bias initialization.

---

## 📜 License
MIT License. See [LICENSE](LICENSE) for details.
