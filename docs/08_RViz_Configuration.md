# RViz Configuration Guide

## Quick Setup

If you're using the provided config file:
```bash
rviz -d ~/VIO/rviz/vio_visualization.rviz
```

The config is pre-configured, but here's what each setting does:

---

## Essential Parameters

### 1. Fixed Frame (CRITICAL)
**Location**: Global Options → Fixed Frame

**Set to**: `world`

**Why**: VINS-Fusion publishes all data in the `world` frame. If this is wrong, you'll see "Global Status: Error" and nothing will display.

**How to set**:
- Click on "Global Options" in the left panel
- Find "Fixed Frame" dropdown
- Type or select: `world`

---

## Displays to Add

### 2. Path (Trajectory Visualization)
**Type**: `Path`

**Topic**: `/vins_estimator/path`

**Settings**:
- **Color**: Green (25, 255, 0) or your preference
- **Line Width**: 0.05
- **Buffer Length**: 1

**How to add**:
1. Click "Add" button (bottom left)
2. Select "Path" from list
3. Set Topic to `/vins_estimator/path`
4. Set Color to green

---

### 3. Odometry (Pose Visualization)
**Type**: `Odometry`

**Topic**: `/vins_estimator/odometry`

**Settings**:
- **Shape**: Arrow
- **Axes Length**: 0.2
- **Show Axes**: ✓
- **Show Trail**: ✓ (optional)
- **Keep**: 100 (number of poses to keep)

**How to add**:
1. Click "Add"
2. Select "Odometry"
3. Set Topic to `/vins_estimator/odometry`
4. Set Shape to "Arrow"
5. Enable "Show Axes"

---

### 4. Point Cloud (3D Features)
**Type**: `PointCloud`

**Topic**: `/vins_estimator/point_cloud`

**Settings**:
- **Size (m)**: 0.01
- **Style**: Points
- **Color Transformer**: Intensity (or FlatColor)
- **Decay Time**: 0 (show all points)

**How to add**:
1. Click "Add"
2. Select "PointCloud"
3. Set Topic to `/vins_estimator/point_cloud`
4. Set Size to 0.01

---

### 5. Feature Tracking Image
**Type**: `Image`

**Topic**: `/vins_estimator/image_track`

**Settings**:
- **Transport**: `raw`
- **Queue Size**: 2

**How to add**:
1. Click "Add"
2. Select "Image"
3. Set Topic to `/vins_estimator/image_track`
4. Set Transport to `raw`

---

### 6. TF (Transform Tree)
**Type**: `TF`

**Settings**:
- **Show Names**: ✓
- **Show Axes**: ✓
- **Marker Scale**: 0.3
- **Frame Timeout**: 15

**Frames to show**:
- `world` ✓
- `body` ✓
- `camera` ✓
- `imu_link` ✓

**How to add**:
1. Click "Add"
2. Select "TF"
3. Enable "Show Names" and "Show Axes"
4. Expand "Frames" and enable: world, body, camera, imu_link

---

### 7. Grid (Reference)
**Type**: `Grid`

**Settings**:
- **Plane**: XY
- **Plane Cell Count**: 10
- **Cell Size**: 1
- **Color**: Gray (160, 160, 164)

**How to add**:
1. Click "Add"
2. Select "Grid"
3. Set Plane to "XY"
4. Set Cell Count to 10

---

## Complete Display List

Your RViz should have these displays (in order):

1. ✅ **Grid** - Reference plane
2. ✅ **Axes** - Coordinate system
3. ✅ **TF** - Transform tree
4. ✅ **VIOGroup** (group containing):
   - **VIOPath** - `/vins_estimator/path`
   - **Odometry** - `/vins_estimator/odometry`
   - **CameraMarker** - `/vins_estimator/camera_pose_visual`
   - **PointCloud** - `/vins_estimator/point_cloud`
   - **HistoryPointCloud** - `/vins_estimator/margin_cloud`
   - **track_image** - `/vins_estimator/image_track`

---

## View Settings

### Camera View
**Type**: `ThirdPersonFollower`

**Settings**:
- **Distance**: 10 (meters)
- **Target Frame**: `<Fixed Frame>` (will use `world`)
- **Field of View**: 0.785 (45 degrees)

**How to set**:
1. Click "Views" panel
2. Select "Current View"
3. Set Type to "ThirdPersonFollower"
4. Adjust Distance slider

---

## Common Issues

### "Global Status: Error"
**Fix**: Set Fixed Frame to `world`

### No path visible
**Check**:
- Fixed Frame is `world`
- Topic `/vins_estimator/path` exists: `rostopic list | grep path`
- VINS has initialized (wait 5-10 seconds)

### No point cloud
**Check**:
- Topic `/vins_estimator/point_cloud` exists
- Size is not too small (try 0.05)
- Color Transformer is set correctly

### TF tree empty
**Check**:
- VINS is running: `rostopic list | grep vins`
- VINS has initialized (after static period)
- Check TF: `rosrun tf view_frames`

---

## Quick Verification

After setting up, verify topics exist:
```bash
rostopic list | grep vins
```

Should see:
- `/vins_estimator/odometry`
- `/vins_estimator/path`
- `/vins_estimator/point_cloud`
- `/vins_estimator/image_track`
- `/vins_estimator/camera_pose_visual`

---

## Save Configuration

Once configured:
1. File → Save Config As
2. Save to: `~/VIO/rviz/vio_visualization.rviz`

Then you can load it next time:
```bash
rviz -d ~/VIO/rviz/vio_visualization.rviz
```
