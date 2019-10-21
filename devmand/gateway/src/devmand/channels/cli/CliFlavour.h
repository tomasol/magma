// Copyright (c) 2019-present, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.

#pragma once

#include <memory>
#include <devmand/channels/cli/SshSessionAsync.h>

using devmand::channels::cli::sshsession::SshSessionAsync;

namespace devmand {
namespace channels {
namespace cli {

using std::string;
using std::shared_ptr;

class PromptResolver {
public:
    string resolvePrompt(shared_ptr<SshSessionAsync> session, const string & newline);
    void removeEmptyStrings(std::vector<string> &split) const;
};

class CliInitializer {
public:
    void initialize(shared_ptr<SshSessionAsync> session);
};


class CliFlavour {
public:
    PromptResolver resolver;
    CliInitializer initializer;
    string newline;

    CliFlavour(PromptResolver _resolver = PromptResolver(),
               CliInitializer _initializer = CliInitializer(),
               string _newline = "\n");
};


} // namespace cli
} // namespace channels
} // namespace devmand
