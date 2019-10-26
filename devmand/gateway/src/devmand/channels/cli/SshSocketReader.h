// Copyright (c) 2019-present, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.

#pragma once

#include <devmand/channels/cli/SshSessionAsync.h>

using devmand::channels::cli::sshsession::SshSessionAsync;

namespace devmand {
namespace channels {
namespace cli {

class SshSocketReader {
private:
    struct event_base *base;
public:
    SshSocketReader();

    virtual ~SshSocketReader();

    struct event * addSshReader(SshSessionAsync * session);

};

}
}
}