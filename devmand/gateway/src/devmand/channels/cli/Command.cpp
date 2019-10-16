// Copyright (c) 2019-present, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.

#include <devmand/channels/cli/Command.h>

namespace devmand {
namespace channels {
namespace cli {

Command::Command(const std::string _command) : command(_command) {}

// Factory methods
Command Command::makeReadCommand(const std::string& cmd) {
  return Command(cmd);
}

} // namespace cli
} // namespace channels
} // namespace devmand