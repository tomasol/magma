#pragma once

#include <boost/algorithm/string.hpp>
#include <ydk/types.hpp>
#include <regex>
#include <vector>

namespace devmand {
namespace devices {
namespace cli {

using namespace std;

template <typename T>
vector<T> parseKeys(
    const string& output,
    const regex& pattern,
    const uint& groupToExtract,
    const int& skipLines = 0,
    const function<T(string)>& postProcess = [](auto str) { return str; });

// Implementation is in header since this is a template and we cant declare
// all the variations
template <typename T>
vector<T> parseKeys(
    const string& output,
    const regex& pattern,
    const uint& groupToExtract,
    const int& skipLines,
    const function<T(string)>& postProcess) {
  std::stringstream ss(output);
  std::string line;
  vector<T> retval;
  int counter = 0;

  while (std::getline(ss, line, '\n')) {
    counter++;
    if (counter <= skipLines) {
      continue;
    }

    boost::algorithm::trim(line);
    std::smatch match;
    if (std::regex_search(line, match, pattern) and
        match.size() > groupToExtract) {
      T processed = postProcess(match[groupToExtract]);
      retval.push_back(processed);
    }
  }

  return retval;
}

void parseValue(
    const string& output,
    const regex& pattern,
    const uint& groupToExtract,
    const std::function<void(string)>& setter);

template <typename T>
void parseLeaf(
    const string& output,
    const regex& pattern,
    ydk::YLeaf& leaf,
    const uint& groupToExtract = 1,
    const function<T(string)>& postProcess = [](auto str) { return str; }) {
  parseValue(output, pattern, groupToExtract, [&postProcess, &leaf](string v) {
    leaf = postProcess(v);
  });
}

extern function<ydk::uint64(string)> toUI64;
extern function<ydk::uint16(string)> toUI16;

} // namespace cli
} // namespace devices
} // namespace devmand
