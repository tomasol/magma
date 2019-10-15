// Copyright (c) 2019-present, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.

#include <devmand/channels/cli/Cli.h>
#include <folly/futures/Future.h>
#include <devmand/channels/cli/SshSessionAsync.h>

namespace devmand {
namespace channels {
namespace cli {

using devmand::channels::cli::sshsession::SshSessionAsync;
using std::string;

class PromptResolver {
public:
    string resolvePrompt(SshSessionAsync & session, const string & newline);

    void removeEmptyStrings(std::vector<string> &split) const;
};

class CliInitializer {
public:
    void initialize(SshSessionAsync & session);
};

class PromptAwareCli : public Cli {
private:
    SshSessionAsync &session;
public:
    PromptAwareCli(SshSessionAsync &session);

    string prompt;
public:
    string newline = "\n"; //TODO make configurable
    void resolvePrompt(PromptResolver resolver = PromptResolver());
    void initializeCli(CliInitializer initializer = CliInitializer());
    folly::Future<std::string> executeAndRead(const Command& cmd) const;

    folly::Future<std::string> executeAndSwitchPrompt(const Command& cmd) const {
        return folly::Future<std::string>(cmd.toString());
    }
};


} // namespace cli
} // namespace channels
} // namespace devmand
