#include <devmand/devices/cli/ParsingUtils.h>

namespace devmand {
namespace devices {
namespace cli {

using namespace std;

void parseValue(
    const string& output,
    const regex& pattern,
    const uint& groupToExtract,
    const std::function<void(string)>& setter) {
  std::stringstream ss(output);
  std::string line;

  while (std::getline(ss, line, '\n')) {
    boost::algorithm::trim(line);
    std::smatch match;
    if (std::regex_match(line, match, pattern) and
        match.size() > groupToExtract and match[groupToExtract].length() > 0) {
      setter(match[groupToExtract]);
    }
  }
}

} // namespace cli
} // namespace devices
}