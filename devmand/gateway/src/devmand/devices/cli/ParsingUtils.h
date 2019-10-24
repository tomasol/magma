#pragma once

#include <boost/algorithm/string.hpp>
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

extern void parseValue(
    const string& output,
    const regex& pattern,
    const uint& groupToExtract,
    const std::function<void(string)>& setter);

} // namespace cli
} // namespace devices
} // namespace devmand
