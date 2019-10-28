// Copyright (c) 2019-present, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.

#include <devmand/channels/cli/SshSocketReader.h>
#include <devmand/channels/cli/SshSessionAsync.h>

using devmand::channels::cli::sshsession::SshSessionAsync;

void sshReadNotificationThread(struct event_base *base);
void sshReadNotificationThread(struct event_base *base) {
    event_base_dispatch(base);
}

devmand::channels::cli::SshSocketReader::SshSocketReader() {
    base = event_base_new();
    std::thread notificationThread(sshReadNotificationThread, base);
    notificationThread.detach();
}

struct event * devmand::channels::cli::SshSocketReader::addSshReader(event_callback_fn callbackFn, socket_t fd, void *ptr) {
    struct event *event_on_heap = event_new(this->base, fd, EV_READ|EV_PERSIST, callbackFn, ptr);
    event_add(event_on_heap, nullptr);
    return event_on_heap;
}

devmand::channels::cli::SshSocketReader::~SshSocketReader() {
    event_base_free(base);
}

devmand::channels::cli::SshSocketReader &devmand::channels::cli::SshSocketReader::getInstance() {
    static SshSocketReader instance;
    return instance;
}
