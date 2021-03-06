#include "ros/ros.h"
#include "std_msgs/String.h"
#include "ros_aruco/Markers.h"
#include <tf/transform_broadcaster.h>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui_c.h>
#include <opencv2/imgproc/imgproc.hpp>



//std::string marker_id;

void getQuaternion(const cv::Mat &rvec, cv::Mat &quat) { 
    cv::Mat R(3, 3, CV_64F); 
    cv::Rodrigues(rvec, R); 

    if ((quat.rows == 4) && (quat.cols == 1)) 
    {
        //Mat size OK 
    } 
    else 
    { 
     cv::Size size_quat(4,1);
     cv::resize(quat,quat,size_quat); 
    } 
    double w; 

    w = R.at<double>(0,0) + R.at<double>(1,1)+ R.at<double>(2,2) + 1; 
//    if ( w < 0.0 ) return 1; 

    w = sqrt( w ); 
    quat.at<double>(0,0) = (R.at<double>(2,1) - R.at<double>(1,2)) / (w*2.0); 
    quat.at<double>(1,0) = (R.at<double>(0,2) - R.at<double>(2,0)) / (w*2.0); 
    quat.at<double>(2,0) = (R.at<double>(1,0) - R.at<double>(0,1)) / (w*2.0); 
    quat.at<double>(3,0) = w / 2.0; 
//    return 0; 
} 


void poseCallback(const ros_aruco::MarkersConstPtr& msg){
  static tf::TransformBroadcaster br;
  if(msg->count>=1)
  {
    //find the right marker id
    for(int i=0; i< msg->count; i++)
    {
//      if(msg->ids[i] == atoi( marker_id.c_str() ))
      {
        tf::Transform transform;
        transform.setOrigin( tf::Vector3(msg->T[3*i], msg->T[3*i+1], msg->T[3*i+2]));
        cv::Mat rot_rodrigues(3, 1, CV_64F);
        rot_rodrigues.at<double>(0) = msg->R[3*i];
        rot_rodrigues.at<double>(1) = msg->R[3*i+1];
        rot_rodrigues.at<double>(2) = msg->R[3*i+2];
        cv::Mat quat(4,1,CV_64F);
        getQuaternion(rot_rodrigues, quat);

        tf::Quaternion q(quat.at<double>(0,0), quat.at<double>(1,0), quat.at<double>(2,0), quat.at<double>(3,0));
        transform.setRotation(q);
        std::stringstream ss;
        ss << msg->ids[i];
        br.sendTransform(tf::StampedTransform(transform, ros::Time::now(), "camera_rgb_optical_frame", "Marker_"+ss.str()));
      }
    }
  } 

}

int main(int argc, char** argv){
  ros::init(argc, argv, "aruco_axion_tf_broadcaster_node");
  //if (argc != 2){ROS_ERROR("need marker id as argument"); return -1;};
  //marker_id = argv[1];

  ros::NodeHandle node;
  ros::Subscriber sub = node.subscribe("/ros_aruco/markers", 10, &poseCallback);

  ros::spin();
  return 0;
};
