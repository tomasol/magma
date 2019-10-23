// Copyright (c) 2019-present, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.

#pragma once

#include <devmand/channels/cli/Cli.h>
#include <folly/container/EvictingCacheMap.h>

namespace devmand::channels::cli {

using folly::EvictingCacheMap;
using std::string;
using std::shared_ptr;
using folly::Future;

class ReadCachingCli : public Cli {
private:
    shared_ptr <Cli> cli{};
    shared_ptr <EvictingCacheMap <string, string>> cache;
public:
    ReadCachingCli(const shared_ptr<Cli> &cli, const shared_ptr <EvictingCacheMap<string, string>> &cache);
    Future<string> executeAndRead(const Command& cmd) override;
    Future<string> executeAndSwitchPrompt(const Command &cmd) override;
};
}