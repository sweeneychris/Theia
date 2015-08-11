.. _chapter-releases:

========
Releases
========

HEAD
====

New Features
------------
* Jenkins Traub polynomial root-finding algorithm.

Bug Fixes
---------
* Disable the unit tests for Optimo (thanks to bvanavery).

Misc.
-----
* Refactoring of the polynomial root-finding algorithms to make the files easier to follow.


`0.4.0 <https://github.com/sweeneychris/TheiaSfM/archive/v0.4.tar.gz>`_
=======================================================================

New Features
------------
* Incremental SfM pipeline.
* New website: `www.theia-sfm.org <http://www.theia-sfm.org>`_.
* Linear method for camera pose registration [JiangICCV]_.
* Better rendering for point clouds.
* Significantly better Cmake scripts for Windows (thanks to bvanevery for testing)
* Mutable priority queue class.
* Bundle adjustment method for cameras only (points held constant).
* Calibrated and Uncalibrated absolute pose estimators.
* Two-view bundle adjustment will now optimize camera intrinsics if they are not known.
* New small and large-scale benchmarking results on the Theia website.

Bug Fixes
---------
* Some Visual Studio bugs and incompatabilities (thanks to Pierre Moulon and Brojeshwar Bhowmick).
* Sample Consensus estimators were incorrectly counting the number of samples needed (found by inspirit).
* Proper normalization the 1dSfM axis of projection.
* OpenGL viewer properly sets zero-values of matrices upon initialization.
* Relative translation optimization (with known rotation) is dramatically improved (thanks to Onur Ozyesil)
* Translations solver uses SPARSE_NORMAL_CHOLESKY when no 3D points are used.

`0.3.0 <https://github.com/sweeneychris/TheiaSfM/archive/v0.3.tar.gz>`_
=======================================================================

New Features
------------
* All cameras are calibrated from EXIF or a median focal length.
* Triangulation is set to use the midpoint method by default.
* All operations on two-view geometry directly operate on the view graph.
* Power method for computing the dominant eigenvector of densor or sparse matrices.
* New program to verify the 1dsfm input against the ground truth model.
* New program to compare two SfM models.
* Nonlinear position estimation uses the nonlinear solver of [WilsonECCV2014]_.
* Removed confusing CameraIntrinsics struct and now all methods use CameraIntrinsicsPrior.
* Calibration files now accept radial distortion and all other camera intrinsics.
* Several new applications to evaluate model and matching quality.
* Robust reconstruction alignment (using RANSAC) to align reconstruction with potential outliers.
* Ability to normalize reconstructions to approximately center and scale nicely for viewing.

Bug Fixes
---------
* 1dSfM dataset input was previously mal-formed.
* GFlags now links pthreads properly.
* Two-view bundle adjustment will no longer use poorly triangulated points for optimization.
* Installation to user-specified folder is done properly.
* Viewing angle test for triangulation.
* Properly estimating relative pose of partially calibrated image matches.

`0.2.0 <https://github.com/sweeneychris/TheiaSfM/archive/v0.2.tar.gz>`_
=======================================================================

New Features
------------

* L1 Solver
* Robust Rotation Solver of [ChatterjeeICCV13]_
* Gflags can now have any namespace
* Reconstructions viewer is now improved
* Initializing rotations from a view graph now use the maximum spanning tree
  instead of a random spanning tree
* Additional run-time options added for building reconstructions

  * ``only_calibrated_views`` will only use calibrated views (from EXIF or
    elsewhere) for building a reconstruction.
  * ``reconstruct_largest_connected_component`` will only build the largest
    connected component of the model instead of building as many models as
    possible.

* 1dSfM datasets [WilsonECCV2014]_ now can be input properly (no quality
  guarantees on the reconstructions though)
* PLY files can be written from a Reconstruction (3D points are all black at
  this point)

Bug Fixes
---------

* Bug fix: removing disconnected view pairs
* Bug fix: 1dSfM filtering of [WilsonECCV2014]_ uses a gaussian distribution to
  randomly sample axis of projections.
* Lowes ratio is fixed.
* Proper hash function for std::pairs (inspiration from Boost)
* Fix BRISK compiler warning for GCC 4.9.1
* Reconstruction viewer bugs and controls are improved
* Better memory management for descriptor extraction and matching

0.1.0
=====

Initial release.
