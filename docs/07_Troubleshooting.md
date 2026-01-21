# Troubleshooting Guide

## Common Issues and Solutions

### Issue: "Global Status: Error" in RViz

**Symptom**: RViz shows red "Error" status, nothing visible

**Causes**:
1. VINS node not running (most common)
2. Fixed frame "world" doesn't exist yet
3. Topics not being published

**Solution**:
1. **Check if VINS is running**:
   ```bash
   rostopic list | grep vins
   # Should see: /vins_estimator/odometry, /vins_estimator/path, etc.
   ```

2. **If no VINS topics exist**:
   - Check launch file output for errors
   - Verify `vins_node` exists: `ls -la ~/VIO/devel/lib/vins/vins_node`
   - Ensure workspace is sourced: `source ~/VIO/devel/setup.bash`

3. **Wait for initialization**:
   - VINS needs 5-10 seconds of static camera to initialize
   - Watch terminal for: "Initialization finish!"
   - Once initialized, "world" frame will appear and RViz error will clear

4. **Check TF tree**:
   ```bash
   rosrun tf view_frames
   # Should show world -> body -> camera transforms
   ```

### Issue: "Cannot locate node of type [vins_node]"

**Symptom**: Launch fails with node not found error

**Solution**:
1. **Rebuild workspace**:
   ```bash
   cd ~/VIO
   catkin_make
   source devel/setup.bash
   ```

2. **Verify executable exists**:
   ```bash
   ls -la devel/lib/vins/vins_node
   # Should show executable file
   ```

3. **Check permissions**:
   ```bash
   chmod +x devel/lib/vins/vins_node
   ```

### Issue: "Cannot locate node of type [loop_fusion_node]"

**Symptom**: Loop closure node fails to build/launch

**Solution**:
- **Loop closure is optional** - it's already disabled in the launch file
- VIO works fine without loop closure
- To enable later, fix compilation errors in `loop_fusion` package

### Issue: "wait for imu and image data"

**Symptom**: VINS terminal shows waiting message, never initializes

**Causes**:
1. Topic names don't match config
2. Camera not publishing
3. IMU not publishing

**Solution**:
1. **Check camera topics**:
   ```bash
   rostopic list | grep camera
   # Should see: /camera/infra1/image_rect_raw, /camera/imu
   ```

2. **Check topic names in config**:
   ```bash
   cat ~/VIO/src/vio_launch/config/realsense_d455_stereo_imu.yaml | grep topic
   # Should match actual topic names
   ```

3. **Verify data is flowing**:
   ```bash
   rostopic hz /camera/infra1/image_rect_raw  # Should be ~30 Hz
   rostopic hz /camera/imu                     # Should be ~200 Hz
   ```

4. **Check camera is connected**:
   ```bash
   lsusb | grep Intel
   # Should show RealSense D455
   ```

### Issue: No path visible in RViz

**Symptom**: RViz loads but no trajectory path appears

**Causes**:
1. VINS not initialized yet
2. Camera not moving
3. Fixed frame incorrect

**Solution**:
1. **Wait for initialization** (5-10 seconds static)
2. **Check path topic**:
   ```bash
   rostopic echo /vins_estimator/path
   # Should show path messages
   ```

3. **Verify fixed frame**:
   - In RViz: Global Options → Fixed Frame → set to `world`

4. **Move camera slowly** after initialization

### Issue: Segmentation fault

**Symptom**: VINS crashes immediately

**Causes**:
1. Invalid calibration file paths
2. Wrong image dimensions
3. Corrupted config file

**Solution**:
1. **Check calibration file paths are absolute**:
   ```bash
   cat ~/VIO/src/vio_launch/config/realsense_d455_stereo_imu.yaml | grep calib
   # Should show full paths like: /home/avishkar/VIO/src/vio_launch/config/left_d455.yaml
   ```

2. **Verify calibration files exist**:
   ```bash
   ls -la ~/VIO/src/vio_launch/config/*.yaml
   ```

3. **Check image dimensions match**:
   - Config: `image_width: 848, image_height: 480`
   - Camera: Should match (check with `rostopic echo /camera/infra1/camera_info`)

### Issue: High drift / poor accuracy

**Symptom**: Path drifts quickly, doesn't close loops

**Causes**:
1. Poor camera calibration
2. Incorrect IMU noise parameters
3. Fast motion
4. Low texture environment

**Solution**:
1. **Recalibrate cameras**:
   ```bash
   python3 ~/VIO/scripts/create_calibration_files.py
   ```

2. **Tune IMU noise** (in config file):
   ```yaml
   acc_n: 0.15  # Increase if accelerometer noisy
   gyr_n: 0.015 # Increase if gyroscope noisy
   ```

3. **Move slower** (< 0.5 m/s for testing)

4. **Ensure textured environment** (not blank walls)

### Issue: Initialization fails

**Symptom**: VINS never shows "Initialization finish!"

**Solution**:
1. **Keep camera completely still** for 10+ seconds
2. **Check IMU is working**:
   ```bash
   rostopic echo /camera/imu
   # Should show accel and gyro data
   ```

3. **Verify gravity magnitude**:
   - Config: `g_norm: 9.805` (adjust for your location if needed)

4. **Check for sufficient motion after static period**:
   - Need some motion to initialize (but start static)

### Issue: Build errors

**Symptom**: `catkin_make` fails

**Common errors**:
1. **OpenCV version mismatch**: VINS-Fusion may need specific OpenCV version
2. **Ceres Solver missing**: Install with `sudo apt-get install libceres-dev`
3. **Eigen3 missing**: Install with `sudo apt-get install libeigen3-dev`

**Solution**:
```bash
# Install dependencies
sudo apt-get update
sudo apt-get install libceres-dev libeigen3-dev libopencv-dev

# Rebuild
cd ~/VIO
catkin_make clean
catkin_make
source devel/setup.bash
```

## Quick Diagnostic Commands

```bash
# Check if VINS is running
rostopic list | grep vins

# Check camera topics
rostopic list | grep camera

# Monitor VINS output
rostopic echo /vins_estimator/odometry

# Check TF tree
rosrun tf view_frames
evince frames.pdf

# Check topic rates
rostopic hz /camera/infra1/image_rect_raw
rostopic hz /camera/imu
rostopic hz /vins_estimator/odometry

# View feature tracking
rosrun image_view image_view image:=/vins_estimator/image_track
```

## Still Having Issues?

1. **Check logs**: `~/.ros/log/` for detailed error messages
2. **Verify hardware**: Ensure D455 is connected via USB 3.0
3. **Check ROS master**: `roscore` must be running
4. **Verify workspace**: Always source `devel/setup.bash`
