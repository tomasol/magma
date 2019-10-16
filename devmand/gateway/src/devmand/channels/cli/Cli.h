// Copyright (c) 2019-present, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.

#pragma once

#include <devmand/channels/cli/Command.h>
#include <folly/futures/Future.h>

namespace devmand {
namespace channels {
namespace cli {
class Cli {
 public:
  Cli() = default;
  virtual ~Cli() = default;
  Cli(const Cli&) = delete;
  Cli& operator=(const Cli&) = delete;
  Cli(Cli&&) = delete;
  Cli& operator=(Cli&&) = delete;

 public:
  virtual folly::Future<std::string> executeAndRead(
      const Command& cmd) const = 0;

  virtual folly::Future<std::string> executeAndSwitchPrompt(
      const Command& cmd) const = 0;
};

// TODO remove once not necessary (when the real CLI stack is implemented)

class EchoCli : public Cli {
 public:
  folly::Future<std::string> executeAndRead(const Command& cmd) const override {
    return folly::Future<std::string>(cmd.toString());
  }

  folly::Future<std::string> executeAndSwitchPrompt(const Command& cmd) const override {
    return folly::Future<std::string>(cmd.toString());
  }
};

} // namespace cli
} // namespace channels
} // namespace devmand