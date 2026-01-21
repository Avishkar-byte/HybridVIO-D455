# Build Instructions (Low Memory)

Since `catkin_make` can consume a lot of RAM and freeze the system, use the `-j` flag to limit parallel jobs.

## 1. Build the Workspace
Run this command in your terminal. If it still crashes, try replacing `-j2` with `-j1`.

```bash
cd ~/VIO
catkin_make -j2
```

## 2. Refresh Environment
After the build finishes successfully:

```bash
source devel/setup.bash
```

## 3. Run the System
Now you can launch the VIO pipeline:

```bash
roslaunch vio_launch rs_d455_vins.launch
```
