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

class PromptAwareCli : public Cli {
private:
    shared_ptr<SshSessionAsync> session;
public:
    PromptAwareCli(shared_ptr<SshSessionAsync> session);

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
