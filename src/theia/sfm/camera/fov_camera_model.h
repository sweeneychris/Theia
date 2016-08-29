// Copyright (C) 2016 The Regents of the University of California (Regents).
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
// Author: Chris Sweeney (cmsweeney@cs.ucsb.edu)

#ifndef THEIA_SFM_CAMERA_FOV_CAMERA_MODEL_H_
#define THEIA_SFM_CAMERA_FOV_CAMERA_MODEL_H_

#include <cereal/access.hpp>
#include <cereal/cereal.hpp>
#include <cereal/types/base_class.hpp>
#include <cereal/types/polymorphic.hpp>
#include <stdint.h>
#include <Eigen/Core>
#include <Eigen/Geometry>
#include <vector>

#include "theia/sfm/camera/camera_intrinsics_model.h"
#include "theia/sfm/types.h"

namespace theia {

// This class contains the camera intrinsic information for fov cameras. This is
// an alternative representation for camera models with large radial distortion
// (such as fisheye cameras) where the distance between an image point and
// principal point is roughly proportional to the angle between the 3D point and
// the optical axis. This camera model is first proposed in:
//
//   "Straight Lines Have to Be Straight: Automatic Calibration and Removal of
//   Distortion from Scenes of Structured Environments" by Devernay and Faugeras
//   (Machine Vision and Applications). 2001.
class FOVCameraModel : public CameraIntrinsicsModel {
 public:
  FOVCameraModel();
  ~FOVCameraModel() {}

  static const int kIntrinsicsSize = 5;

  enum InternalParametersIndex{
    FOCAL_LENGTH = 0,
    ASPECT_RATIO = 1,
    PRINCIPAL_POINT_X = 2,
    PRINCIPAL_POINT_Y = 3,
    RADIAL_DISTORTION_1 = 4,
  };

  int NumParameters() const override;

  // Returns the camera model type of the object.
  CameraIntrinsicsModelType Type() const override;

  // Set the intrinsic camera parameters from the priors.
  void SetFromCameraIntrinsicsPriors(
      const CameraIntrinsicsPrior& prior) override;

  // Returns the indices of the parameters that will be optimized during bundle
  // adjustment.
  std::vector<int> GetSubsetFromOptimizeIntrinsicsType(
      const OptimizeIntrinsicsType& intrinsics_to_optimize) const override;

  // Returns the calibration matrix in the form specified above.
  void GetCalibrationMatrix(Eigen::Matrix3d* kmatrix) const override;

  // Given a point in the camera coordinate system, apply the camera intrinsics
  // (e.g., focal length, principal point, distortion) to transform the point
  // into pixel coordinates.
  //
  // NOTE: This method should transform to pixel coordinates and so lens
  // distortion should be applied.
  template <typename T>
  static void CameraToPixelCoordinates(const T* intrinsic_parameters,
                                       const T* point,
                                       T* pixel);

  // Given a pixel in the image coordinates, remove the effects of camera
  // intrinsics parameters and lens distortion to produce a point in the camera
  // coordinate system. The point output by this method is effectively a ray in
  // the direction of the pixel in the camera coordinate system.
  template <typename T>
  static void PixelToCameraCoordinates(const T* intrinsic_parameters,
                                       const T* pixel,
                                       T* point);

  // Given an undistorted pixel, apply lens distortion to the pixel to get a
  // distorted pixel. The type of distortion (i.e. radial, tangential, fisheye,
  // etc.) will depend on the camera intrinsics model.
  template <typename T>
  static void DistortPoint(const T* intrinsic_parameters,
                           const T* undistorted_point,
                           T* distorted_point);

  // Given an undistorted pixel, apply lens distortion to the pixel to get a
  // distorted pixel. The type of distortion (i.e. radial, tangential, fisheye,
  // etc.) will depend on the camera intrinsics model.
  template <typename T>
  static void UndistortPoint(const T* intrinsic_parameters,
                             const T* distorted_point,
                             T* undistorted_point);

  // ----------------------- Getter and Setter methods ---------------------- //
  void SetAspectRatio(const double aspect_ratio);
  double AspectRatio() const;

  void SetRadialDistortion(const double radial_distortion_1);
  double RadialDistortion1() const;

 private:
  // Templated method for disk I/O with cereal. This method tells cereal which
  // data members should be used when reading/writing to/from disk.
  friend class cereal::access;
  template <class Archive>
  void serialize(Archive& ar, const std::uint32_t version) {  // NOLINT
    ar(cereal::base_class<CameraIntrinsicsModel>(this));
  }
};

// Implementation of the static templated methods.
// Given a point in the camera coordinate system, apply the camera intrinsics
// (e.g., focal length, principal point) to transform the point into
// undistorted pixel coordinates.
//
// NOTE: This method should transform to UNDISTORTED pixel coordinates and no
// lens distortion should be applied.
template <typename T>
void FOVCameraModel::CameraToPixelCoordinates(
    const T* intrinsic_parameters, const T* point, T* pixel) {
  // Get normalized pixel projection at image plane depth = 1.
  const T& depth = point[2];
  const T normalized_pixel[2] = { point[0] / depth,
                                  point[1] / depth };

  // Apply radial distortion.
  T distorted_pixel[2];
  FOVCameraModel::DistortPoint(intrinsic_parameters,
                               normalized_pixel,
                               distorted_pixel);

  // Apply calibration parameters to transform normalized units into pixels.
  const T& focal_length =
      intrinsic_parameters[FOVCameraModel::FOCAL_LENGTH];
  const T& aspect_ratio =
      intrinsic_parameters[FOVCameraModel::ASPECT_RATIO];
  const T focal_length_y = focal_length * aspect_ratio;
  const T& principal_point_x =
      intrinsic_parameters[FOVCameraModel::PRINCIPAL_POINT_X];
  const T& principal_point_y =
      intrinsic_parameters[FOVCameraModel::PRINCIPAL_POINT_Y];

  pixel[0] = focal_length * distorted_pixel[0] + principal_point_x;
  pixel[1] = focal_length_y * distorted_pixel[1] + principal_point_y;
}

// Given a point in the camera coordinate system, apply the camera intrinsics
// (e.g., focal length, principal point) to transform the point into
// undistorted pixel coordinates.
//
// NOTE: This method should transform to UNDISTORTED pixel coordinates and no
// lens distortion should be applied.
template <typename T>
void FOVCameraModel::PixelToCameraCoordinates(const T* intrinsic_parameters,
                                              const T* pixel,
                                              T* point) {
  const T& focal_length =
      intrinsic_parameters[FOVCameraModel::FOCAL_LENGTH];
  const T& aspect_ratio =
      intrinsic_parameters[FOVCameraModel::ASPECT_RATIO];
  const T focal_length_y = focal_length * aspect_ratio;
  const T& principal_point_x =
      intrinsic_parameters[FOVCameraModel::PRINCIPAL_POINT_X];
  const T& principal_point_y =
      intrinsic_parameters[FOVCameraModel::PRINCIPAL_POINT_Y];

  // Normalize the y coordinate first.
  T distorted_point[2];
  distorted_point[0] = (pixel[0] - principal_point_x) / focal_length;
  distorted_point[1] = (pixel[1] - principal_point_y) / focal_length_y;

  // Undo the radial distortion.
  T undistorted_point[2];
  FOVCameraModel::UndistortPoint(intrinsic_parameters,
                                 distorted_point,
                                 point);
  point[2] = T(1.0);
}

// Given an undistorted pixel, apply lens distortion to the pixel to get a
// distorted pixel. The type of distortion (i.e. radial, tangential, fisheye,
// etc.) will depend on the camera intrinsics model.
template <typename T>
void FOVCameraModel::DistortPoint(const T* intrinsic_parameters,
                                  const T* undistorted_point,
                                  T* distorted_point) {
  static const T kVerySmallNumber(1e-8);
  // The FOV distortion term omega.
  const T& omega = intrinsic_parameters[RADIAL_DISTORTION_1];

  // The squared radius of the undistorted image point.
  const T r_u_sq = undistorted_point[0] * undistorted_point[0] +
                   undistorted_point[1] * undistorted_point[1];

  // If omega or r_sq are small then the distortion is effectively zero.
  if (omega < kVerySmallNumber || r_u_sq < kVerySmallNumber) {
    distorted_point[0] = undistorted_point[0];
    distorted_point[1] = undistorted_point[1];
    return;
  }

  // Compute the radius of the distorted image point based on the FOV model
  // equations.
  const T r_u = ceres::sqrt(r_u_sq);
  const T r_d =
      ceres::atan(T(2.0) * r_u * ceres::tan(omega / T(2.0))) / (r_u * omega);
  distorted_point[0] = r_d * undistorted_point[0];
  distorted_point[1] = r_d * undistorted_point[1];
}

// Given an undistorted pixel, apply lens distortion to the pixel to get a
// distorted pixel. The type of distortion (i.e. radial, tangential, fisheye,
// etc.) will depend on the camera intrinsics model.
template <typename T>
void FOVCameraModel::UndistortPoint(const T* intrinsic_parameters,
                                    const T* distorted_point,
                                    T* undistorted_point) {
  static const T kVerySmallNumber(1e-8);
  // The FOV distortion term omega.
  const T& omega = intrinsic_parameters[RADIAL_DISTORTION_1];

  // The squared radius of the distorted image point.
  const T r_d_sq = distorted_point[0] * distorted_point[0] +
                   distorted_point[1] * distorted_point[1];

  // If omega or r_d_sq are small then the distortion is effectively zero.
  if (omega < kVerySmallNumber || r_d_sq < kVerySmallNumber) {
    undistorted_point[0] = distorted_point[0];
    undistorted_point[1] = distorted_point[1];
    return;
  }

  // Compute the radius of the distorted image point based on the FOV model
  // equations.
  const T r_d = ceres::sqrt(r_d_sq);
  const T r_u =
      ceres::tan(r_d * omega) / (T(2.0) * r_d * ceres::tan(omega / T(2.0)));
  undistorted_point[0] = r_u * distorted_point[0];
  undistorted_point[1] = r_u * distorted_point[1];
}

}  // namespace theia

#include <cereal/archives/portable_binary.hpp>

CEREAL_CLASS_VERSION(theia::FOVCameraModel, 0)
// Register the polymorphic relationship for serialization.
CEREAL_REGISTER_TYPE(theia::FOVCameraModel)
CEREAL_REGISTER_POLYMORPHIC_RELATION(theia::CameraIntrinsicsModel,
                                     theia::FOVCameraModel)

#endif  // THEIA_SFM_CAMERA_FOV_CAMERA_MODEL_H_
