// Copyright (C) 2014 The Regents of the University of California (Regents).
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

#include <Eigen/Core>
#include <glog/logging.h>
#include <gflags/gflags.h>
#include <theia/theia.h>
#include <string>
#include <vector>

#ifdef __APPLE__
#include <OpenGL/OpenGL.h>
#ifdef FREEGLUT
#include <GL/freeglut.h>
#else
#include <GLUT/glut.h>
#endif
#else
#ifdef _WIN32
#include <windows.h>
#endif
#define GL_GLEXT_PROTOTYPES 1
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#endif

DEFINE_string(reconstruction, "", "Reconstruction file to be viewed.");

// Containers for the data.
std::vector<theia::Camera> cameras;
std::vector<Eigen::Vector3d> world_points;
std::vector<int> num_views_for_track;

// Parameters for OpenGL.
int width = 1200;
int height = 800;

// OpenGL camera parameters.
float viewer_position[3] = { 0.0, 0.0, -50.0 };

// Rotation values for the navigation
float navigation_rotation[3] = { 0.0, 0.0, 0.0 };

// Position of the mouse when pressed
int mouse_pressed_x = 0, mouse_pressed_y = 0;
float last_x_offset = 0.0, last_y_offset = 0.0;
// Mouse button states
int left_mouse_button_active = 0, right_mouse_button_active = 0;
float zoom = 25;

// Visualization parameters.
bool draw_cameras = true;
bool draw_axes = false;
float point_size = 1.0;
float normalized_focal_length = 1.0;
int min_num_views_for_track = 3;
double anti_aliasing_blend = 0.4;

void GetPerspectiveParams(double* aspect_ratio, double* fovy) {
  double focal_length = 800.0;
  *aspect_ratio = static_cast<double>(width) / static_cast<double>(height);
  *fovy = 2 * atan(height / (2.0 * focal_length)) * 180.0 / M_PI;
}

void ChangeSize(int w, int h) {
  // Prevent a divide by zero, when window is too short
  // (you cant make a window of zero width).
  if (h == 0) h = 1;

  // Use the Projection Matrix
  glMatrixMode(GL_PROJECTION);

  // Reset Matrix
  glLoadIdentity();

  // Set the viewport to be the entire window
  double aspect_ratio, fovy;
  GetPerspectiveParams(&aspect_ratio, &fovy);
  glViewport(0, 0, w, h);

  // Set the correct perspective.
  gluPerspective(fovy, aspect_ratio, 0.001f, 100000.0f);

  // Get Back to the Reconstructionview
  glMatrixMode(GL_MODELVIEW);
}

void DrawAxes(float length) {
  glPushAttrib(GL_POLYGON_BIT | GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT);

  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glDisable(GL_LIGHTING);
  glLineWidth(5.0);
  glBegin(GL_LINES);
  glColor3f(1.0, 0.0, 0.0);
  glVertex3f(0, 0, 0);
  glVertex3f(length, 0, 0);

  glColor3f(0.0, 1.0, 0.0);
  glVertex3f(0, 0, 0);
  glVertex3f(0, length, 0);

  glColor3f(0.0, 0.0, 1.0);
  glVertex3f(0, 0, 0);
  glVertex3f(0, 0, length);
  glEnd();

  glPopAttrib();
  glLineWidth(1.0);
}

void DrawCamera(const theia::Camera& camera) {
  glPushMatrix();
  Eigen::Matrix4d transformation_matrix;
  transformation_matrix.block<3, 3>(0, 0) =
      camera.GetOrientationAsRotationMatrix().transpose();
  transformation_matrix.col(3).head<3>() = camera.GetPosition();
  transformation_matrix(3, 3) = 1.0;

  // Apply world pose transformation.
  glMultMatrixd(reinterpret_cast<GLdouble*>(transformation_matrix.data()));

  // Draw Cameras.
  glColor3f(1.0, 0.0, 0.0);

  // Create the camera wireframe. If intrinsic parameters are not set then use
  // the focal length as a guess.
  const double normalized_width =
      (camera.ImageWidth() / 2.0) / camera.FocalLength();
  const double normalized_height =
      (camera.ImageHeight() / 2.0) / camera.FocalLength();

  const Eigen::Vector3d top_left =
      normalized_focal_length *
      Eigen::Vector3d(-normalized_width, -normalized_height, 1);
  const Eigen::Vector3d top_right =
      normalized_focal_length *
      Eigen::Vector3d(normalized_width, -normalized_height, 1);
  const Eigen::Vector3d bottom_right =
      normalized_focal_length *
      Eigen::Vector3d(normalized_width, normalized_height, 1);
  const Eigen::Vector3d bottom_left =
      normalized_focal_length *
      Eigen::Vector3d(-normalized_width, normalized_height, 1);

  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glBegin(GL_TRIANGLE_FAN);
  glVertex3f(0.0, 0.0, 0.0);
  glVertex3f(top_right[0], top_right[1], top_right[2]);
  glVertex3f(top_left[0], top_left[1], top_left[2]);
  glVertex3f(bottom_left[0], bottom_left[1], bottom_left[2]);
  glVertex3f(bottom_right[0], bottom_right[1], bottom_right[2]);
  glVertex3f(top_right[0], top_right[1], top_right[2]);
  glEnd();
  glPopMatrix();
}

void RenderScene() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glTranslatef(viewer_position[0], viewer_position[1], viewer_position[2]);
  glRotatef(navigation_rotation[0], 1.0f, 0.0f, 0.0f);
  glRotatef(navigation_rotation[1], 0.0f, 1.0f, 0.0f);

  if (draw_axes) {
    DrawAxes(10.0);
  }

  glClearColor(1.0f, 1.0f, 1.0f, 0.0f);

  // Plot the point cloud.
  glDisable(GL_LIGHTING);
  glEnable(GL_MULTISAMPLE);
  glEnable(GL_BLEND);
  glEnable(GL_POINT_SMOOTH);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glPointSize(point_size);

  // the coordinates for calculating point attenuation:
  GLfloat point_size_coords[3];
  point_size_coords[0] = 1.0f;
  point_size_coords[1] = 0.055f;
  point_size_coords[2] = 0.0f;
  glPointParameterfv(GL_POINT_DISTANCE_ATTENUATION, point_size_coords);

  // Draw the points.
  glColor4f(0.01, 0.01, 0.01, anti_aliasing_blend);
  glBegin(GL_POINTS);
  for (int i = 0; i < world_points.size(); i++) {
    if (num_views_for_track[i] < min_num_views_for_track) {
      continue;
    }
    glVertex3d(world_points[i].x(), world_points[i].y(), world_points[i].z());
  }
  glEnd();

  // Draw the cameras.
  if (draw_cameras) {
    for (int i = 0; i < cameras.size(); i++) {
      DrawCamera(cameras[i]);
    }
  }

  glutSwapBuffers();
}

void MouseButton(int button, int state, int x, int y) {
  // get the mouse buttons
  if (button == GLUT_RIGHT_BUTTON) {
    if (state == GLUT_DOWN) {
      right_mouse_button_active += 1;
    } else {
      right_mouse_button_active -= 1;
    }
  } else if (button == GLUT_LEFT_BUTTON) {
    if (state == GLUT_DOWN) {
      left_mouse_button_active += 1;
      last_x_offset = 0.0;
      last_y_offset = 0.0;
    } else {
      left_mouse_button_active -= 1;
    }
  }

  // scroll event - wheel reports as button 3 (scroll up) and button 4 (scroll
  // down)
  if ((button == 3) || (button == 4)) {
    // Each wheel event reports like a button click, GLUT_DOWN then GLUT_UP
    if (state == GLUT_UP) return;  // Disregard redundant GLUT_UP events
    if (button == 3) {
      viewer_position[2] += zoom;
    }
    else {
      viewer_position[2] -= zoom;
    }
  }

  mouse_pressed_x = x;
  mouse_pressed_y = y;
}

void MouseMove(int x, int y) {
  float x_offset = 0.0, y_offset = 0.0;

  // Rotation controls
  if (right_mouse_button_active) {
    navigation_rotation[0] += ((mouse_pressed_y - y) * 180.0f) / 200.0f;
    navigation_rotation[1] += ((mouse_pressed_x - x) * 180.0f) / 200.0f;

    mouse_pressed_y = y;
    mouse_pressed_x = x;

  } else if (left_mouse_button_active) {
    // Panning controls.
    x_offset = (mouse_pressed_x - x);
    if (last_x_offset != 0.0) {
      viewer_position[0] -= (x_offset - last_x_offset) / 8.0;
    }
    last_x_offset = x_offset;

    y_offset = (mouse_pressed_y - y);
    if (last_y_offset != 0.0) {
      viewer_position[1] += (y_offset - last_y_offset) / 8.0;
    }
    last_y_offset = y_offset;
  }
}

void Keyboard(unsigned char key, int x, int y) {
  switch (key) {
    case 'r':  // reset viewpoint
      viewer_position[0] = 0.0;
      viewer_position[1] = 0.0;
      viewer_position[2] = -50.0;
      navigation_rotation[0] = 0.0;
      navigation_rotation[1] = 0.0;
      navigation_rotation[2] = 0.0;
      mouse_pressed_x = 0;
      mouse_pressed_y = 0;
      last_x_offset = 0.0;
      last_y_offset = 0.0;
      left_mouse_button_active = 0;
      right_mouse_button_active = 0;
      point_size = 1.0;
      break;
    case 'z':
      viewer_position[2] += zoom;
      break;
    case 'Z':
      viewer_position[2] -= zoom;
      break;
    case 'p':
      point_size /= 1.2;
      break;
    case 'P':
      point_size *= 1.2;
      break;
    case 'f':
      normalized_focal_length /= 1.2;
      break;
    case 'F':
      normalized_focal_length *= 1.2;
      break;
    case 'c':
      draw_cameras = !draw_cameras;
      break;
    case 'a':
      draw_axes = !draw_axes;
      break;
    case 't':
      ++min_num_views_for_track;
      break;
    case 'T':
      --min_num_views_for_track;
      break;
    case 'b':
      if (anti_aliasing_blend > 0) {
        anti_aliasing_blend -= 0.05;
      }
      break;
    case 'B':
      if (anti_aliasing_blend < 1.0) {
        anti_aliasing_blend += 0.05;
      }
      break;
  }
}

int main(int argc, char* argv[]) {
  THEIA_GFLAGS_NAMESPACE::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);

  // Output as a binary file.
  std::unique_ptr<theia::Reconstruction> reconstruction(
      new theia::Reconstruction());
  CHECK(ReadReconstruction(FLAGS_reconstruction, reconstruction.get()))
      << "Could not read reconstruction file.";

  // Centers the reconstruction based on the absolute deviation of 3D points.
  reconstruction->Normalize();

  // Set up camera drawing.
  cameras.reserve(reconstruction->NumViews());
  for (const theia::ViewId view_id : reconstruction->ViewIds()) {
    const auto* view = reconstruction->View(view_id);
    if (view == nullptr || !view->IsEstimated()) {
      continue;
    }
    cameras.emplace_back(view->Camera());
  }

  // Set up world points.
  world_points.reserve(reconstruction->NumTracks());
  for (const theia::TrackId track_id : reconstruction->TrackIds()) {
    const auto* track = reconstruction->Track(track_id);
    if (track == nullptr || !track->IsEstimated()) {
      continue;
    }
    world_points.emplace_back(track->Point().hnormalized());
    num_views_for_track.emplace_back(track->NumViews());
  }

  reconstruction.release();

  // Set up opengl and glut.
  glutInit(&argc, argv);
  glutInitWindowPosition(600, 600);
  glutInitWindowSize(1200, 800);
  glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
  glutCreateWindow("Theia Reconstruction Viewer");

  // Set the camera
  gluLookAt(0.0f, 0.0f, -6.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);

  // register callbacks
  glutDisplayFunc(RenderScene);
  glutReshapeFunc(ChangeSize);
  glutMouseFunc(MouseButton);
  glutMotionFunc(MouseMove);
  glutKeyboardFunc(Keyboard);
  glutIdleFunc(RenderScene);

  // enter GLUT event processing loop
  glutMainLoop();

  return 0;
}
