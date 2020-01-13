// Copyright (c) 2019-present, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.

#pragma once
#include <devmand/channels/cli/CliFlavour.h>

namespace devmand::channels::cli {

using namespace std;
using namespace folly;

/*
 * Thread unsafe structured cache that can serve subsets of running configuration.
 * When running configuration is returned from the device, calling update function
 * will split it into sections. Consequent calls to get method
 */
class TreeCache {
 private:
  shared_ptr<CliFlavour> cliFlavour;
  // section cache
  map<vector<string>, string> cache;

 public:

  TreeCache(shared_ptr<CliFlavour> sharedCliFlavour);
  /*
   * Try to update cache with actual command output.
   */
  bool update(string cmd, string output);

  /*
   * Try to get result of command from cache
   */
  Optional<string> get(string cmd);

};

}
