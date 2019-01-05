// Copyright (C) 2018 The Regents of the University of California (Regents).
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//
//     * Redistributions in binary form must reproduce the above
//       copyright notice, this list of conditions and the following
//       disclaimer in the documentation and/or other materials provided
//       with the distribution.
//
//     * Neither the name of The Regents or University of California nor the
//       names of its contributors may be used to endorse or promote products
//       derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// Please contact the author of this library if you have any questions.
// Author: Victor Fragoso (victor.fragoso@mail.wvu.edu)

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <glog/logging.h>
#include "gtest/gtest.h"

#include <vector>

#include "theia/math/util.h"
#include "theia/sfm/pose/upnp.h"

namespace theia {
namespace {

using Eigen::Quaterniond;
using Eigen::AngleAxisd;
using Eigen::Vector3d;

struct InputDatum {
  std::vector<Eigen::Vector3d> ray_origins;
  std::vector<Eigen::Vector3d> ray_directions;
  std::vector<Eigen::Vector3d> world_points;
};

InputDatum ComputeInputDatum(
    const std::vector<Vector3d>& world_points,
    const std::vector<Vector3d>& camera_centers,
    const Quaterniond& expected_rotation,
    const Vector3d& expected_translation) {
  const int num_points = world_points.size();
  const int num_cameras = camera_centers.size();
  
  InputDatum input_datum;
  std::vector<Vector3d>& ray_directions = input_datum.ray_directions;
  std::vector<Vector3d>& ray_origins = input_datum.ray_origins;
  ray_directions.reserve(num_points);
  ray_origins.reserve(num_points);

  for (int i = 0; i < num_points; ++i) {
    // Ray origin wrt to coordinate system of the camera rig.
    const Vector3d ray_origin =
        expected_rotation * camera_centers[i % num_cameras] +
        expected_translation;

    ray_origins.emplace_back(std::move(ray_origin));

    // Reproject 3D points into camera frame.
    ray_directions.emplace_back(
        (expected_rotation * world_points[i] + expected_translation -
         ray_origins[i]).normalized());
  }

  input_datum.world_points = world_points;
  return input_datum;
}

void TestUpnpPoseEstimationWithNoise(
    const std::vector<Vector3d>& world_points,
    const Quaterniond& expected_rotation,
    const Vector3d& expected_translation,
    const double projection_noise_std_dev,
    const double max_reprojection_error,
    const double max_rotation_difference,
    const double max_translation_difference) {
  const int num_points = world_points.size();
  // TODO(vfragoso): Implement me!
}

TEST(UpnpTests, ComputeCostParametersForCentralCameraPoseEstimation) {
  const std::vector<Vector3d> kPoints3d = { Vector3d(-1.0, 3.0, 3.0),
                                            Vector3d(1.0, -1.0, 2.0),
                                            Vector3d(-1.0, 1.0, 2.0),
                                            Vector3d(2.0, 1.0, 3.0) };
  const std::vector<Vector3d> kImageOrigin = { Vector3d(2.0, 0.0, 0.0) };
  const Quaterniond soln_rotation = Quaterniond(
      AngleAxisd(DegToRad(13.0), Vector3d(1.0, 0.0, 1.0).normalized()));
  const Vector3d soln_translation(1.0, 1.0, 1.0);
  const double kNoise = 0.0;
  InputDatum input_datum = ComputeInputDatum(kPoints3d,
                                             kImageOrigin,
                                             soln_rotation,
                                             soln_translation);

  std::vector<Eigen::Quaterniond> solution_rotations;
  std::vector<Eigen::Vector3d> solution_translations;
  const UpnpCostParameters upnp_params = Upnp(input_datum.ray_origins,
                                              input_datum.ray_directions,
                                              input_datum.world_points,
                                              &solution_rotations,
                                              &solution_translations);

  const double upnp_cost = EvaluateUpnpCost(upnp_params, soln_rotation);
  EXPECT_NEAR(upnp_cost, 0.0, 1e-6);
}

TEST(UpnpTests, ComputeCostParametersForNonCentralCameraPoseEstimation) {
  const std::vector<Vector3d> kPoints3d = { Vector3d(-1.0, 3.0, 3.0),
                                            Vector3d(1.0, -1.0, 2.0),
                                            Vector3d(-1.0, 1.0, 2.0),
                                            Vector3d(2.0, 1.0, 3.0) };
  const std::vector<Vector3d> kImageOrigins = { Vector3d(-1.0, 0.0, 0.0),
                                                Vector3d(0.0, 0.0, 0.0),
                                                Vector3d(2.0, 0.0, 0.0),
                                                Vector3d(3.0, 0.0, 0.0) };
  const Quaterniond soln_rotation = Quaterniond(
      AngleAxisd(DegToRad(13.0), Vector3d(0.0, 0.0, 1.0)));
  const Vector3d soln_translation(1.0, 1.0, 1.0);
  const double kNoise = 0.0;
  InputDatum input_datum = ComputeInputDatum(kPoints3d,
                                             kImageOrigins,
                                             soln_rotation,
                                             soln_translation);

  std::vector<Eigen::Quaterniond> solution_rotations;
  std::vector<Eigen::Vector3d> solution_translations;
  const UpnpCostParameters upnp_params = Upnp(input_datum.ray_origins,
                                              input_datum.ray_directions,
                                              input_datum.world_points,
                                              &solution_rotations,
                                              &solution_translations);

  const double upnp_cost = EvaluateUpnpCost(upnp_params, soln_rotation);
  EXPECT_NEAR(upnp_cost, 0.0, 1e-6);
}

TEST(UpnpTests, MinimalCentralCameraPoseEstimation) {
  
}

TEST(UpnpTests, MinimalNonCentralCameraPoseEstimation) {
}

}  // namespace
}  // namespace theia