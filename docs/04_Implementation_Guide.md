# VIO Implementation Guide - Step by Step

## Overview
This guide walks through the complete implementation of a Visual-Inertial Odometry (VIO) system using Intel RealSense D455 and VINS-Fusion on ROS Noetic.

## System Architecture

### Hardware Components
- **Intel RealSense D455**: Provides stereo IR cameras (infra1, infra2) and IMU (gyro + accel)
- **Topics Published**:
  - `/camera/infra1/image_rect_raw` - Left IR camera (848x480 @ 30Hz)
  - `/camera/infra2/image_rect_raw` - Right IR camera (848x480 @ 30Hz)
  - `/camera/imu` - Combined IMU data (gyro @ 200Hz, accel @ 250Hz)

### Software Stack
1. **Driver**: `realsense2_camera` (ROS wrapper)
2. **VIO Core**: `VINS-Fusion` (stereo-inertial mode)
3. **Visualization**: RViz with custom configuration

## Project Structure

```
~/VIO/
├── build/              # Catkin build artifacts
├── devel/              # Development space (setup.bash here)
├── src/
│   ├── VINS-Fusion/   # VIO algorithm implementation
│   └── vio_launch/     # Custom launch files and configs
│       ├── config/
│       │   ├── left_d455.yaml          # Left camera intrinsics
│       │   ├── right_d455.yaml         # Right camera intrinsics
│       │   └── realsense_d455_stereo_imu.yaml  # VINS config
│       └── launch/
│           └── rs_d455_vins.launch     # Main launch file
├── config/             # Root-level configs
│   └── realsense_d455_stereo_imu.yaml  # Alternative config
├── launch/             # Root-level launch files
│   └── realsense_vio.launch            # Camera-only launch
├── docs/               # Documentation
├── logs/               # VIO output logs
└── scripts/            # Utility scripts
```

## Key Concepts Explained

### 1. Visual Odometry (VO) vs Visual-Inertial Odometry (VIO)

**Visual Odometry (VO)**:
- Uses only camera images
- Monocular VO: scale ambiguity (can't tell if object is 1m or 10m away)
- Stereo VO: scale known from baseline, but computationally expensive
- Fails in: low texture, fast motion (blur), pure rotation

**Visual-Inertial Odometry (VIO)**:
- Fuses camera + IMU measurements
- **IMU provides**:
  - High-frequency motion prediction (200-1000Hz vs 30Hz camera)
  - Scale observability (gravity + acceleration integration)
  - Robustness during fast motion (IMU holds state when images blur)
- **Camera provides**:
  - Absolute position constraints (feature matching)
  - Loop closure capability
  - Drift correction

### 2. Why Stereo + IMU for D455?

The RealSense D455 has:
- **Two IR cameras** (infra1, infra2) with hardware synchronization
- **IMU** (BMI085) with hardware timestamping
- **Pre-computed depth** (optional, but we use raw stereo for VIO)

**Stereo VIO advantages**:
- Known scale from baseline (95mm on D455)
- Works in textureless areas better than monocular
- More robust than RGB-D (depth has artifacts, limited range)

### 3. How IMU Fusion Works

```
Time: t0 -------- t1 -------- t2
      |           |           |
Camera: [Frame0] [Frame1] [Frame2]  (30Hz)
IMU:    [||||||||||||||||||||||]   (200Hz)

Between camera frames:
1. IMU pre-integration: Integrate gyro (rotation) and accel (translation)
2. Predict camera pose at t1 using IMU
3. Camera frame arrives: Extract features, match with previous frame
4. Optimization: Minimize reprojection error + IMU prediction error
5. Update state: Position, velocity, orientation, IMU biases
```

**State Vector** (what VIO estimates):
- **p** (3x1): Position in world frame
- **v** (3x1): Velocity in world frame  
- **q** (4x1): Orientation quaternion
- **b_a** (3x1): Accelerometer bias
- **b_g** (3x1): Gyroscope bias

### 4. Coordinate Frames (ROS REP-105)

```
world (fixed) 
  └── odom (smoothed, same as world in VIO without loop closure)
       └── base_link (robot center)
            ├── camera_link (optical center of left IR camera)
            └── imu_link (IMU sensor frame)
```

VINS-Fusion publishes:
- `/vins_estimator/odometry` (frame_id: "world", child_frame_id: "world")
- TF: `world` → `body` → `camera`

## Configuration Files Explained

### Camera Calibration Files (`left_d455.yaml`, `right_d455.yaml`)

These contain **intrinsic parameters**:
- **fx, fy**: Focal length in pixels
- **cx, cy**: Principal point (image center)
- **k1, k2, p1, p2**: Distortion coefficients (radial + tangential)

**How to obtain**:
1. Use RealSense's built-in calibration (from `/camera/infra1/camera_info`)
2. Or use `scripts/create_calibration_files.py` to extract from ROS topics
3. Or use Kalibr for high-precision calibration

### VINS Configuration (`realsense_d455_stereo_imu.yaml`)

**Key Parameters**:

```yaml
# Sensor Configuration
imu: 1                    # Use IMU
num_of_cam: 2             # Stereo setup
imu_topic: "/camera/imu"
image0_topic: "/camera/infra1/image_rect_raw"
image1_topic: "/camera/infra2/image_rect_raw"

# Feature Tracking
max_cnt: 150              # Max features per frame
min_dist: 30              # Min distance between features (pixels)
freq: 10                  # Tracking result publish rate (Hz)

# IMU Noise Parameters (tuned for D455's BMI085)
acc_n: 0.1                # Accelerometer noise std dev
gyr_n: 0.01               # Gyroscope noise std dev
acc_w: 0.001              # Accel bias random walk
gyr_w: 0.0001             # Gyro bias random walk
g_norm: 9.805             # Gravity magnitude (m/s²)

# Extrinsic Calibration
estimate_extrinsic: 1     # Online estimation (set to 0 after calibration)
body_T_cam0: ...          # Initial guess: Camera-to-IMU transform
```

## Launch Files

### 1. Camera Only (`launch/realsense_vio.launch`)

Launches only the RealSense driver. Use for:
- Testing camera connectivity
- Extracting calibration data
- Debugging sensor issues

### 2. Full VIO (`src/vio_launch/launch/rs_d455_vins.launch`)

Launches:
1. RealSense driver (stereo IR + IMU)
2. VINS-Fusion estimator
3. Loop closure (optional)

## Topics Published by VINS-Fusion

| Topic | Type | Description |
|-------|------|-------------|
| `/vins_estimator/odometry` | `nav_msgs/Odometry` | Main odometry output (position, velocity, orientation) |
| `/vins_estimator/path` | `nav_msgs/Path` | Trajectory path for visualization |
| `/vins_estimator/image_track` | `sensor_msgs/Image` | Feature tracking visualization |
| `/vins_estimator/point_cloud` | `sensor_msgs/PointCloud` | 3D feature points |
| `/vins_estimator/camera_pose` | `nav_msgs/Odometry` | Camera pose in world frame |
| `/vins_estimator/key_poses` | `visualization_msgs/Marker` | Keyframe poses |

## Next Steps

See `05_Testing_Procedure.md` for step-by-step testing instructions.
