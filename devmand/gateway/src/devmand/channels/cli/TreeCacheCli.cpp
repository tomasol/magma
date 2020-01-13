// Copyright (c) 2019-present, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.

#include <devmand/channels/cli/TreeCacheCli.h>

namespace devmand::channels::cli {

using namespace std;
using namespace folly;

TreeCacheCli::TreeCacheCli(string _id,
    shared_ptr<Cli>& _cli,
    shared_ptr<folly::Executor> _executor,
   shared_ptr<CliFlavour> _sharedCliFlavour) :
   id(_id), cli(_cli), executor(_executor), sharedCliFlavour(_sharedCliFlavour)
{
  // TODO cache invalidation
  cache = make_unique<TreeCache>(_sharedCliFlavour);
}


SemiFuture<folly::Unit> TreeCacheCli::destroy() {
  MLOG(MDEBUG) << "[" << id << "] "
               << "destroy: started";
  // call underlying destroy()
  SemiFuture<folly::Unit> innerDestroy = cli->destroy();
  MLOG(MDEBUG) << "[" << id << "] "
               << "destroy: done";
  return innerDestroy;
}

TreeCacheCli::~TreeCacheCli() {
  MLOG(MDEBUG) << "[" << id << "] "
               << "~TCcli: started";
  destroy().get();
  executor = nullptr;
  cli = nullptr;
  MLOG(MDEBUG) << "[" << id << "] "
               << "~TCcli: done";
}

folly::SemiFuture<std::string> TreeCacheCli::executeRead(
    const ReadCommand cmd) {
  if (!cmd.skipCache()) {
    Optional<string> cachedResult = cache->get(cmd.raw());
    if (cachedResult) {
      return folly::SemiFuture<string>(cachedResult.value());
    }
  }
  return cli->executeRead(cmd)
      .via(executor.get())
      .thenValue([=](string output) {
        cache->update(cmd.raw(), output);
        return output;
      })
      .semi();
}

folly::SemiFuture<std::string> TreeCacheCli::executeWrite(
    const WriteCommand cmd) {
  return cli->executeWrite(cmd);
}
}
