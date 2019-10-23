

#include <devmand/channels/cli/ReadCachingCli.h>
#include <magma_logging.h>

using folly::Future;
using std::string;
using devmand::channels::cli::Command;
using devmand::channels::cli::Cli;
using std::shared_ptr;
using folly::EvictingCacheMap;

Future<string> devmand::channels::cli::ReadCachingCli::executeAndRead(const Command &cmd) {
    const string &command = cmd.toString();
    if (cache->exists(command)) {
        MLOG(MDEBUG) << "Found command: " << command << " in cache";
        return Future<string>(cache->get(command));
    } else {
        return cli->executeAndRead(cmd).thenValue([=](string output){
            cache->insert(command, output);
            return output;
        });
    }
}

devmand::channels::cli::ReadCachingCli::ReadCachingCli(
        const std::shared_ptr<Cli> &_cli,
        const shared_ptr <EvictingCacheMap<string, string>> &_cache) : cli(_cli), cache(_cache) {}

Future<string> devmand::channels::cli::ReadCachingCli::executeAndSwitchPrompt(const Command &cmd) {
    return Future<string>(cmd.toString());
}
