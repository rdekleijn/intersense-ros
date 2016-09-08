#include "ros/ros.h"
#include "std_msgs/Int16.h"
#include "sensor_msgs/JointState.h"
#include <sstream>
#include <stdio.h>
#include "isense.h"
#ifdef UNIX
#include <unistd.h>
#endif

int main(int argc, char **argv)
{
  int choice;
  std::cout << "(1) Synchronization, (2) Random movement, or (3) E-Prime? ";
  std::cin >> choice;

  ros::init(argc, argv, "talker");

  ros::NodeHandle n;

  ros::Publisher chatter_pub = n.advertise<std_msgs::Int16>("head_yaw", 1000);
  ros::Publisher chatter_pub_2 = n.advertise<std_msgs::Int16>("head_accel", 1000);
  ros::Publisher head_track_sender = n.advertise<sensor_msgs::JointState>("cmd_joints", 1000);

  ros::Rate loop_rate(10);

  ISD_TRACKER_HANDLE       handle;
  ISD_TRACKER_INFO_TYPE    tracker;
  ISD_TRACKING_DATA_TYPE   data;

  handle = ISD_OpenTracker((Hwnd)NULL, 0, FALSE, FALSE );

  ISD_ResetHeading(handle, 1);

  if ( handle > 0 )
    printf( "\n    Az    El    Rl    X    Y    Z \n" );
  else
    printf( "Tracker not found. Press any key to exit" );

  int count = 0;
  int yaw = 0;
  int pitch = 0;
  double yaw_cmd = 0;
  long counter = 0;
  while (ros::ok())
  {
    counter = counter + 1;

    ISD_GetTrackingData( handle, &data );
    printf( "%7.2f \n", data.Station[0].AngularVelNavFrame[2]);
    yaw = (data.Station[0].Euler[0]);
    yaw_cmd = (float)(yaw) / (float)(60);
    pitch = data.Station[0].AngularVelNavFrame[2];
    ISD_GetCommInfo( handle, &tracker );

    std_msgs::Int16 msg;
    std_msgs::Int16 msg2;
    sensor_msgs::JointState head_track_msg;

    msg.data = yaw;
    msg2.data = pitch;
    head_track_msg.header.stamp = ros::Time::now();
    head_track_msg.name.resize(2);
    head_track_msg.position.resize(2);
    head_track_msg.name[0] = "head_tilt_joint";
    head_track_msg.name[1] = "head_pan_joint";
    head_track_msg.position[0] = 0;
    head_track_msg.position[1] = yaw_cmd;

    if (choice == 2 && (counter % 20 == 0)) {
      double randpos = (double)(rand() % 200);
      randpos = randpos / 100 - 1;
      head_track_msg.position[1] = randpos;
      head_track_sender.publish(head_track_msg);
    }

    chatter_pub.publish(msg);
    chatter_pub_2.publish(msg2);

    if (choice == 1) {head_track_sender.publish(head_track_msg);}

    ros::spinOnce();
    loop_rate.sleep();
    ++count;
  }

  return 0;
}

