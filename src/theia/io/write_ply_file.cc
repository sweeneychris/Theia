// Copyright (C) 2015 The Regents of the University of California (Regents).
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

#include "theia/io/write_ply_file.h"

#include <glog/logging.h>
#include <fstream>  // NOLINT
#include <string>
#include <vector>

#include "theia/sfm/reconstruction.h"

namespace theia {

// Gather points from tracks.
void GatherTracks(const Reconstruction& reconstruction,
                  std::vector<Eigen::Vector3d>* points_to_write,
                  std::vector<Eigen::Vector3f>* colors_to_write) {
  for (const TrackId track_id : reconstruction.TrackIds()) {
    const Track& track = *reconstruction.Track(track_id);
    if (!track.IsEstimated()) {
      continue;
    }
    points_to_write->emplace_back(track.Point().hnormalized());
    colors_to_write->emplace_back(1.0, 1.0, 1.0);
  }
}

// Gather camera positions.
void GatherCameras(const Reconstruction& reconstruction,
                   std::vector<Eigen::Vector3d>* points_to_write,
                   std::vector<Eigen::Vector3f>* colors_to_write) {
  for (const ViewId view_id : reconstruction.ViewIds()) {
    const View& view = *reconstruction.View(view_id);
    if (!view.IsEstimated()) {
      continue;
    }
    points_to_write->emplace_back(view.Camera().GetPosition());
    colors_to_write->emplace_back(0.0, 1.0, 0.0);
  }
}

// Writes a PLY file for viewing in software such as MeshLab.
bool WritePlyFile(const std::string& ply_file,
                  const Reconstruction& reconstruction) {
  CHECK_GT(ply_file.length(), 0);

  // Return false if the file cannot be opened for writing.
  std::ofstream ply_writer(ply_file, std::ofstream::out);
  if (!ply_writer.is_open()) {
    LOG(ERROR) << "Could not open the file: " << ply_file
               << " for writing a PLY file.";
    return false;
  }

  // Containers for points that we will write to the PLY file.
  std::vector<Eigen::Vector3d> points_to_write;
  std::vector<Eigen::Vector3f> colors_to_write;
  GatherTracks(reconstruction, &points_to_write, &colors_to_write);
  GatherCameras(reconstruction, &points_to_write, &colors_to_write);

  ply_writer << "ply"
    << '\n' << "format ascii 1.0"
             << '\n' << "element vertex " << points_to_write.size()
    << '\n' << "property double x"
    << '\n' << "property double y"
    << '\n' << "property double z"
    << '\n' << "property float red"
    << '\n' << "property float green"
    << '\n' << "property float blue"
    << '\n' << "end_header" << std::endl;

  for (int i = 0; i < points_to_write.size(); i++) {
    ply_writer << points_to_write[i].transpose() << " "
               << colors_to_write[i].transpose() << "\n";
  }

  return true;
}

}  // namespace theia
