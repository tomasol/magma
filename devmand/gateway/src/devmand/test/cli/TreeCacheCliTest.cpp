#define LOG_WITH_GLOG

#include <magma_logging.h>

#include <devmand/cartography/DeviceConfig.h>
#include <devmand/channels/cli/Cli.h>
#include <devmand/channels/cli/IoConfigurationBuilder.h>
#include <devmand/channels/cli/TimeoutTrackingCli.h>
#include <devmand/devices/Device.h>
#include <devmand/test/cli/utils/Log.h>
#include <devmand/test/cli/utils/MockCli.h>
#include <devmand/test/cli/utils/Ssh.h>
#include <folly/executors/CPUThreadPoolExecutor.h>
#include <folly/executors/ThreadedExecutor.h>
#include <folly/futures/ThreadWheelTimekeeper.h>
#include <gtest/gtest.h>
#include <atomic>
#include <chrono>
#include <regex>
#include <boost/algorithm/string.hpp>

namespace devmand {
namespace test {
namespace cli {

using namespace devmand::test::utils::cli;
using namespace std;
using namespace std::chrono_literals;
using namespace devmand::channels::cli;
using devmand::Application;
using devmand::cartography::ChannelConfig;
using devmand::cartography::DeviceConfig;
using devmand::devices::Datastore;
using devmand::devices::Device;
using devmand::test::utils::cli::AsyncCli;
using devmand::test::utils::cli::EchoCli;
using folly::CPUThreadPoolExecutor;
using namespace devmand::test::utils::ssh;


// more than one whitespace, used in StringUtils::sanitizeSpaces
static const regex sanitizerRegex = regex(R"(\s(\s+))");
// used in StringUtils::getFirstLine
static const regex firstLineRegex = regex("^([^\n]*)\n");


class StringUtils {
 private:
  // trim from start (in place)
  static inline void ltrim(std::string& s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(),
                                    std::not1(std::ptr_fun<int, int>(std::isspace))));
  }

// trim from end (in place)
  static inline void rtrim(std::string& s) {
    s.erase(std::find_if(s.rbegin(), s.rend(),
                         std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
  }

 public:
// trim from both ends (in place)
  static inline void trim(std::string& s) {
    ltrim(s);
    rtrim(s);
  }

  static string getFirstLine(string input) {
    smatch sm;
    if(regex_search(input, sm, firstLineRegex)) {
      return sm[1];
    } else {
      // no \n, return everything
      return input;
    }
  }

  // visible for testing
  /*
   * Clean up redundant whitespace characters.
   */
  // TODO: needed?
  static string sanitizeSpaces(string str) {
    string result = regex_replace(str, sanitizerRegex, " ");
    trim(result);
    return result;
  }
};

// ---

class Flavour {
 private:
  // TODO: const
  regex baseShowConfig;
  Optional<char> singleIndentChar;
  string configSubsectionEnd;

 public:

  Flavour(
      regex _baseShowConfig,
      Optional<char> _singleIndentChar,
      string _configSubsectionEnd
      ):
      baseShowConfig(_baseShowConfig),
      singleIndentChar(_singleIndentChar),
      configSubsectionEnd(_configSubsectionEnd)
      {}

  /*
   * If supplied command is command to show running config, return index where the base command ends.
   * E.g. 'sh run interface 0/14' should return index 6:
   *             ^
   * If command does not match, return none value.
   */
  // visible for testing
  Optional<size_t> getBaseCommandIdx(const string cmd) const {
    smatch pieces_match;
    if (regex_match(cmd, pieces_match, baseShowConfig)) {
      assert(pieces_match.size() == 3);
      return Optional<size_t>((size_t) pieces_match[1].length());
    }
    return Optional<size_t>(none);
  }

  Optional<char> getSingleIndentChar() {
    return singleIndentChar;
  }

  string getConfigSubsectionEnd() {
    return configSubsectionEnd;
  }

  vector<string> splitSubcommands(string subcommands) {
    Optional<char> maybeIndentChar = getSingleIndentChar();
    char indentChar = maybeIndentChar.value_or(' ');
    vector<string> args;
    boost::split(args, subcommands, [indentChar](char s) { return s == indentChar; });
    return args;
  }
};

// ---

class CacheMapBuilder {
 private:
  Flavour cliFlavour;

 public:

  CacheMapBuilder(Flavour _cliFlavour): cliFlavour(_cliFlavour) {}

  /*
   * Assuming command output starts and ends with \n and has lines separated by \n,
   * this regex pattern will match first section of it.
   */
  string createSectionPattern(unsigned int indentationLevel) {
    return createSectionPattern(cliFlavour.getSingleIndentChar(),
                                cliFlavour.getConfigSubsectionEnd(),
                                indentationLevel);
  }

  static string createSectionPattern(
      Optional<char> maybeIndentChar,
      string configSubsectionEnd,
      unsigned int indentationLevel) {
    string match = "\\S[^]*?";
    if (maybeIndentChar.hasValue()) {
      // CONFIG_SUBSECTION_PATTERN = ^%s{LVL}\S.*?^%s{LVL}END
      string nlIndent = "";//"\n";
      nlIndent += maybeIndentChar.value();
      nlIndent += "{" + to_string(indentationLevel) + "}";
      return "\n(" + nlIndent + match + "\n" + nlIndent + configSubsectionEnd + "\n)";
    } else {
      // CONFIG_SUBSECTION_NO_INDENT_PATTERN = ^\S.*?^END
      return "\n(" + match + "\n" + configSubsectionEnd + "\n)";
    }
  }

  /*
   * Iterate over sections. If previous match is detected, content will be
   * set to start after found section starting with newline.
   */
 static  bool hasNextSection(regex& regexMatch, smatch& sm, string& content) {
    if (not sm.empty()) {
      // move content after previous section
      content = "\n";
      content += sm.suffix();
    } else if (content.size() > 0 && content.substr(0,1) != "\n") {
      content = "\n" + content;
    }
    return regex_search(content, sm, regexMatch);
  }

  map<vector<string>, string> readConfigurationToMap(string showRunningOutput, int indentationLevel) {
    regex regexMatch = regex(createSectionPattern(0));
    smatch sm;
    map<vector<string>, string> result;
    (void)indentationLevel;//TODO
    while (hasNextSection(regexMatch, sm, showRunningOutput)) {
      string section = sm[1];
      string firstLine = StringUtils::getFirstLine(section);
      vector<string> args = cliFlavour.splitSubcommands(firstLine);
      result.insert(make_pair(args, section));
    }
    // last piece of content that is not a section is ignored
    return result;
  }

  map<vector<string>, string> readConfigurationToMap(string showRunningOutput) {
    return readConfigurationToMap(showRunningOutput, 0);
  }

};

// ---

class TreeCacheCli {
 private:
  map<vector<string>, string> treeCache;
  Flavour cliFlavour;
  string shRun;

  /*
   * If show running command supplied, return either pipe remainder, or arguments of command.
   * Adapted from parseRunningConfig.
   */
  Optional<string> getShowConfigSections(ReadCommand cmd) {
    // remove base command
    const string& rawCmd = cmd.raw();
    const Optional<size_t>& maybePosition = cliFlavour.getBaseCommandIdx(rawCmd);
    if (maybePosition.hasValue()) {
      string section = rawCmd.substr(maybePosition.value());
      StringUtils::trim(section);
      return Optional<string>(section);
    } else {
      // not a show running config command
      return Optional<string>(none);
    }
  }

 public:
  TreeCacheCli(
      map<vector<string>, string> _treeCache,
      Flavour _cliFlavour,
      string _shRun
      ):
      treeCache(_treeCache),
      cliFlavour(_cliFlavour),
      shRun(_shRun)
      {}


  /*
   * Parse command output, return only section that was specified by subset argument.
   * This can be one or more subcommands separated by indentation characters.
   */
  static Optional<string> findMatchingSubset(
      vector<string> subcommands,
      map<vector<string>, string> treeCache) {
    // simplistic case: find whole key in map
    if (treeCache.count(subcommands) == 1) {
      return treeCache.at(subcommands);
    }
    return none;
  }

  /*
   * If show running command is passed, return result from cache.
   * Otherwise return none;
   */
  Optional<string> executeRead(ReadCommand cmd) {
    const Optional<string>& maybeSubcommands = getShowConfigSections(cmd);
    if (maybeSubcommands.hasValue()) {
      if (maybeSubcommands.value().length() == 0) {
        // just show running config
        return shRun;
      } else {
        // TODO magic
        vector<string> subcommands = cliFlavour.splitSubcommands(maybeSubcommands.value());
        return findMatchingSubset(subcommands, treeCache);
      }
      // TODO pipe
    }
    // not a show running config
    // TODO pass to underlying cli
    return Optional<string>(folly::none);
  }
};

namespace testdata {

static constexpr const char* SH_RUN_UBIQUITI = R"template(*Ubiquiti Edgeswitch outdoor configs:*

hostname "MPK14-11-Switch"
network protocol none
network parms 192.168.0.35 255.255.255.0 0.0.0.0
network ipv6 address autoconfig
vlan database
vlan 10,101,302,312,345,307
vlan name 101 "MGMT"
exit

network mgmt_vlan 101
configure
sntp server "10.128.203.238"
sntp server "1.ntp.vip.facebook.com"
clock timezone -7 minutes 0 zone "PST"
ip name server 2620:10d:c097::c0de
username "ubnt" password asdasdasdasdasdasdasdasdasd level 15 encrypted
line console
exit

line ssh
exit

snmp-server sysname "MPK10-06-Switch"
!

interface 0/1
description 'Primary-1'
switchport mode access
switchport access vlan 307
exit


interface 0/2
description 'Secodary-3'
switchport mode access
switchport access vlan 307
exit



interface 0/3
description 'Secondary-2'
switchport mode access
switchport access vlan 307
exit



interface 0/4
description 'Secondary-1'
switchport mode access
switchport access vlan 307
exit



interface 0/5
description 'Primary-2'
switchport mode access
switchport access vlan 307
exit



interface 0/6
switchport mode access
switchport access vlan 307
exit

interface 0/7
switchport mode access
switchport access vlan 101
exit

interface 0/9
description 'SOMA-AP'
switchport mode access
switchport access vlan 101
exit



interface 0/10
description 'MAGMA-CPE'
switchport mode access
switchport access vlan 345
exit


interface 0/14
description 'Ruckus-AP'
switchport mode access
switch access vlan 100
exit

interface 0/15
description 'NanoBeam'
spanning-tree bpdufilter
switchport mode trunk
switchport trunk native vlan 101
switchport trunk allowed vlan 101,307,312,345
poe opmode passive24v
exit

interface 0/16
description 'PDU'
switchport mode access
switchport access vlan 101
exit

copy system:running-config nvram:startup-config

)template";

static constexpr const char* SH_RUN_CISCO = R"template(Building configuration...
Current configuration : 3781 bytes
!
interface FastEthernet0
 ip address 192.1.12.2 255.255.255.0
 no ip directed-broadcast (default)
 ip nat outside
 duplex auto
 speed auto
!
line con 0
 password cisco123
 transport preferred all
 transport output all
line aux 0
 transport preferred all
 transport output all
!
)template";

static constexpr const char* SH_RUN_INT_GI4 = R"template(interface 0/14
description 'Ruckus-AP'
switchport mode access
switch access vlan 100
exit
)template";

static constexpr const char* SH_RUN_INCLUDE_INT = R"template(interface 0/1
interface 0/2
interface 0/3
interface 0/4
interface 0/5
interface 0/6
interface 0/7
interface 0/9
interface 0/10
interface 0/14
interface 0/15
interface 0/16)template";

static constexpr const char* SH_RUN_TWO_IFC = R"template(

 foo


interface Loopback99
 description bla
!



interface Bundle-Ether103.100
 description TOOL_TEST
 ethernet cfm
  mep domain DML3 service 504 mep-id 1
   cos 6
  !
 !
!
  )template";
}

static const Flavour ubiquitiFlavour(
    regex(R"(^((do )?sho?w? runn?i?n?g?-?c?o?n?f?i?g?).*)"),
    none,
    "exit");

static const Flavour ciscoFlavour(
    regex(R"(^((do )?sho?w? runn?i?n?g?-?c?o?n?f?i?g?).*)"),
    ' ',
    "!");

class TreeCacheCliTest : public ::testing::Test {
 protected:
  unique_ptr<CacheMapBuilder> cacheMap_ubiquiti;
  unique_ptr<CacheMapBuilder> cacheMap_cisco;
  unique_ptr<TreeCacheCli> tested_ubiquiti;
  unique_ptr<TreeCacheCli> tested_cisco;

  void SetUp() override {
    devmand::test::utils::log::initLog();
    cacheMap_ubiquiti = make_unique<CacheMapBuilder>(ubiquitiFlavour);
    cacheMap_cisco = make_unique<CacheMapBuilder>(ciscoFlavour);
    tested_ubiquiti = make_unique<TreeCacheCli>(
        cacheMap_ubiquiti->readConfigurationToMap(testdata::SH_RUN_UBIQUITI),
        ubiquitiFlavour, testdata::SH_RUN_UBIQUITI);
    tested_cisco = make_unique<TreeCacheCli>(
        cacheMap_cisco->readConfigurationToMap(testdata::SH_RUN_CISCO),
        ciscoFlavour, testdata::SH_RUN_CISCO);
  }
};

// ---

TEST_F(TreeCacheCliTest, stringSanitization) {
  EXPECT_EQ(StringUtils::sanitizeSpaces("foo bar"), "foo bar");
  EXPECT_EQ(StringUtils::sanitizeSpaces("foo bar   "), "foo bar");
  EXPECT_EQ(StringUtils::sanitizeSpaces("  foo   bar  "), "foo bar");
}

TEST_F(TreeCacheCliTest, getFirstLine) {
  EXPECT_EQ(StringUtils::getFirstLine("foo\nbar\nbaz"), "foo");
  EXPECT_EQ(StringUtils::getFirstLine("\nbar\nbaz"), "");
}

// ---

TEST_F(TreeCacheCliTest, getBaseCommandIdx) {
  EXPECT_EQ(Optional<size_t>(none), ubiquitiFlavour.getBaseCommandIdx("foo"));
  EXPECT_EQ(Optional<size_t>(6), ubiquitiFlavour.getBaseCommandIdx("sh run"));
  EXPECT_EQ(Optional<size_t>(6), ubiquitiFlavour.getBaseCommandIdx("sh run x"));
  EXPECT_EQ(Optional<size_t>(19), ubiquitiFlavour.getBaseCommandIdx("show running-config x"));
  EXPECT_EQ(Optional<size_t>(22), ubiquitiFlavour.getBaseCommandIdx("do show running-config x"));
}

TEST_F(TreeCacheCliTest, createSectionPattern_noIndent) {
  auto content_chars = R"template(section1
foo
exit
section2
bar
baz
exit
      )template";
  string content(content_chars);
  regex regexMatch = regex(cacheMap_ubiquiti->createSectionPattern(0));
  smatch sm;

  EXPECT_TRUE(cacheMap_ubiquiti->hasNextSection(regexMatch, sm, content));
  EXPECT_EQ(sm[1], "section1\nfoo\nexit\n");

  EXPECT_TRUE(cacheMap_ubiquiti->hasNextSection(regexMatch, sm, content));
  EXPECT_EQ(sm[1], "section2\nbar\nbaz\nexit\n");

  EXPECT_FALSE(cacheMap_ubiquiti->hasNextSection(regexMatch, sm, content));
}

TEST_F(TreeCacheCliTest, createSectionPattern_noNewLineAtStart) {
  string content = "section1\nfoo\nexit\n";
  regex regexMatch = regex(cacheMap_ubiquiti->createSectionPattern(0));
  smatch sm;

  EXPECT_TRUE(cacheMap_ubiquiti->hasNextSection(regexMatch, sm, content));
  EXPECT_EQ(sm[1], "section1\nfoo\nexit\n");
  EXPECT_FALSE(cacheMap_ubiquiti->hasNextSection(regexMatch, sm, content));
}

TEST_F(TreeCacheCliTest, createSectionPattern_noNewLine) {
  string content = "section1";
  regex regexMatch = regex(cacheMap_ubiquiti->createSectionPattern(0));
  smatch sm;

  EXPECT_FALSE(cacheMap_ubiquiti->hasNextSection(regexMatch, sm, content));
}

TEST_F(TreeCacheCliTest, createSectionPattern_indent) {
  string content(testdata::SH_RUN_TWO_IFC);

  string sectionPattern = cacheMap_ubiquiti->createSectionPattern(' ', "!", 0);

  regex regexMatch = regex(sectionPattern);
  smatch sm;

  EXPECT_TRUE(cacheMap_ubiquiti->hasNextSection(regexMatch, sm, content));
  EXPECT_EQ(sm[1], "interface Loopback99\n"
                   " description bla\n"
                   "!\n");
  EXPECT_TRUE(cacheMap_ubiquiti->hasNextSection(regexMatch, sm, content));
  EXPECT_EQ(sm[1], "interface Bundle-Ether103.100\n"
                   " description TOOL_TEST\n"
                   " ethernet cfm\n"
                   "  mep domain DML3 service 504 mep-id 1\n"
                   "   cos 6\n"
                   "  !\n"
                   " !\n"
                   "!\n");
  EXPECT_FALSE(cacheMap_ubiquiti->hasNextSection(regexMatch, sm, content));
  EXPECT_FALSE(cacheMap_ubiquiti->hasNextSection(regexMatch, sm, content));
}

TEST_F(TreeCacheCliTest, readConfigurationToMap_ubiquiti) {
  string content(testdata::SH_RUN_INT_GI4);
  map<vector<string>, string> actual = cacheMap_ubiquiti->readConfigurationToMap(content);
  EXPECT_EQ(actual.size(), 1);
  map<vector<string>, string> expected;

  vector<string> key = {"interface", "0/14"};
  string value = "interface 0/14\n"
                 "description 'Ruckus-AP'\n"
                 "switchport mode access\n"
                 "switch access vlan 100\n"
                 "exit\n";
  expected.insert(make_pair(key, value));
  EXPECT_EQ(actual, expected);
}

TEST_F(TreeCacheCliTest, readConfigurationToMap_cisco) {
  string content(testdata::SH_RUN_TWO_IFC);
  map<vector<string>, string> actual = cacheMap_cisco->readConfigurationToMap(content);
  EXPECT_EQ(actual.size(), 2);

  map<vector<string>, string> expected;
  {
    vector<string> key = {"interface", "Loopback99"};
    string value = "interface Loopback99\n"
                   " description bla\n"
                   "!\n";
    expected.insert(make_pair(key, value));
  }
  {
    vector<string> key = {"interface", "Bundle-Ether103.100"};
    string value = "interface Bundle-Ether103.100\n"
                   " description TOOL_TEST\n"
                   " ethernet cfm\n"
                   "  mep domain DML3 service 504 mep-id 1\n"
                   "   cos 6\n"
                   "  !\n"
                   " !\n"
                   "!\n";
    expected.insert(make_pair(key, value));
  }
  EXPECT_EQ(actual, expected);
}

// ---

TEST_F(TreeCacheCliTest, getWholeConfig) {
  Optional<string> wholeConfig = tested_ubiquiti->executeRead(ReadCommand::create(
      "show running-config", false));
  EXPECT_TRUE(wholeConfig.has_value());
  EXPECT_EQ(testdata::SH_RUN_UBIQUITI, wholeConfig.value());
}

TEST_F(TreeCacheCliTest, getParticularIfc) {
  Optional<string> wholeConfig = tested_ubiquiti->executeRead(ReadCommand::create(
      "sh run interface 0/14", false));
  EXPECT_TRUE(wholeConfig.has_value());
  EXPECT_EQ(testdata::SH_RUN_INT_GI4, wholeConfig.value());
}

}
}
}
