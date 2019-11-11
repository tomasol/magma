// Copyright (c) 2019-present, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.

#include <devmand/channels/cli/SshSocketReader.h>
#include <devmand/channels/cli/SshSessionAsync.h>
#include <event2/event.h>
#include <thread>
#include <chrono>

using devmand::channels::cli::sshsession::SshSessionAsync;

void sshReadNotificationThread(struct event_base *base);
void sshReadNotificationThread(struct event_base *base) {
    while (true) {
        int rv = event_base_dispatch(base);
        if (rv == 1) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            continue;
        }
        else {
            const char *error = "event_base_dispatch catastrophic failure, terminating";
            MLOG(MERROR) << error << " returned value: " << rv;
            throw std::runtime_error(error);
        }
    }
}

devmand::channels::cli::SshSocketReader::SshSocketReader() {
    event_config *pConfig = event_config_new();
    int rv = event_config_avoid_method(pConfig, "poll");
    LOG(INFO) << "return value for event_config_avoid_method: " << rv;
    rv = event_config_avoid_method(pConfig, "epoll");
    LOG(INFO) << "return value for event_config_avoid_method: " << rv;
//    rv = event_config_avoid_method(pConfig, "select");
//    LOG(INFO) << "return value for event_config_avoid_method: " << rv;
    event_config_require_features(pConfig, EV_FEATURE_FDS);
    event_config_set_flag(pConfig, EVENT_BASE_FLAG_IGNORE_ENV);
    event_config_set_flag(pConfig, EVENT_BASE_FLAG_NO_CACHE_TIME);
    base = event_base_new_with_config(pConfig);
    const char *method = event_base_get_method(base);
    LOG(INFO) << "event_base_get_method: " << method;

    std::thread notificationThread(sshReadNotificationThread, base);
    notificationThread.detach();
    //event_config_free(pConfig);
}

struct event * devmand::channels::cli::SshSocketReader::addSshReader(event_callback_fn callbackFn, socket_t fd, void *ptr) {
    LOG(INFO) << "pridavam FD: " << fd;
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
