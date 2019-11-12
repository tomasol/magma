// Copyright (c) 2019-present, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.

#pragma once

#include <devmand/channels/cli/Cli.h>
#include <devmand/channels/cli/CliFlavour.h>
#include <devmand/cartography/DeviceConfig.h>
#include <folly/executors/IOThreadPoolExecutor.h>

namespace devmand::channels::cli {

using devmand::channels::cli::Cli;
using devmand::channels::cli::CliFlavour;
using devmand::cartography::DeviceConfig;
using std::shared_ptr;
using folly::IOThreadPoolExecutor;

class IoConfigurationBuilder {
 private:

  shared_ptr<IOThreadPoolExecutor> executor; //TODO executor?
 public:
  IoConfigurationBuilder();

  shared_ptr<Cli> createSSH(const DeviceConfig &deviceConfig);

  shared_ptr<Cli> getIo(shared_ptr<Cli> underlyingCliLayer);
};
}
