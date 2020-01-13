// Copyright (c) 2019-present, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.

#include <devmand/channels/cli/TreeCache.h>


namespace devmand::channels::cli {

TreeCache::TreeCache(shared_ptr<CliFlavour> _cliFlavour)
:cliFlavour(_cliFlavour)
{}

bool TreeCache::update(string cmd, string output) {
  (void)cmd;
  (void)output;
  return false;
}

Optional<string> TreeCache::get(string cmd) {
  (void)cmd;
  return none;
}

}
