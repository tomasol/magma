// Copyright (c) 2019-present, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.

#include <devmand/channels/cli/Cli.h>
#include <folly/futures/Future.h>
#include <devmand/channels/cli/SshSessionAsync.h>
#include <devmand/channels/cli/CliFlavour.h>

namespace devmand {
namespace channels {
namespace cli {

using devmand::channels::cli::sshsession::SshSessionAsync;
using devmand::channels::cli::PromptResolver;
using devmand::channels::cli::CliInitializer;
using std::string;
using std::shared_ptr;

class PromptAwareCli : public Cli {
private:
    shared_ptr<SshSessionAsync> session;
    CliFlavour cliFlavour;
public:
    PromptAwareCli(shared_ptr<SshSessionAsync> session, CliFlavour cliFlavour);

    string prompt;
public:
    void init(const string hostname, const int port, const string username, const string password);
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
