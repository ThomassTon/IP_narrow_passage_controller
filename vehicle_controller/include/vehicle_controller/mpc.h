#ifndef MPC_H
#define MPC_H

#include <vehicle_controller/MPCParamsConfig.h>
#include <vehicle_controller/controller.h>
#include <vehicle_controller/lqr_controller.h>

#include <narrow_passage_detection_msgs/NarrowPassageController.h>
#include <narrow_passage_detection_msgs/NarrowPassageDetection.h>

#include <vehicle_controller/ekf.h>

#include <exception>
#include <geometry_msgs/PointStamped.h>
#include <geometry_msgs/PoseStamped.h>
#include <geometry_msgs/Twist.h>
#include <geometry_msgs/Vector3Stamped.h>
#include <grid_map_cv/GridMapCvConverter.hpp>
#include <grid_map_msgs/GetGridMap.h>
#include <grid_map_msgs/ProcessFile.h>
#include <grid_map_msgs/SetGridMap.h>
#include <grid_map_ros/grid_map_ros.hpp>
#include <iostream>
#include <hector_math/robot/robot_model.h>
#include "hector_math_ros/urdf/robot_model.h"
#include <fstream>
#include <ros/package.h>
struct robot_range {
  grid_map::Position position;
  // double angle;
  double distance = 0;

  robot_range( grid_map::Position pos, double dis ) : position( pos ), distance( dis ) { }
};

struct node{
  double linear_vel;
  double angle_vel;

  double reward;
  double min_distance;
  double angle_diff;
  double dis_diff;
  node *parent;
  geometry_msgs::Pose predict_pos1;
  geometry_msgs::Pose predict_pos2;
  geometry_msgs::Pose predict_pos3;
  geometry_msgs::Pose predict_pos4;
  geometry_msgs::Pose predict_pos5;
  node(double linear, double angle, double r, geometry_msgs::Pose predict_pos2, geometry_msgs::Pose predict_pos1, geometry_msgs::Pose predict_pos3, geometry_msgs::Pose predict_pos4, geometry_msgs::Pose predict_pos5) :linear_vel(linear), angle_vel(angle), reward(r), predict_pos2(predict_pos2), predict_pos1(predict_pos1), predict_pos3(predict_pos3), predict_pos4(predict_pos4), predict_pos5(predict_pos5){};
  node(double linear, double angle, double r, double m) :linear_vel(linear), angle_vel(angle), reward(r), min_distance(m){};
  node(double linear, double angle, double r,  geometry_msgs::Pose predict_pos1) :linear_vel(linear), angle_vel(angle), reward(r), predict_pos1(predict_pos1){};

  node(double linear, double angle, double r, double min, double d, double an) :linear_vel(linear), angle_vel(angle), reward(r), min_distance(min), angle_diff(an), dis_diff(d){}
};
struct robot_ladar {
  double left_distance;
  double right_distance;
  double front_distance;
  double back_distance;
};
struct Point_ {
  double x, y;
};

struct dis_buffer_type {
  double distance;
  grid_map::Index index;
  grid_map::Position position;
};

struct Vector_ {
  Point_ start;
  Point_ end;
};
class MPC_Controller : public Controller
{
public:
  Lqr_Controller *lqr;

  MPC_Controller( ros::NodeHandle &nh_ );
  ~MPC_Controller() override;
  void controllerParamsCallback(vehicle_controller::MPCParamsConfig & config, uint32_t level);

  // bool configure() override;
  dynamic_reconfigure::Server<vehicle_controller::MPCParamsConfig> * dr_controller_params_server;

  ros::Time last_time;
  inline std::string getName() override { return "MPC"; }
  ros::NodeHandle nh_dr_paramsss;
  ros::Subscriber stateSubscriber;
  ros::Subscriber map_sub;
  // ros::Subscriber map_sub2;
  ros::Subscriber circle_path_sub;
  // ros::Publisher smoothPathPublisher;
  ros::Subscriber endpoint_approaced;
  ros::Subscriber smoothPath_sub;
  ros::Subscriber controllerTypeSwitch;

  void map_messageCallback2( const grid_map_msgs::GridMap &msg );
  // void map_messageCallback22( const grid_map_msgs::GridMap &msg );
  void smoothPath_messageCallback(const nav_msgs::Path &msg);
  void endpoint_approaced_messageCallback(const narrow_passage_detection_msgs::NarrowPassageController &msg);

  grid_map::GridMap occupancy_map;
  grid_map::GridMap dist_map;
  // grid_map::GridMap elevation_map;
  nav_msgs::Path current_path_;
  nav_msgs::Path adjust_path_;


  bool get_debugging_map = false;
  // bool get_elevation_map = false;
  void predict_position( const geometry_msgs::Pose robot_pose, double linear_vel, double angluar_vel,geometry_msgs::Pose &predict_pose, const double dt );
  void create_robot_range( const geometry_msgs::Pose robot_pose, grid_map::Position &p_front_right, grid_map::Position &p_front_left, grid_map::Position 
&p_back_right, grid_map::Position &p_back_left );

  static bool compareByDistance( robot_range &a, robot_range &b );
  double crossProduct( const Vector_ &AB, const Point_ &C );

  bool collision_detection(const geometry_msgs::Pose robot_pose , double threshold, const grid_map::Position &p_front_right, const grid_map::Position &p_front_left, const grid_map::Position 
&p_back_right,  const grid_map::Position &p_back_left);
  double calc_local_path(geometry_msgs::Pose &lookahead, double distance);
  int calcClosestPoint();
  // bool isPointOnSegment( const grid_map::Position A, const grid_map::Position B );
  static bool compareByDis( const dis_buffer_type &a, const dis_buffer_type &b );
  static bool compareByReward( const node &a, const node &b );
  bool adjust_pos(int index, double radius, int collision_points);
  void appro_integral(double &x, double &y, double dt, double yaw, double linear_vel, double angluar_vel);
  void controllerTypeSwitchCallback(const narrow_passage_detection_msgs::NarrowPassageDetection &msg);
  bool finde_next_sol(geometry_msgs::Pose predict_pos, double lin_vel_dir, double angluar_vel);
  double compute_distance( grid_map::Position pos1, grid_map::Position pos2 );

  bool compute_cmd(double &linear_vel, double & angluar_vel);

  double width = 0.50;  // 0.52
  double length = 0.75; // 0.72
  geometry_msgs::PoseStamped pose;
  geometry_msgs::Vector3Stamped velocity_linear;
  geometry_msgs::Vector3Stamped velocity_angular;
  // std::vector<robot_range> robot_right;
  // std::vector<robot_range> robot_left;
  // std::vector<robot_range> robot_front;
  // std::vector<robot_range> robot_back;
  // std::vector<robot_range> robot_middle;
  double right_min_distance;
  double left_min_distance;
  double front_min_distance;
  double back_min_distance;
  double last_error_front = 0;
  double last_error_back = 0;
  double last_error_mid = 0;

  double p;
  double d;
  double lookahead;
  double lookahead_angle;
  double p2;
  double d2;
  double dt_;
  double dt_c;

  double w_l;
  double w_a;
  double w_l_c;
  double w_a_c;
  double w_min;

  geometry_msgs::PointStamped closest_point;
  double rot_vel_dir, lin_vel_dir;
  double local_path_radius;
  double alignment_angle;

  double lqr_y_error, lqr_x_error;
  double lqr_angle_error;

  geometry_msgs::Twist lqr_last_cmd;
  double lqr_last_y_error;
  double lqr_last_angle_error;

  std::vector<dis_buffer_type> dis_buffer;


  bool get_smoothpath = false;
  bool switch_to_smoothpath =true;


protected:
  void computeMoveCmd() override;
  void reset() override;
  void stateCallback( const nav_msgs::Odometry odom_state );
};

#endif // LQR_CONTROLLER_H
