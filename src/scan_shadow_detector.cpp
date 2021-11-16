/*********************************************************************
* Software License Agreement (BSD License)
* 
*  Copyright (c) 2017-2021, laser_filters authors
*  All rights reserved.
* 
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
* 
*   * Redistributions of source code must retain the above copyright
*     notice, this list of conditions and the following disclaimer.
*   * Redistributions in binary form must reproduce the above
*     copyright notice, this list of conditions and the following
*     disclaimer in the documentation and/or other materials provided
*     with the distribution.
*   * Neither the name of the Willow Garage nor the names of its
*     contributors may be used to endorse or promote products derived
*     from this software without specific prior written permission.
* 
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
*  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
*  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
*  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
*  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
*  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
*  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
*  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
*  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
*  POSSIBILITY OF SUCH DAMAGE.
*********************************************************************/

/*
\author Atsushi Watanabe (SEQSENSE, Inc.)
*/

#include <laser_filters/scan_shadow_detector.h>
#include <math.h>
#include <ros/ros.h>

namespace laser_filters
{
  void ScanShadowDetector::configure(const float min_angle, const float max_angle, const int window)
  {
    min_angle_tan_ = tanf(min_angle);
    max_angle_tan_ = tanf(max_angle);
    window_ = window;
    angle_increment_ = 0;
    if (sin_map_ != nullptr)
    {
      delete [] sin_map_;
      delete [] cos_map_;
    }
    sin_map_ = new float[window_ * 2 + 1];
    cos_map_ = new float[window_ * 2 + 1];
    shifted_sin_map_ = sin_map_ + window_;
    shifted_cos_map_ = cos_map_ + window_;

    // Correct sign of tan around singularity points
    if (min_angle_tan_ < 0.0)
      min_angle_tan_ = -min_angle_tan_;
    if (max_angle_tan_ > 0.0)
      max_angle_tan_ = -max_angle_tan_;
  }

  void ScanShadowDetector::prepareForInput(const float angle_increment) {
    if (angle_increment_ != angle_increment) {
      ROS_DEBUG ("[projectLaser] No precomputed map given. Computing one.");
      angle_increment_ = angle_increment;

      float included_angle = -window_ * angle_increment;
      for (int i = -window_; i < window_ + 1; ++i) {
        shifted_sin_map_[i] = fabs(sinf(included_angle));
        shifted_cos_map_[i] = cosf(included_angle);
        included_angle += angle_increment;
      }
    }
  }

  bool ScanShadowDetector::isShadow(const float r1, const float r2, const int angle_index)
  {
    const float perpendicular_y_ = r2 * shifted_sin_map_[angle_index];
    const float perpendicular_x_ = r1 - r2 * shifted_cos_map_[angle_index];
    const float perpendicular_tan_ = perpendicular_y_ / perpendicular_x_;

    return perpendicular_tan_ < min_angle_tan_ && perpendicular_tan_ > max_angle_tan_;
  }

  ScanShadowDetector::~ScanShadowDetector()
  {
    if (sin_map_ != nullptr)
    {
      delete [] sin_map_;
      delete [] cos_map_;
    }
  }
}
