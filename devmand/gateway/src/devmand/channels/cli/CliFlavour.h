// Copyright (c) 2019-present, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.

#pragma once

#include <devmand/channels/cli/SshSessionAsync.h>
#include <memory>

using devmand::channels::cli::sshsession::SshSessionAsync;

namespace devmand {
namespace channels {
namespace cli {

using std::shared_ptr;
using std::string;

class PromptResolver {
public:
    virtual string resolvePrompt(
            shared_ptr<SshSessionAsync> session,
            const string& newline) = 0;
    virtual ~PromptResolver() = default;
};

class DefaultPromptResolver : public PromptResolver {
 public:
  string resolvePrompt(
      shared_ptr<SshSessionAsync> session,
      const string& newline);
  void removeEmptyStrings(std::vector<string>& split) const;
};

class CliInitializer {
 public:
 virtual ~CliInitializer() = default;
 virtual void initialize(shared_ptr<SshSessionAsync> session) = 0;
};

class EmptyInitializer : public CliInitializer {
 public:
  void initialize(shared_ptr<SshSessionAsync> session) override;
  ~EmptyInitializer() override = default;
};

class UbiquitiInitializer : public CliInitializer {
 public:
  void initialize(shared_ptr<SshSessionAsync> session) override;
  ~UbiquitiInitializer() override = default;
};

class CliFlavour {
 public:
  PromptResolver * resolver;
  CliInitializer * initializer;
  string newline;

  CliFlavour(
      PromptResolver * _resolver,
      CliInitializer * _initializer,
      string _newline = "\n");

    virtual ~CliFlavour();
};

} // namespace cli
} // namespace channels
} // namespace devmand
