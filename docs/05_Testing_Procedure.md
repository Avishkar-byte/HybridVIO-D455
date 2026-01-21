# VIO Testing and Validation Procedure

## Prerequisites Checklist

Before testing, ensure:
- [ ] RealSense D455 is connected via USB 3.0
- [ ] ROS Noetic is sourced: `source /opt/ros/noetic/setup.bash`
- [ ] Workspace is sourced: `source ~/VIO/devel/setup.bash`
- [ ] Camera calibration files exist and have correct paths
- [ ] Logs directory exists: `mkdir -p ~/VIO/logs/pose_graph`

## Step 1: Verify Camera Connectivity

### 1.1 Check USB Connection
```bash
lsusb | grep Intel
# Should show: Intel Corp. RealSense D455
```

### 1.2 Test RealSense Driver
```bash
# Terminal 1: Start roscore
roscore

# Terminal 2: Launch camera
roslaunch vio_launch realsense_vio.launch
# OR use the root launch file:
roslaunch realsense_vio realsense_vio.launch

# Terminal 3: Check topics
rostopic list | grep camera
```

**Expected Topics**:
- `/camera/infra1/image_rect_raw`
- `/camera/infra2/image_rect_raw`
- `/camera/imu`
- `/camera/infra1/camera_info`
- `/camera/infra2/camera_info`

### 1.3 Verify Image Streams
```bash
# View left IR camera
rosrun image_view image_view image:=/camera/infra1/image_rect_raw

# View right IR camera
rosrun image_view image_view image:=/camera/infra2/image_rect_raw

# Check IMU data
rostopic echo /camera/imu | head -20
```

**What to check**:
- Images are not black/blank
- IMU messages arrive at ~200Hz (check timestamps)
- No error messages in camera launch terminal

## Step 2: Extract Camera Calibration (If Needed)

If calibration files are missing or incorrect:

```bash
# Terminal 1: Launch camera
roslaunch realsense_vio realsense_vio.launch

# Terminal 2: Run calibration extractor
cd ~/VIO
python3 scripts/create_calibration_files.py

# Wait for both left and right camera info messages
# Files will be saved to ~/VIO/config/
```

**Verify calibration files**:
```bash
cat ~/VIO/src/vio_launch/config/left_d455.yaml
cat ~/VIO/src/vio_launch/config/right_d455.yaml
```

**Check values**:
- `fx`, `fy` should be ~400-500 for 848x480 resolution
- `cx`, `cy` should be ~424, 240 (half of width/height)
- Distortion parameters should be small (< 0.1)

## Step 3: Launch Full VIO System

### 3.1 Start VIO
```bash
# Terminal 1: roscore
roscore

# Terminal 2: Launch full VIO
roslaunch vio_launch rs_d455_vins.launch

# Terminal 3: Monitor topics
rostopic list | grep vins
```

**Expected VINS Topics**:
- `/vins_estimator/odometry`
- `/vins_estimator/path`
- `/vins_estimator/image_track`
- `/vins_estimator/point_cloud`

### 3.2 Check VINS Initialization

Watch the VINS terminal for:
```
[ INFO] [vins_estimator]: wait for imu and image data...
[ INFO] [vins_estimator]: Initialization finish!
```

**Common Issues**:
- **"wait for imu and image data"**: Check topic names match config file
- **"no extrinsic param"**: Set `estimate_extrinsic: 1` in config
- **Segmentation fault**: Check calibration file paths are absolute

## Step 4: Visualize in RViz

### 4.1 Launch RViz
```bash
# Terminal 4: RViz
rviz
```

### 4.2 Configure RViz Displays

**Add these displays**:

1. **Path**:
   - Type: `Path`
   - Topic: `/vins_estimator/path`
   - Color: Green (or your preference)

2. **Odometry**:
   - Type: `Odometry`
   - Topic: `/vins_estimator/odometry`
   - Show Axes: ✓
   - Show Trail: ✓

3. **Point Cloud**:
   - Type: `PointCloud`
   - Topic: `/vins_estimator/point_cloud`
   - Size: 0.01

4. **Image** (Feature Tracking):
   - Type: `Image`
   - Topic: `/vins_estimator/image_track`
   - Transport: `raw`

5. **TF**:
   - Type: `TF`
   - Show Names: ✓
   - Show Axes: ✓

**Fixed Frame**: Set to `world`

### 4.3 Save RViz Config
- File → Save Config As → `~/VIO/rviz/vio_visualization.rviz`

## Step 5: Motion Testing

### 5.1 Static Test (Initialization)
1. Keep camera **completely still** for 5-10 seconds
2. VINS needs static initialization to estimate gravity and IMU biases
3. Watch for: "Initialization finish!" message

### 5.2 Slow Motion Test
1. Move camera **slowly** (< 0.5 m/s) in a straight line
2. **Expected**: Path should be smooth, no jumps
3. **Check**: Odometry position should increase linearly

### 5.3 Rotation Test
1. Rotate camera around vertical axis (yaw)
2. **Expected**: Path should show circular motion
3. **Check**: Orientation in odometry should change smoothly

### 5.4 Full Motion Test
1. Move camera in a **figure-8** or **square** pattern
2. **Expected**: Path should close (return to start)
3. **Check**: Final position error < 10% of total distance

## Step 6: Validation Metrics

### 6.1 Check Odometry Frequency
```bash
rostopic hz /vins_estimator/odometry
# Should be ~10-30 Hz (depends on config freq parameter)
```

### 6.2 Check Feature Tracking
```bash
# View feature tracking image
rosrun image_view image_view image:=/vins_estimator/image_track
```

**What to see**:
- Green/red dots: tracked features
- Lines: feature tracks between frames
- Should have 50-150 features per frame

### 6.3 Check IMU Integration
```bash
# Monitor IMU data rate
rostopic hz /camera/imu
# Should be ~200 Hz
```

### 6.4 Check Logs
```bash
# VINS saves trajectory to:
ls -lh ~/VIO/logs/*.csv

# Check latest trajectory file
tail -20 ~/VIO/logs/*.csv
```

**Format**: `timestamp, x, y, z, qw, qx, qy, qz`

## Step 7: Troubleshooting

### Issue: "wait for imu and image data"
**Solution**:
- Check topic names: `rostopic list`
- Verify config file topic names match
- Ensure camera is publishing: `rostopic echo /camera/imu`

### Issue: Path drifts quickly
**Possible causes**:
- Poor camera calibration (recalibrate)
- Incorrect IMU noise parameters (tune `acc_n`, `gyr_n`)
- Fast motion (slow down)
- Low texture environment (add visual features)

### Issue: Initialization fails
**Solution**:
- Keep camera still longer (10+ seconds)
- Check IMU is working: `rostopic echo /camera/imu`
- Verify gravity magnitude: `g_norm: 9.805` (adjust for your location)

### Issue: Segmentation fault
**Solution**:
- Check calibration file paths are absolute
- Verify calibration files are valid YAML
- Check image dimensions match config

### Issue: No features detected
**Solution**:
- Ensure environment has texture (not blank wall)
- Check `max_cnt` parameter (increase if needed)
- Verify images are not black: `rosrun image_view image_view image:=/camera/infra1/image_rect_raw`

## Step 8: Performance Tuning

### 8.1 Adjust Feature Tracking
In `realsense_d455_stereo_imu.yaml`:
```yaml
max_cnt: 200        # Increase for more features (slower)
min_dist: 20        # Decrease for denser features
```

### 8.2 Adjust Optimization
```yaml
max_solver_time: 0.04    # Increase for better accuracy (slower)
max_num_iterations: 10   # Increase iterations
```

### 8.3 IMU Noise Tuning
If drift is high:
```yaml
acc_n: 0.15         # Increase if accelerometer is noisy
gyr_n: 0.015        # Increase if gyroscope is noisy
```

## Step 9: Record and Replay

### 9.1 Record Data
```bash
rosbag record -O vio_test.bag \
  /camera/infra1/image_rect_raw \
  /camera/infra2/image_rect_raw \
  /camera/imu \
  /vins_estimator/odometry \
  /vins_estimator/path
```

### 9.2 Replay for Testing
```bash
# Terminal 1: roscore
roscore

# Terminal 2: Launch VINS
roslaunch vio_launch rs_d455_vins.launch

# Terminal 3: Play bag
rosbag play vio_test.bag
```

## Success Criteria

✅ **System is working if**:
1. VINS initializes within 10 seconds of static period
2. Path visualization shows smooth motion
3. Odometry publishes at 10+ Hz
4. Feature tracking shows 50+ features per frame
5. Path closes within 10% error for closed-loop motion
6. No crashes or segmentation faults

## Next Steps

Once validated:
1. **Extrinsic Calibration**: Use Kalibr to get accurate camera-IMU transform
2. **Loop Closure**: Enable loop closure for long-term accuracy
3. **Mapping**: Integrate with mapping packages (voxblox, octomap)
4. **SLAM**: Extend to full SLAM with loop closure
