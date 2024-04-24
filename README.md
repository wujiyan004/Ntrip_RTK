## LSLIDAR_CX_ 3.0_ROS 驱动说明

## 1.工程介绍

​		LSLIDAR_CX_3.0_ROS为linux环境下雷达ros驱动，适用于镭神C16/C32,2.6/2.8/3.0版本的雷达 ，程序在ubuntu18.04 ros melodic , ubuntu20.04 ros noetic 下测试通过。

## 2.适用范围：

* 适用于镭神c16、c32, 2.6\2.8\3.0版本的雷达

  

## 3.依赖

1.ubuntu18.04 ros melodic/ubuntu20.04 ros noetic

2.ros依赖

```bash
# 安装
sudo apt-get install ros-$ROS_DISTRO-pcl-ros ros-$ROS_DISTRO-pluginlib  ros-$ROS_DISTRO-pcl-conversions 
```

3.其他依赖

pcap,boost

~~~bash
sudo apt-get install libpcap-dev
sudo apt-get install libboost${BOOST_VERSION}-dev   #选择适合的版本
~~~



## 4.编译与运行：

~~~shell
mkdir -p ~/lslidar_ws/src
cd ~/lslidar_ws/src
把驱动压缩包拷贝到src目录下，并解压
cd ~/lslidar_ws
catkin_make
source devel/setup.bash
roslaunch lslidar_driver lslidar_c16.launch #启动c16雷达
~~~



## 5.launch 文件参数说明：

- c16为例

~~~xml
<launch>
  <arg name="device_ip" default="192.168.1.200" />  //雷达ip
  <arg name="msop_port" default="2368"/>   //数据包目的端口
  <arg name="difop_port" default="2369"/>   //设备包目的端口
  <arg name="use_gps_ts" default="false" />  //雷达是否使用gps或ptp授时，使用改为true
  <arg name="pcl_type" default="false" />   //点云类型，默认false点云中的点为xyzirt字段。改为true，点云中的点为xyzi字段。
  <arg name="lidar_type" default="c16"/>   //选择雷达型号,若为c32雷达改为c32
  <arg name="packet_size" default="1206"/>  //   udp包长1206或1212,若为1212字节雷达改为1212
  <arg name="c16_type" default="c16_2"/> //c16_2表示16线垂直角度分辨率为2度的雷达，c16_1表示16线垂直角度分辨率为1.33度的雷达
  <arg name="c32_type" default="c32_2"/> // c32_2表示32线垂直角度分辨率为1度的雷达，c32_1表示32线垂直角度分辨率为0.33度的雷达
  <arg name = "c32_fpga_type" default="3"/> //3表示32线fpga为\2.8\3.0的版本的雷达，2表示32线fpga为2.6的版本的雷达

  <node pkg="lslidar_driver" type="lslidar_driver_node" name="lslidar_driver_node" output="screen">
    <!--param name="pcap" value="$(find lslidar_driver)/pcap/123.pcap" /--> //取消注释（删除!-- --），启用离线pcap模式
    <param name="device_ip" value="$(arg device_ip)" />
    <param name="msop_port" value="$(arg msop_port)" />
    <param name="difop_port" value="$(arg difop_port)"/>
    <param name="pcl_type" value="$(arg pcl_type)"/>
    <param name="lidar_type" value="$(arg lidar_type)"/>
    <param name="packet_size" value="$(arg packet_size)"/>
    <param name="c16_type" value="$(arg c16_type)"/>
    <param name="c32_type" value="$(arg c32_type)"/>
    <param name="c32_fpga_type" value="$(arg c32_fpga_type)"/>
    <param name="add_multicast" value="false"/>  // 是否开启组播模式。
    <param name="group_ip" value="224.1.1.2"/>  //组播ip地址
    <param name="use_gps_ts" value="$(arg use_gps_ts)"/> 
    <param name="min_range" value="0.15"/>   //单位，米。雷达盲区最小值，小于此值的点被过滤
    <param name="max_range" value="150.0"/>  //单位，米。雷达盲区最大值 ，大于此值的点被过滤
    <param name="horizontal_angle_resolution" value="0.2"/>  //雷达水平角度分辨率
    <param name="frame_id" value="laser_link"/>  //坐标系id
    <param name="distance_unit" value="0.25"/>   //雷达距离分辨率
    <param name="angle_disable_min" value="0"/>   //雷达裁剪角度开始值 ，单位0.01°
    <param name="angle_disable_max" value="0"/>  //雷达裁剪角度结束值，单位0.01°
    <param name="packet_rate" value="840.0"/>    // 离线播放时参数，c16雷达为840.0（双回波1680.0），c32雷达1700.0（双回波3400.0）
    <param name="scan_num" value="10"/>          // laserscan线号
    <param name="read_once" value="false"/>      // pcap离线模式下,是否播放一次
    <param name="publish_scan" value="false"/>  //是否发布laserscan话题，发布改为true
    <param name="pointcloud_topic" value="lslidar_point_cloud"/> //点云话题名称，可修改
    <param name="coordinate_opt" value="false"/> //默认false 雷达零度角对应点云y轴,true雷达零度角对应点云x轴
  </node>

  <node pkg="rviz" type="rviz" name="rviz" args="-d $(find lslidar_driver)/rviz_cfg/lslidar.rviz"/>  // 运行驱动，同时打开rviz
 
 <!--node pkg="tf" type="static_transform_publisher" name="laser_link_to_world" args="0 0 1 0 0 0 world laser_link 100" /-->  //取消注释（删除!-- --）,静态坐标系转换 


~~~

### 组播模式：

- 上位机设置雷达开启组播模式

- 修改launch文件以下参数

  ~~~xml
      <param name="add_multicast" value="true"/> 
      <param name="group_ip" value="224.1.1.2"/>  //上位机设置的组播ip地址
  ~~~

- 运行以下指令将电脑加入组内（将指令中的enp2s0替换为用户电脑的网卡名,可用ifconfig查看网卡名)

  ~~~shell
  ifconfig
  sudo route add -net 224.0.0.0/4 dev enp2s0
  ~~~





### 离线pcap模式：

- 把录制好的pcap文件，拷贝到cx_4.0/src/lslidar_ros/lslidar_driver/pcap文件夹下。（cx_4.0是ros工作空间,根据实际工作空间修改）

- 修改launch文件以下参数

  ~~~xml
     //取消注释
     <param name="pcap" value="$(find lslidar_driver)/pcap/123.pcap"/>  // 123.pcap改为拷贝的pcap文件名
  
     <param name="read_once" value="false"/>      //默认false循环播放，只播放一次改为true
  ~~~



###  pcl点云类型：

- 修改launch文件以下参数

  ~~~xml
    <arg name="pcl_type" default="false" />   //点云类型，默认false点云中的点为xyzirt字段。改为true，点云中的点为xyzi字段。
  ~~~

  

- 默认false为自定义点云类型，定义参考lslidar_driver/include/lslidar_driver.h头文件

- 改为true,为pcl自带类型 :

  ~~~c++
  pcl::PointCloud<pcl::PointXYZI>
  ~~~

  

### c16 上下电功能（适用部分版本）：

- 通过ros service通信控制雷达上下电的功能。

- 另开终端，输入以下命令:   

  ~~~shell
  source devel/setup.bash  
  rosservice call /lslidarcontrol 0 # 下电
  rosservice call /lslidarcontrol 1 # 上电
  
  ~~~

  

## FAQ

Bug Report

Original version : LSLIDAR_CX_V3.2.2_220507_ROS

Modify:  original version

Date    : 2022-05-07

---------------



### 更新版本：

-----

- LSLIDAR_CX_V3.2.3_220520_ROS

- 更新说明: 优化c32雷达垂直角度校准



### 更新版本：

---------

- LSLIDAR_CX_V3.2.5_220729_ROS

- 更新说明:

  - 设备包和数据包在进秒时没对齐的情况下，增加判断，未及时进秒加1秒，提前进秒减1秒
  - 修复c32雷达点云缺失的bug
  - 未接收到设备包之前不发布点云      




### 更新版本：

------

- LSLIDAR_CX_V3.2.6_220905_ROS

- 更新说明：
  - 兼容1212字节版本的雷达， 通过修改launch文件参数 <arg name="packet_size" default="1206"/>  //   udp包长1206或1212,若为1212字节雷达改为1212



### 更新版本：

--------------

- LSLIDAR_CX_V3.2.7_221008_ROS

- 更新说明：
  - 修正laserscan类型话题方向，跟pointcloud2点云保持一致。

### 更新版本：

----------------

- LSLIDAR_CX_V3.2.8_221214_ROS

- 更新说明：

  - 优化1206字节雷达包的时间解析。
  - 数据包间隔时间通过两包相减插值求点的时间。

  

### 更新版本：

----------------

- LSLIDAR_CX_V3.2.9_221228_ROS
- 更新说明：
  - 修复C32雷达补偿角度为负数时导致点云分层的问题。



### 更新版本：

----------------

- LSLIDAR_CX_V3.3.1_230313_ROS

- 更新说明：
  - 降低cpu占用；boost库改为标准库；点的时间改为相对时间。
