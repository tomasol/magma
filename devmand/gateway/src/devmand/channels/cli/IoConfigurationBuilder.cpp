// Copyright (c) 2019-present, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.

#include <devmand/channels/cli/IoConfigurationBuilder.h>
#include <devmand/channels/cli/PromptAwareCli.h>
#include <devmand/channels/cli/SshSessionAsync.h>
#include <folly/executors/IOThreadPoolExecutor.h>

namespace devmand::channels::cli {
    using devmand::channels::cli::IoConfigurationBuilder;
    using folly::IOThreadPoolExecutor;

    //TODO executor?
    shared_ptr<IOThreadPoolExecutor> executor =
            std::make_shared<IOThreadPoolExecutor>(10);

    IoConfigurationBuilder::IoConfigurationBuilder(
            const DeviceConfig &_deviceConfig,
            const CliFlavour &_cliFlavour)
            : cliFlavour(_cliFlavour), deviceConfig(_deviceConfig) {}

    shared_ptr<Cli> IoConfigurationBuilder::getIo() {
        const auto& plaintextCliKv = deviceConfig.channelConfigs.at("cli").kvPairs;
        // crate session
        const std::shared_ptr<SshSessionAsync>& session =
                std::make_shared<SshSessionAsync>(executor);
        // TODO opening SSH connection
        session->openShell(
                        deviceConfig.ip,
                        std::stoi(plaintextCliKv.at("port")),
                        plaintextCliKv.at("username"),
                        plaintextCliKv.at("password"))
                .get();
        // TODO create CLI - how to create a CLI stack?
        const shared_ptr<PromptAwareCli>& cli =
                std::make_shared<PromptAwareCli>(session, CliFlavour());
        // TODO initialize CLI
        cli->initializeCli();
        // TODO resolve prompt needs to happen
        cli->resolvePrompt();
        return cli;
    }
}