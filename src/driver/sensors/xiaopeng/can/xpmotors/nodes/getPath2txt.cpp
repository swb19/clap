/**
 * @file auto_state_node.cpp
 * @brief sub msgs:
 * 	Auto state, EPS status, ESC status，Imu msgs,
 *       pub msgs:
 *  Crtolmsgs
 * @author xiang.zhang@novauto.com.cn
 * @version 0.0.1
 * @date 2020-1-12
 */
#include <iostream>
#include <fstream>
#include <string>
#include<cmath>
//ros headfile
#include "ros/ros.h"
#include "std_msgs/String.h"
#include <signal.h>
#include "param.h"
#include <sensor_msgs/NavSatFix.h>
#include <sensor_msgs/Imu.h>
#include <geometry_msgs/TwistWithCovarianceStamped.h>
#include <nav_msgs/Odometry.h>
//persnal headfile
#include "xpmotors_can_msgs/AutoCtlReq.h"
#include "xpmotors_can_msgs/AutoStateEx.h"
#include "xpmotors_can_msgs/AutoState.h"
#include "xpmotors_can_msgs/ESCStatus.h"
#include "xpmotors_can_msgs/EPSStatus.h"

using namespace std;
double PI= 3.141592654;

double vehiclemode;
double SpeedReq ;
//AutoStateEx
int StateTurningLight;
int CurDriveMode;
int StateBraking;
//AutoState
int EPBState;
int GearState;
int BrkPedal;
int AccPedal;
//esc
double RRWheelSpd;
double LFWheelSpd;
double RFWheelSpd;
double LRWheelSpd;
//eps
double AngleSpd;
double Angle;
double StrngWhlTorq;
//IMU
double Imu_accX;
double Imu_accY;
double Imu_accZ;
double Yaw;//航向角
double Roll;
double Pitch;
//GPS
double latitude;//纬度
double longitude;
//轨迹点长度
int m=300;//轨迹点长度   
float cte_D=0;
float cte_A;
//回调函数flag
bool config_flag=false;
bool callback_auto_state_ex_flag=false;
bool callback_auto_state_flag=false;
bool callback_esc_flag=false;
bool callback_eps_flag=false;
bool callback_imu_flag=false;
bool callback_gps_flag=false;
bool callback_gpsfix_flag=false;
//数据采集
ofstream ofile;                  //定义输出文件
//结构体定义
typedef struct Path
{
    float x=0;
    float y=0;
    float z=0;
    float theta=0;//度。
    float pitch=0;
    float roll=0;
    float v=0;//车辆速度m/s
}Pathpoint;
Pathpoint Waypoints[10240];
Pathpoint Current_Point;//车辆当前状态
//
void    callback_eps(const xpmotors_can_msgs::EPSStatus &msg);
void    callback_Config(const  xpmotors_can_msgs::AutoCtlReq &config);
void    callback_auto_state_ex(const xpmotors_can_msgs::AutoStateEx &msg);
void    callback_auto_state(const xpmotors_can_msgs::AutoState &msg);
void    callback_esc(const xpmotors_can_msgs::ESCStatus &msg);
void    callback_imu(const sensor_msgs::Imu &msg);
void callback_gpsfix(const sensor_msgs::NavSatFix &msg);
void    MySigintHandler(int sig);
int main(int argc, char **argv)
{
    
    ros::init(argc, argv, "Speedtest");	
    ros::NodeHandle n;	
    ros::Publisher pub = n.advertise<xpmotors_can_msgs::AutoCtlReq>("/xp/auto_control", 1000);	//告诉系统要发布话题了，话题名为“str_message”，类型为std_msgs::String，缓冲队列为1000。
    ros::Rate loop_rate(10);	
    ros::Subscriber sub1_ = n.subscribe("config/waypoint_follower", 10, callback_Config);
    ros::Subscriber sub2_ = n.subscribe("/xp/auto_state_ex", 10, callback_auto_state_ex);
    ros::Subscriber sub3_ = n.subscribe("/xp/auto_state", 10, callback_auto_state);
    ros::Subscriber sub4_ = n.subscribe("/xp/esc_status", 10, callback_esc);
    ros::Subscriber sub5_ = n.subscribe("/xp/eps_status", 10, callback_eps);
    ros::Subscriber sub6_ = n.subscribe("/imu/data", 10, callback_imu);
    ros::Subscriber sub7_ = n.subscribe("gps/fix", 10, callback_gpsfix);
 
    //数据采集    
    ofile.open("/home/icv/vehicle/Vehicle data/MeiyPath2.txt");     //作为输出文件打开
    ofile<<"x "<<" y "<<" theta "<<" latitude "<<" longtitude "<<endl;//写入txt数据

    signal(SIGINT, MySigintHandler);
    //ros::ok()返回false会停止运行，进程终止。
    while(ros::ok())
    {
    xpmotors_can_msgs::AutoCtlReq ctrmsg;	
    ros::Time begin = ros::Time::now();//获取系统时间
    if(callback_imu_flag&callback_gpsfix_flag)
    {
        // ofile<<setiosflags(ios::fixed)<<setprecision(7)<<Current_Point.x<<" "<<Current_Point.y<<" "<<Current_Point.theta<<" "<<latitude<<" "<<longitude<<endl;//写入txt数据 
        ofile<<setiosflags(ios::fixed)<<setprecision(3)<<Current_Point.x<<" "<<Current_Point.y<<" "<<Current_Point.theta<<endl;//写入txt数据
        // ofile<<setiosflags(ios::fixed)<<setprecision(7)<<latitude<<" "<<longitude<<endl;//写入txt数据  
    }
    ros::spinOnce();	
    loop_rate.sleep();	//按前面设置的10Hz频率将程序挂起
    }

    return 0;
}

void callback_Config(const  xpmotors_can_msgs::AutoCtlReq &config)
{
    vehiclemode  = config.AutoMode;
    SpeedReq = config.TarSpeedReq;
    ROS_INFO("ConfigSpeed is %lf",SpeedReq);      
    bool config_flag=false;

}

void callback_auto_state_ex(const xpmotors_can_msgs::AutoStateEx &msg)
{

    StateTurningLight=msg.StateTurningLight;
    CurDriveMode=msg.CurDriveMode;
    StateBraking=msg.StateBraking;
    callback_auto_state_ex_flag=true;
  
}

void callback_auto_state(const xpmotors_can_msgs::AutoState &msg)
{
    EPBState=msg.EPBState;
    GearState=msg.GearState;
    BrkPedal=msg.BrkPedal;
    AccPedal=msg.AccPedal; 
    callback_auto_state_flag=true;
  
}

void callback_esc(const xpmotors_can_msgs::ESCStatus &msg)
{
    RRWheelSpd=msg.RRWheelSpd;
    LFWheelSpd=msg.LFWheelSpd;
    RFWheelSpd=msg.RFWheelSpd;
    LRWheelSpd=msg.LRWheelSpd;
    callback_esc_flag=true;
 
}

void callback_eps(const xpmotors_can_msgs::EPSStatus &msg)
{

    AngleSpd=msg.AngleSpd;
    Angle=msg.Angle;
    StrngWhlTorq=msg.StrngWhlTorq;
    callback_eps_flag=true; 

}

void callback_imu(const sensor_msgs::Imu &msg)
{
    double w,x,y,z;
    Imu_accX=msg.linear_acceleration.x;
    Imu_accY=msg.linear_acceleration.y;
    Imu_accZ=msg.linear_acceleration.z;
    w=msg.orientation.w;
    x=msg.orientation.x;
    y=msg.orientation.y;
    z=msg.orientation.z;
    Roll = (atan2((2*(w*x+y*z)),(1-2*(x*x+y*y))))/PI*180.0;
    Pitch = (asin(2*(w*y-z*x)))/PI*180.0;
    Yaw = (atan2((2*(w*z+x*y)),(1-2*(z*z+y*y))))/PI*180.0;
    if(Yaw<0)
    {
        Yaw=-1*Yaw;
    }
    else if (Yaw>0)
    {
        Yaw=360-Yaw;
    }
        
    Current_Point.theta=Yaw;
    // cout<<"Yaw"<<Yaw<<endl;
    ROS_INFO("YAW is %.2lf",Yaw);   
    callback_imu_flag=true;
}
void callback_gpsfix(const sensor_msgs::NavSatFix &msg)
{
    double f;
    double a,b,e,N,M,H,A,B,C,T,FE,X,Y,k0,L,L0;

    latitude=msg.latitude;
    longitude=msg.longitude;
    callback_gps_flag=true;
    ROS_INFO("lat is %.7lf",latitude);   
    ROS_INFO("lon is %.7lf",longitude);

    // double scale[2];
    // const double lat1 = 40.00654925; //GPScede 40.00683544,116.32814736
    // const double lon1 = 116.3281271;    
    // // cout<<"latitude "<<latitude<<"lontitude"<<longitude<<endl;
    // float a = 6378137.0;
    // float f = 298.2572;
    // float b = (f - 1) / f * a;
    // float e2 = (a * a - b * b) / (a * a);
    // float A = a * (1 - e2) / pow((1 - e2 * pow(sin(lat1 / 180.0 * PI), 2)), 1.5);
    // float B = a * cos(lat1 / 180.0 * PI) / sqrt(1 - e2 * pow(sin(lat1 / 180.0 * PI), 2));
    // scale[1] = B * 1.0 / 180.0 * PI;
    // scale[2] = A * 1.0 / 180.0 * PI;
    // Current_Point.x = (latitude - lon1) * scale[1]; //转换为坐标x
    // Current_Point.y = (longitude - lat1) * scale[2];
    


    B=PI*latitude/180.0;
    L=PI*longitude/180.0;
    a=6378137;
    b=6356752.3142;
    k0=1;
    H=20;
    L0=PI*117/180.0;
    f=sqrt((a/b)*(a/b)-1);
    e=sqrt(1-(b/a)*(b/a));
    N=(a*a/b)/sqrt(1+f*f*cos(B)*cos(B));
    M=a*((1-e*e/4-3*pow(e,4)/64-5*pow(e,6)/256)*B-(3*e*e/8+3*pow(e,4)/32+45*pow(e,6)/1024)*sin(2*B)+(15*pow(e,4)/256+45*pow(e,6)/1024)*sin(4*B)-sin(6*B)*35*pow(e,6)/3072);
    A=(L-L0)*cos(B);
    C=f*f*cos(B)*cos(B);
    T=tan(B)*tan(B);
    FE=500000+H*1000000;
    Y=k0*(M+N*tan(B)*(A*A/2+(5-T+9*C+4*pow(C,2))*pow(A,4)/24)+(61-58*T+T*T+270*C-330*T*C)*pow(A,6)/720);
    X=FE+k0*N*(A+(1-T+C)*pow(A,3)/6+(5-18*T+T*T+14*C-58*T*C)*pow(A,5)/120);    

    //ximenjiayouzhan wei yuandian 
    X=X-20441065.1238410;
    Y=Y-4429649.9202231;
    Current_Point.x=X;
    Current_Point.y=Y;

    ROS_INFO("Current_Point.x is %.7lf",Current_Point.x);   
    ROS_INFO("Current_Point.y is %.7lf",Current_Point.y);  
    
    // ROS_INFO("recive gps ");
    callback_gpsfix_flag=true;

}
void MySigintHandler(int sig)
{
	//这里主要进行退出前的数据保存、内存清理、告知其他节点等工作
    // ROS_INFO("stop outfilel data!");
    // ofile.close();
	ROS_INFO("shutting down!");
	ros::shutdown();
}