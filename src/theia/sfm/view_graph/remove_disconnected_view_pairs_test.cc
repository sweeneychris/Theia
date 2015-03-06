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

#include <glog/logging.h>
#include <unordered_map>

#include "gtest/gtest.h"

#include "theia/math/graph/connected_components.h"
#include "theia/util/hash.h"
#include "theia/sfm/twoview_info.h"
#include "theia/sfm/types.h"
#include "theia/sfm/view_graph/remove_disconnected_view_pairs.h"

namespace theia {

TEST(RemoveDisconnectedViewPairs, SingleConnectedComponent) {
  std::unordered_map<ViewIdPair, TwoViewInfo> view_pairs;
  TwoViewInfo info;
  view_pairs[ViewIdPair(0, 1)] = info;
  view_pairs[ViewIdPair(1, 2)] = info;
  view_pairs[ViewIdPair(2, 3)] = info;

  RemoveDisconnectedViewPairs(&view_pairs);
  EXPECT_EQ(view_pairs.size(), 3);
}

TEST(RemoveDisconnectedViewPairs, TwoConnectedComponents) {
  std::unordered_map<ViewIdPair, TwoViewInfo> view_pairs;
  TwoViewInfo info;
  view_pairs[ViewIdPair(0, 1)] = info;
  view_pairs[ViewIdPair(1, 2)] = info;
  view_pairs[ViewIdPair(2, 3)] = info;
  view_pairs[ViewIdPair(5, 6)] = info;
  view_pairs[ViewIdPair(6, 7)] = info;

  RemoveDisconnectedViewPairs(&view_pairs);
  EXPECT_EQ(view_pairs.size(), 3);
}

TEST(RemoveDisconnectedViewPairs, ThreeConnectedComponents) {
  std::unordered_map<ViewIdPair, TwoViewInfo> view_pairs;
  TwoViewInfo info;
  view_pairs[ViewIdPair(0, 1)] = info;
  view_pairs[ViewIdPair(1, 2)] = info;
  view_pairs[ViewIdPair(2, 3)] = info;
  view_pairs[ViewIdPair(5, 6)] = info;
  view_pairs[ViewIdPair(7, 8)] = info;

  RemoveDisconnectedViewPairs(&view_pairs);
  EXPECT_EQ(view_pairs.size(), 3);
}

}  // namespace theia
