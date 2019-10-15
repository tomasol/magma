// Copyright (c) 2019-present, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.

#include <devmand/channels/cli/PromptAwareCli.h>
#include <devmand/channels/cli/SshSessionAsync.h>
#include <devmand/channels/cli/Command.h>
#include <boost/algorithm/string/trim.hpp>
#include <folly/executors/IOThreadPoolExecutor.h>

#include <fstream>
#include <iostream>
#include <algorithm>
#include <iterator>
#include <regex>


using devmand::channels::cli::PromptAwareCli;
using devmand::channels::cli::PromptResolver;
using devmand::channels::cli::CliInitializer;
using devmand::channels::cli::SshSessionAsync;
using devmand::channels::cli::Command;
using std::string;

namespace devmand {
namespace channels {
namespace cli {

    static const int DEFAULT_MILLIS = 1000; //TODO value?

    void CliInitializer::initialize(SshSessionAsync &session) {
        session.read(1000).wait();
    }

    string PromptResolver::resolvePrompt(SshSessionAsync &session, const string &newline) {
        session.read(DEFAULT_MILLIS).get(); //clear input, converges faster on prompt
        for (int i = 1; ; i++) {
            int millis = i * DEFAULT_MILLIS;
            session.write(newline + newline).get();
            string output = session.read(millis).get();

            std::regex regxp("\\" + newline);
            std::vector<string> split (
                    std::sregex_token_iterator(output.begin(),
                            output.end(), regxp, -1),
                            std::sregex_token_iterator());

            removeEmptyStrings(split);

            if (split.size() == 2) {
                string s0 = boost::algorithm::trim_copy(split[0]);
                string s1 = boost::algorithm::trim_copy(split[1]);
                if (s0 == s1) {
                    return s0;
                }
            }
        }
    }

    void PromptResolver::removeEmptyStrings(std::vector<string> &split) const {
        split.erase(remove_if(split.begin(), split.end(), [](string & el) {
            boost::algorithm::trim(el);
            return el.empty();
        }), split.end());
    }

    void PromptAwareCli::resolvePrompt(PromptResolver resolver) {
        session.openShell("localhost", 22, "root", "root").get(); //TODO credentials
        this->prompt = resolver.resolvePrompt(session, newline);
    }

    void PromptAwareCli::initializeCli(CliInitializer initializer) {
        initializer.initialize(session);
    }

    PromptAwareCli::PromptAwareCli(SshSessionAsync &_session) : session(_session) {}

    folly::Future<std::string> PromptAwareCli::executeAndRead(const Command &cmd) const {
        const string &command = cmd.toString();
        return session.write(command)
          .thenValue([=](...) {
              return session.readUntilOutput(command);
        }).thenValue([=](...) {
              return session.write(newline);
        }).thenValue([=](...) {
              return session.readUntilOutput(prompt);;
        });
    }
}
}
}