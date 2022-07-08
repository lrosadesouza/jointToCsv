#include "ros/ros.h"
#include "sensor_msgs/JointState.h"
#include "std_msgs/String.h"
#include <string>
#include <fstream>
#include <iostream> 
#include <cstdlib>


bool recordFlag = true; //flag variable that controls when recording happens.
std::ofstream file;
std::string fileName = "/home/airlab/Desktop/default.csv"; //default name for first file to be written to. 

/*
* MainRecorderCallback is the callback function for the /joint_states topic
* used by the jointSub subscriber node created in the recorderControlCallback function. 
* It is used to filter and format information from /joint_states and store it into a csv file. 
*/
void MainRecorderCallback(const sensor_msgs::JointState::ConstPtr& jointMsg)
{
  ROS_ERROR("In Joint Recording Function");
  double timeSec = (double)jointMsg->header.stamp.toSec(); 
  double timeNanoSec = (double)jointMsg->header.stamp.toNSec();

  std::string entry = std::to_string(timeSec) + "," + std::to_string(timeNanoSec) + ",";

  for(int i = 0; i < jointMsg->position.size(); i++)
  {
    entry += std::to_string(jointMsg->position[i]) + ",";
  }

  for(int i = 0; i < jointMsg->velocity.size(); i++)
  {
    entry += std::to_string(jointMsg->velocity[i]) + ",";
  }

  for(int i = 0; i < jointMsg->effort.size(); i++)
  {
    entry += std::to_string(jointMsg->effort[i]) + ((i == jointMsg->effort.size() -1) ? "" : ",") ;
  }

  file.open(fileName.c_str(), std::ios::app);
  ROS_ERROR(fileName.c_str());

  if(file.is_open())
  {
  file << entry.c_str() << "\n";
  ROS_INFO(entry.c_str());
  file.close();
  }else{
    ROS_ERROR("File is not open");
  }
}

/*
* recorderControlCallback function is the callback for the /record topic. 
* It is  mainly used to control when this node starts/stops listening to the /joint_states topic.
* This is done by (start, stop, shutdown) messages sent to the /record topic. 
* To change the name of the file this node writes to, i.e, to write to a new file,
* just publish the name of the new file to the /record topic. The default file name is default.csv
*/

void recorderControlCallback(const std_msgs::String::ConstPtr& msg)
{ 
  std::string message = "recorderControlCallback received: ";
  message += msg->data;
  ROS_ERROR(message.c_str());

  ros::NodeHandle m;
  ros::Subscriber jointSub; 
  ros::Duration duration(1);
  
  if (msg->data.compare("start") == 0) //starts and keeps recording
  {
      jointSub = m.subscribe("joint_states", 1000, MainRecorderCallback);            
      recordFlag = true;
      duration.sleep();
    
      while(recordFlag){ 
       ros::spinOnce();
       }


  }
  else if(msg->data.compare("stop") == 0) //stops recording, closes file, but keeps node running on B.G
  {
     recordFlag = false;
     file.close();
     duration.sleep();
     
  }
  else if (msg->data.compare("shutdown") == 0) { // shuts node down
      ros::shutdown();
  }
  else
  {
    fileName = "/home/airlab/Desktop/" + msg->data; //changes output file
    ROS_ERROR("Output file name changed");
    duration.sleep();
  }

} 


int main(int argc, char **argv)
{
  
  ros::init(argc, argv, "recorderNode");
  ros::NodeHandle n;
  
  ros::Subscriber recordSub = n.subscribe("record", 1000, recorderControlCallback);
  ros::Duration duration(1);
  
  while(ros::ok())
  {

    ROS_ERROR("Spinning from MAIN:");
    ros::spinOnce();
    duration.sleep();
  
  } 

  return 0;
}
