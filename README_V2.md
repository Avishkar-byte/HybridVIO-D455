# COMPLETE VIO SETUP GUIDE (RGB + Depth + IMU)

This guide integrates the RealSense D455 into a stable VIO pipeline using the **Hybrid V2 Architecture**.

## 1. Architecture
We use a robust "Hybrid" approach that bypasses typical hardware conflicts:
- **Images (Stereo)**: Handled by the standard `realsense-ros` driver (optimized C++).
- **IMU (Accel/Gyro)**: Handled by a custom Python node (`d455_imu_publisher`) using `librealsense2` directly.
- **Fusion**: VINS-Fusion combines these unsynchronized streams, estimating the time offset online.

## 2. Setup

### Workpace
Ensure your workspace is built:
```bash
cd ~/VIO
catkin_make
source devel/setup.bash
```

### Hardware Reset (If Needed)
If the camera is not found, run this Python script to force a hardware reset:
```bash
python3 ~/Test\ IMU/reset_device.py
```

## 3. Run the VIO Pipeline
Launch the unified system (Images + IMU + VINS + RViz):
```bash
roslaunch vio_launch rs_d455_vio_v2.launch
```

## 4. Validation
Open RViz to check the results:
```bash
rosrun rviz rviz -d ~/VIO/src/vio_launch/config/vins.rviz
```

You should see:
- `/vins_estimator/odometry`: Visual-Inertial Path.
- `/vins_estimator/imu_propagate`: High-rate IMU propagation (smooth).

## 5. Files
- **Launch**: `src/vio_launch/launch/rs_d455_vio_v2.launch`
- **IMU Node**: `src/vio_launch/scripts/imu_publisher_node.py`
- **Config**: `src/vio_launch/config/realsense_d455_stereo_imu_v2.yaml`
- **Extrinsics**: 
  - `src/vio_launch/config/left_d455_new.yaml`
  - `src/vio_launch/config/right_d455_new.yaml`

## 6. Important Notes
- **USB Bandwidth**: Ensure the device is on a USB 3.0 port.
- **Lighting**: VIO requires texture. Point the camera at a structured environment.
- **Initialization**: Move the camera slightly to excite the IMU/visual features at startup.
