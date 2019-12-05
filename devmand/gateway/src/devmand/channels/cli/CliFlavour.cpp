// Copyright (c) 2019-present, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.

#include <boost/algorithm/string/trim.hpp>
#include <devmand/channels/cli/CliFlavour.h>
#include <regex>

namespace devmand {
namespace channels {
namespace cli {

using namespace std;
using devmand::channels::cli::CliFlavour;
using devmand::channels::cli::CliInitializer;
using devmand::channels::cli::DefaultPromptResolver;
using devmand::channels::cli::EmptyInitializer;
using devmand::channels::cli::PromptResolver;
using devmand::channels::cli::UbiquitiInitializer;
using devmand::channels::cli::sshsession::SessionAsync;
using folly::Optional;

SemiFuture<Unit> EmptyInitializer::initialize(
    shared_ptr<SessionAsync> session,
    string secret) {
  (void)session;
  (void)secret;
  return folly::makeFuture();
}

SemiFuture<Unit> UbiquitiInitializer::initialize(
    shared_ptr<SessionAsync> session,
    string secret) {
  return session->write("enable\n")
      .thenValue(
          [session, secret](...) { return session->write(secret + "\n"); })
      .thenValue(
          [session](...) { return session->write("terminal length 0\n"); });
}

Future<string> DefaultPromptResolver::resolvePrompt(
    shared_ptr<SessionAsync> session,
    const string& newline) {
  return session->read().thenValue(
      [=](...) { return resolvePrompt(session, newline, delayDelta); });
}

Future<string> DefaultPromptResolver::resolvePrompt(
    shared_ptr<SessionAsync> session,
    const string& newline,
    chrono::milliseconds delay) {
  return resolvePromptAsync(session, newline, delay)
      .thenValue([=](Optional<string> prompt) {
        if (!prompt.hasValue()) {
          return resolvePrompt(session, newline, delay + delayDelta);
        } else {
          return folly::makeFuture(prompt.value());
        }
      });
}

Future<Optional<string>> DefaultPromptResolver::resolvePromptAsync(
    shared_ptr<SessionAsync> session,
    const string& newline,
    chrono::milliseconds delay) {
  return session->write(newline + newline)
      .delayed(delay, timekeeper.get())
      .thenValue([session](...) { return session->read(); })
      .thenValue([=](string output) {
        regex regxp("\\" + newline);
        vector<string> split(
            sregex_token_iterator(output.begin(), output.end(), regxp, -1),
            sregex_token_iterator());

        removeEmptyStrings(split);

        if (split.size() == 2) {
          string s0 = boost::algorithm::trim_copy(split[0]);
          string s1 = boost::algorithm::trim_copy(split[1]);
          if (s0 == s1) {
            return folly::make_optional<string>(s0);
          }
        }
        return Optional<string>();
      });
}

void DefaultPromptResolver::removeEmptyStrings(vector<string>& split) const {
  split.erase(
      remove_if(
          split.begin(),
          split.end(),
          [](string& el) {
            boost::algorithm::trim(el);
            return el.empty();
          }),
      split.end());
}

CliFlavour::CliFlavour(
    shared_ptr<folly::Timekeeper> _timekeeper,
    unique_ptr<PromptResolver>&& _resolver,
    unique_ptr<CliInitializer>&& _initializer,
    string _newline)
    : timekeeper(_timekeeper),
      resolver(forward<unique_ptr<PromptResolver>>(_resolver)),
      initializer(forward<unique_ptr<CliInitializer>>(_initializer)),
      newline(_newline) {}

shared_ptr<CliFlavour> CliFlavour::create(
    string flavour,
    shared_ptr<folly::Timekeeper> timekeeper) {
  if (flavour == UBIQUITI) {
    return make_shared<CliFlavour>(
        timekeeper,
        make_unique<DefaultPromptResolver>(timekeeper),
        make_unique<UbiquitiInitializer>());
  } else {
    return make_shared<CliFlavour>(
        timekeeper,
        make_unique<DefaultPromptResolver>(timekeeper),
        make_unique<EmptyInitializer>());
  }
}

} // namespace cli
} // namespace channels
} // namespace devmand
