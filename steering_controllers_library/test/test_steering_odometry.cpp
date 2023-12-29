//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "gmock/gmock.h"

#include "steering_controllers_library/steering_odometry.hpp"

namespace
{
  const double EPS = 1e-8;
}  // unnamed namespace

using namespace steering_odometry;

TEST(TestSteeringOdometry, initialize)
{
  EXPECT_NO_THROW(SteeringOdometry());

  SteeringOdometry odom(1);
  odom.set_wheel_params(1.,2.,3.);
  odom.set_odometry_type(steering_odometry::ACKERMANN_CONFIG);
  EXPECT_FLOAT_EQ(odom.get_heading(), 0.);
  EXPECT_FLOAT_EQ(odom.get_x(), 0.);
  EXPECT_FLOAT_EQ(odom.get_y(), 0.);
}

TEST(TestSteeringOdometry, ackermann_fwd_kin_linear)
{
  SteeringOdometry odom(1);
  odom.set_wheel_params(1.,2.,1.);
  odom.set_odometry_type(steering_odometry::ACKERMANN_CONFIG);
  odom.update_open_loop(2., 0., 0.5);
  EXPECT_FLOAT_EQ(odom.get_linear(), 2.);
  EXPECT_FLOAT_EQ(odom.get_x(), 1.);
  EXPECT_FLOAT_EQ(odom.get_y(), 0.);
}

TEST(TestSteeringOdometry, ackermann_fwd_kin_angular_left)
{
  SteeringOdometry odom(1);
  odom.set_wheel_params(1.,2.,1.);
  odom.set_odometry_type(steering_odometry::ACKERMANN_CONFIG);
  odom.update_open_loop(1., 1., 1.);
  EXPECT_FLOAT_EQ(odom.get_linear(), 1.);
  EXPECT_FLOAT_EQ(odom.get_angular(), 1.);

  EXPECT_TRUE(odom.get_x() > 0); // pos x
  EXPECT_TRUE(odom.get_y() > 0); // pos y, ie. left

}

TEST(TestSteeringOdometry, ackermann_fwd_kin_angular_right)
{
  SteeringOdometry odom(1);
  odom.set_wheel_params(1.,2.,1.);
  odom.set_odometry_type(steering_odometry::ACKERMANN_CONFIG);
  odom.update_open_loop(1., -1., 1.);
  EXPECT_FLOAT_EQ(odom.get_linear(), 1.);
  EXPECT_FLOAT_EQ(odom.get_angular(), -1.);

  EXPECT_TRUE(odom.get_x() > 0);  // pos x
  EXPECT_TRUE(odom.get_y() < 0);  // neg y ie. right
}

TEST(TestSteeringOdometry, ackermann_back_kin_linear)
{
  SteeringOdometry odom(1);
  odom.set_wheel_params(1.,2.,1.);
  odom.set_odometry_type(steering_odometry::ACKERMANN_CONFIG);
  odom.update_open_loop(1., 0., 1.);
  auto cmd = odom.get_commands(1., 0.);
  auto cmd0 = std::get<0>(cmd); // vel
  EXPECT_TRUE(cmd0[0] == cmd0[1]);  // linear
  EXPECT_TRUE(cmd0[0] > 0); 
  auto cmd1 = std::get<1>(cmd); // steer
  EXPECT_TRUE(cmd1[0] == cmd1[1]);  // no steering
  EXPECT_TRUE(cmd1[0] == 0); 
}

TEST(TestSteeringOdometry, ackermann_back_kin_left)
{
  SteeringOdometry odom(1);
  odom.set_wheel_params(1.,2.,1.);
  odom.set_odometry_type(steering_odometry::ACKERMANN_CONFIG);
  odom.update_from_position(0.,0.2,1.); // assume already turn
  auto cmd = odom.get_commands(1.,0.1);
  auto cmd0 = std::get<0>(cmd); // vel
  EXPECT_TRUE(cmd0[0] > cmd0[1]);  // right (outer) > left (inner)
  EXPECT_TRUE(cmd0[0] > 0); 
  auto cmd1 = std::get<1>(cmd); // steer
  EXPECT_TRUE(cmd1[0] < cmd1[1]);  // right (outer) < left (inner)
  EXPECT_TRUE(cmd1[0] > 0); 
  // std::cout << "\nsteer right (outer)" << cmd1[0] << std::endl; 
  // std::cout << "\nsteer left (inner)" << cmd1[1] << std::endl;
}

TEST(TestSteeringOdometry, ackermann_back_kin_right)
{
  SteeringOdometry odom(1);
  odom.set_wheel_params(1.,2.,1.);
  odom.set_odometry_type(steering_odometry::ACKERMANN_CONFIG);
  odom.update_from_position(0.,-0.2,1.); // assume already turn
  auto cmd = odom.get_commands(1.,-0.1);
  auto cmd0 = std::get<0>(cmd); // vel
  EXPECT_TRUE(cmd0[0] < cmd0[1]);  // right (inner) < left outer)
  EXPECT_TRUE(cmd0[0] > 0); 
  auto cmd1 = std::get<1>(cmd); // steer
  EXPECT_TRUE(std::abs(cmd1[0]) > std::abs(cmd1[1]));  // abs right (inner) > abs left (outer)
  EXPECT_TRUE(cmd1[0] < 0); 
  //std::cout << "\nsteer right (inner)" << cmd1[0] << std::endl; 
  //std::cout << "\nsteer left (outer)" << cmd1[1] << std::endl; 
}