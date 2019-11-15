// Copyright (c) 2019-present, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.

#include <devmand/channels/cli/ReadCachingCli.h>
#include <magma_logging.h>
#include <folly/Synchronized.h>
#include <folly/container/EvictingCacheMap.h>

using folly::Future;
using std::string;
using devmand::channels::cli::Command;
using devmand::channels::cli::Cli;
using std::shared_ptr;
using folly::EvictingCacheMap;
using folly::Synchronized;
using CliCache = Synchronized<EvictingCacheMap<string, string>>;

Future<string> devmand::channels::cli::ReadCachingCli::executeAndRead(const Command &cmd) {
    const string command = cmd.toString();
    if (!cmd.skipCache()) {
      string cachedResult = cache->withWLock([=](auto &cache_) {
        if (cache_.exists(command)) {
          MLOG(MDEBUG) << "RCcli Found command: " << command << " in cache";
          return cache_.get(command);
        } else {
          return string(""); // FIXME: optional
        }
      });

      if (!cachedResult.empty()) {
        return Future<string>(cachedResult);
      }
    }

    return cli->executeAndRead(cmd).thenValue([=](string output){
        cache->wlock()->insert(command, output);
        return output;
    });

}

devmand::channels::cli::ReadCachingCli::ReadCachingCli(
        const std::shared_ptr<Cli> &_cli,
        const shared_ptr <CliCache> &_cache) : cli(_cli), cache(_cache) {}

Future<string> devmand::channels::cli::ReadCachingCli::execute(const Command &cmd) {
    return cli->execute(cmd);
}

devmand::channels::cli::ReadCachingCli::~ReadCachingCli() {
  MLOG(MDEBUG) << "~RCcli";
}

shared_ptr<CliCache> devmand::channels::cli::ReadCachingCli::createCache() {
    return shared_ptr<CliCache>(new Synchronized<EvictingCacheMap<string, string>>(EvictingCacheMap<string, string>(200,10)));
}
