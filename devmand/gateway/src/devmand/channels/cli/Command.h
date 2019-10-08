// Copyright (c) 2019-present, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.

#pragma once

#include <folly/futures/Future.h>
#include <iostream>

namespace devmand {
namespace channels {
namespace cli {
/*
 * Command struct encapsulating a string to be executed on a device.
 */
struct Command {
 public:
  Command() = delete;
  static Command createReadCommand(const std::string& cmd);

 private:
  explicit Command(std::string _command);

 public:
  std::string toString() const {
    return command;
  }

  friend std::ostream& operator<<(std::ostream& _stream, Command const& c) {
    _stream << c.toString();
    return _stream;
  }

 private:
  const std::string command;
};

} // namespace cli
} // namespace channels
} // namespace devmand