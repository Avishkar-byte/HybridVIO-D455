# VIO Pipeline Design & Architecture

## 1. Hardware Mapping (RealSense D455)

The D455 provides the following streams relevant to VIO:
*   `/camera/infra1/image_rect_raw` (Left IR)
*   `/camera/infra2/image_rect_raw` (Right IR)
*   `/camera/color/image_raw` (RGB)
*   `/camera/depth/image_rect_raw` (Depth - computed)
*   `/camera/accel/sample`
*   `/camera/gyro/sample`

**Selected Streams for Pipeline:**
We will aim for a **Stereo-Inertial** configuration (using Infra1+Infra2+IMU) or **RGB-D Inertial** (Color+Depth+IMU).
*   *Recommendation:* **Stereo-Inertial** is often the gold standard for D455 in research as it avoids black-box depth artifacts, but **Color + Depth + IMU** is easier to debug visually. We will allow switching, but prioritize **Stereo + IMU** for robustness.

## 2. Data Flow Architecture

```mermaid
graph TD
    subgraph Driver
        CAM[RealSense D455] -->|/camera/infra1/image| IMG_L[Left Image]
        CAM -->|/camera/infra2/image| IMG_R[Right Image]
        CAM -->|/camera/imu| IMU_RAW[IMU (Accel+Gyro)]
    end

    subgraph Preprocessing
        IMU_RAW -->|Time Sync| SYNC[Visual-Inertial Alignment]
        IMG_L --> SYNC
        IMG_R --> SYNC
    end

    subgraph VIO_Core
        SYNC -->|Synced Data| FEAT[Feature Tracker]
        IMU_RAW -->|High Freq| PRED[IMU Pre-integration]
        FEAT -->|Feature Tracks| OPT[Sliding Window Optimization]
        PRED -->|Motion Constraints| OPT
        OPT -->|State Update| EST[State Estimator]
    end

    subgraph Output
        EST -->|/vio/odom| ODOM[Odometry Topic]
        EST -->|/tf| TF[TF Broadcaster]
        EST -->|/vio/path| PATH[Path Visualization]
    end
```

## 3. Coordinate Frames
Standard ROS REP-105 adherence:

1.  `world` / `map`: Fixed global frame. The origin is where VIO started.
2.  `odom`: Smoothed local frame (often same as `world` in VIO if no loop closure is used).
3.  `base_link`: Center of the robot/device.
4.  `camera_link`: Optical frame of the sensor.
5.  `imu_link`: Origin of the accelerometer/gyro.

**Transform Tree:**
`world` -> `odom` -> `base_link` -> (`camera_link` / `imu_link`)

## 4. State Vector Definition
The estimator will solve for the standard VINS state vector:
$$ X = [p, v, q, b_a, b_g] $$
*   $p$: Position in World (3x1)
*   $v$: Velocity in World (3x1)
*   $q$: Orientation (Quaternion) (4x1)
*   $b_a$: Accelerometer Bias (3x1)
*   $b_g$: Gyroscope Bias (3x1)

## 5. Synchronization Strategy
*   **Hardware Sync:** The D455 supports hardware synchronization where the IMU and images are timestamped on the same clock domain.
*   **Software Handling:** The VIO frontend must define a parameter `td` (time offset) to account for transmission delays or verify that `header.stamp` is strictly trusted.
