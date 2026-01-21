# VIO Quick Start Guide

## Quick Reference

### 1. Setup Environment
```bash
cd ~/VIO
source devel/setup.bash
```

### 2. Launch Full VIO System
```bash
# Terminal 1: roscore
roscore

# Terminal 2: Launch VIO
roslaunch vio_launch rs_d455_vins.launch

# Terminal 3: RViz
rviz -d rviz/vio_visualization.rviz
```

### 3. Verify Topics
```bash
rostopic list | grep vins
# Should see: /vins_estimator/odometry, /vins_estimator/path, etc.
```

## System Overview

**Hardware**: Intel RealSense D455
- Stereo IR cameras (infra1, infra2) @ 848x480, 30Hz
- IMU (gyro + accel) @ 200Hz

**Software**: VINS-Fusion (Stereo-Inertial mode)
- Feature tracking from stereo images
- IMU pre-integration for motion prediction
- Sliding window optimization
- Output: 6-DOF odometry (position + orientation)

## Key Topics

| Topic | Description |
|-------|-------------|
| `/camera/infra1/image_rect_raw` | Left IR camera |
| `/camera/infra2/image_rect_raw` | Right IR camera |
| `/camera/imu` | IMU data (gyro + accel) |
| `/vins_estimator/odometry` | **Main output**: 6-DOF pose |
| `/vins_estimator/path` | Trajectory path |
| `/vins_estimator/image_track` | Feature tracking visualization |

## First Run Checklist

1. **Connect D455** via USB 3.0
2. **Keep camera still** for 5-10 seconds (initialization)
3. **Move slowly** (< 0.5 m/s) for first test
4. **Check RViz** for path visualization
5. **Watch terminal** for "Initialization finish!" message

## Troubleshooting

**"wait for imu and image data"**
- Check: `rostopic list | grep camera`
- Verify camera is publishing

**No path in RViz**
- Check Fixed Frame is set to `world`
- Verify `/vins_estimator/path` topic exists

**Initialization fails**
- Keep camera still longer (10+ seconds)
- Check IMU: `rostopic echo /camera/imu`

## Documentation

- **Concepts**: `docs/01_VIO_Concepts.md`
- **Pipeline Design**: `docs/02_Pipeline_Design.md`
- **Recommendation**: `docs/03_Recommendation.md`
- **Implementation**: `docs/04_Implementation_Guide.md`
- **Testing**: `docs/05_Testing_Procedure.md`

## Project Structure

```
~/VIO/
├── src/VINS-Fusion/          # VIO algorithm
├── src/vio_launch/            # Launch files & configs
├── config/                    # Root configs
├── launch/                    # Root launch files
├── docs/                      # Documentation
├── rviz/                      # RViz configs
└── logs/                      # Output logs
```
