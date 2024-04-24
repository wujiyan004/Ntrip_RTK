##Ntrip_RTK_ 1.0_ROS 驱动说明

## 1.工程介绍

​		Ntrip_RTK_ 1.0_ROS为linux环境下雷达ros驱动，适用于司南导航的K823的RTK板卡 ，程序在ubuntu18.04 ros melodic , ubuntu20.04 ros noetic 下测试通过。

## 2.适用范围：

* 适配于司南K823版本的板卡，同样适用其他厂家的RTK板卡
  

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
mkdir -p ~/catkin_ws/src
cd ~/lslidar_ws/src
把驱动压缩包拷贝到src目录下，并解压
cd ~/lslidar_ws
catkin_make
source devel/setup.bash
rosrun Ntrip_ROS Ntrip_ROS #启动差分接受
~~~



## 5.launch 文件参数说明：

- K823为例


移动站配置：
Log com2 gpybm ontime 0.2   //通过com2口输出GPYBM数据

Interfacemode com2 auto auto on  //t通过com2口输入差分数据

set imuaxestype 1             //设置K823内置的IMU设置K823芯片与车头朝向融合IMU

INSCONTROL ENABLE       //使能IMU与RTK融合算法

inscontrol enable              //使能IMU

set smootheddr on             //设置融合算法轨迹平滑，防止突变

set drtimeout 30                //设置融合超时

set drconfig 0.4                 //设置两天线距离

MAXVECLENERR 6            //设置模式配置

RTKDYNAMICS FOOT          //设置RTK模式为低速行走模式

RTKFIXHOLDTIME 10              //设置RTK差分超时

RTKTIMEOUT 9                   //设置RTK超时

RTKQUALITYLEVEL extra-safe        //设置RTK数据质量

VECTORLENMODE 2             //设置RTK两组数据最大间隔距离米

Saveconfig                       //保持以上设置，把配置写死在芯片，不写死，只生效一次下次不生效
 


基站配置：

freset                          恢复出厂设置

Fix auto/position 经度 纬度 高程    设置基站坐标（注：设置auto为自动获取坐标，position为固定点坐标）

log com1 rtcm1074b ontime 1 

log com1 rtcm1124b ontime 1 

log com1 rtcm1084b ontime 1 

log com1 rtcm1094b ontime 1

log com1 rtcm1005b ontime 5

log com1 rtcm1033b ontime 10

com com1 115200

Saveconfig

  

基准站经纬度坐标高程获取：
log bestposa ontime 1////获取当前位置坐标经纬度信息，通过记录一分钟的经纬度然后通过粒子滤波得到最有可能得经纬度。然后记录该经纬度给基站设置参考


如上图红框中为纬度，黄框中为经度，蓝框中为高程。
建议设置固定点坐标时，采集一段时间的位置坐标数据求出经纬度，高程的均值。以均值作为固定点坐标。另外上述坐标是以度为单位，注意单位之间的换算。  

## FAQ

Bug Report

Original version :CX_V3.2.2_220507_ROS

Modify:  original version

Date    : 2024-05-07


### 更新版本：
重要：此版本通过配置千寻或者中国移动等可直接获取差分数据，并且把获得的差分数据通过/dev/ttyUSB*给RTK设备，理论上支持所有RTK设备，另外由于我们专注做无人驾驶，所有我们优先使用GPYBM输出平面直角坐标，由于K823内部可以配置多种参数：
例如：
1.差分最长作用时效，设备做大运行速度，可以用来估计和收敛，防止由于一次定位失败导致车辆产生安全事故
------

GPYBM 报文定义（ver 3.0）
2016 年 8 月 18 日
1. 报头
 字符串类型，固定为“$GPYBM”
2. 设备序列号
 字符串类型，为 SN+8 位数字，即 SNxxxxxxxx（x 为 0~9 的数字）
 参见：Note1
3. UTC 时间（参见 GPGGA）
 字符串类型，格式：HHMMSS.SS，小数点后 2 位（单位为秒）、
 参见：Note1
4. 纬度(度)
 小数点后 9 位，带符号表示南北
5. 经度(度)
 小数点后 9 位，带符号表示东西
6. 高程(椭球高，m)
 小数点后 3 位
7. 方位角(度)
 小数点后 3 位
8. 俯仰角(度)
 小数点后 3 位
9. 北向速度(m/s)
 小数点后 3 位
10. 东向速度(m/s)
 小数点后 3 位
11. 地向速度(down, m/s)
 小数点后 3 位
12. 地速(ground speed, m/s)
 小数点后 3 位
13. 高斯投影坐标 X 轴(北向，单位 m，参见 PTNL,PJK：Northing)
 小数点后 3 位
14. 高斯投影坐标 Y 轴(东向，单位 m，参见 PTNL,PJK：Easting)
 小数点后 3 位
15. 基站坐标系下的移动站 X 轴坐标(基站坐标为原点，北向)
 小数点后 3 位
 参见：Note2
16. 基站坐标系下的移动站 Y 轴坐标(基站坐标为原点，东向)
 小数点后 3 位
 参见：Note2
17. 定位解状态（主站，即主天线的定位状态）
 Int 型
 0-未定位或无效解
 1-单点定位
 4-定位 RTK 固定解
 5-定位 RTK 浮点解
 6-INS 定位解或 GNSS/INS 组合定位解（2016-8-18 更新）
 参见：Note1
18. 定向解状态（从站，即从天线的定位状态）
 Int 型
 0-未定位或无效解
 1-单点定位
 4-定向 RTK 固定解
 5-定向 RTK 浮点解
 6-INS 姿态解或 GNSS/INS 组合姿态解（2016-8-18 更新）
 参见：Note1
19. 主站天线收星数
 Int 型
20. 差分延迟
 Int 型
21. 基准站 ID（2015-12-29 更新）
 Int 型，4 位
 参见：Note2
22. 主站和从站内之间的距离（双天线基线长）（2015-12-29 更新）
 小数点后 3 位
23. 从站参与解算的卫星数（2015-12-29 更新）
 Int 型
24. 横滚角(度) （2016-8-18 更新）
 小数点后 3 位
25. *校验和（两位）
报文格式（帧格式）：$aaccc,ddd,ddd,…,ddd*hh<CR><LF>
 $ 帧命令起始位
 aaccc 地址域，前两位为识别符，后三位为语句名
 ddd…ddd 数据
 * 校验和前缀
 hh 校验和（check sum）
$与*之间所有字符 ASCII 码的校验和(不包括这两个字符)，即：
各字节做异或运算，得到校验和后，再转换 16 进制格式的 ASCII 字符。
 <CR><LF> CR（Carriage Return，回车）+ LF（Line Feed，换行）帧结束
Note1：无论接收机定位与否（主要是指：与主天线连接的接收机主站），此项始终不为空
Note2：非差分模式下（即收不到基准站数据时），此项不输出（为空）
  - 优化服务时间解析。
  - 数据包间隔时间通过两包相减插值求点的时间。

### 更新版本：

----------------
- 更新说明：
  - 降低cpu占用；boost库改为标准库；点的时间改为相对时间。
