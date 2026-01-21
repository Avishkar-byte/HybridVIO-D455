# VIO Project - Operations Manual

## 🚀 1. Primary Launch: Hybrid VIO (RGB + Depth + IMU)
This is the **fully functional** mode that enables the entire sensor suite of the D455. It uses a custom hybrid driver architecture to bypass hardware conflicts.

**Features Active:**
- **VIO:** Stereo-Inertial Odometry (VINS-Fusion).
- **Sensors:** Stereo IR, RGB Camera, Depth Camera, IMU (400Hz).

### Start the System
```bash
cd ~/VIO
source devel/setup.bash
roslaunch vio_launch rs_d455_hybrid.launch
```
*Note: You may see "Resource temporarily unavailable" warnings during startup. This is normal. Wait for "Initialization finish!"*

### Visualize
Open a new terminal:
```bash
cd ~/VIO
source devel/setup.bash
rosrun rviz rviz -d src/VINS-Fusion/config/vins_rviz_config.rviz
```

---

## 🛡️ 2. Fallback Launch: Stereo Visual Odometry
Use this **only** if the Hybrid VIO launch crashes or fails to initialize. It disables the IMU and runs in pure visual mode.

```bash
cd ~/VIO
source devel/setup.bash
roslaunch vio_launch rs_d455_vins.launch
```

---

## 🔍 3. Verification & Debugging
Run these commands in separate terminals to verify system health:

**Check VIO Output (Odometry):**
```bash
rostopic hz /vins_estimator/odometry
```
*Target: ~30-50 Hz*

**Check IMU Stream:**
```bash
rostopic hz /camera/imu
```
*Target: ~200-400 Hz (published by d455_imu_publisher)*

**Check Color Camera:**
```bash
rostopic hz /camera/color/image_raw
```
*Target: ~30 Hz*

---

## 🛠️ 4. Build & Maintenance
If you modify source code or configuration files:

```bash
cd ~/VIO
# Use -j2 to prevent memory exhaustion/freezes
catkin_make -j2
source devel/setup.bash
```

---

## 🛑 5. Stopping the System
1. Click in the terminal running `roslaunch`.
2. Press `Ctrl + C`.
3. Wait for all nodes to shut down.
4. If the camera seems "stuck" (LEDs on but no data next time), unplug and replug the USB cable.
