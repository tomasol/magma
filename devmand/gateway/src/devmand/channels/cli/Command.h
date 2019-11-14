// Copyright (c) 2019-present, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.

#pragma once

#include <folly/futures/Future.h>
#include <iostream>


using std::vector;
using std::string;

namespace devmand {
namespace channels {
namespace cli {
/*
 * Command struct encapsulating a string to be executed on a device.
 */
struct Command {
 public:
  Command() = delete;
  static Command makeReadCommand(const std::string& cmd, bool skipCache = false);

 private:
  explicit Command(std::string _command, bool skipCache);

 public:
  bool isMultiCommand();
  vector<Command> splitMultiCommand();

  std::string toString() const {
    return command;
  }

  bool skipCache() const {
    return skipCache_;
  }

  friend std::ostream& operator<<(std::ostream& _stream, Command const& c) {
    _stream << c.toString();
    return _stream;
  }

 Command operator=(Command other) {
     return Command(other.toString()); //TODO skipCache equality
 }

 private:
  const string command;
  const bool skipCache_;
};

} // namespace cli
} // namespace channels
} // namespace devmand
