#include "ModelRegistry.h"
#include <ydk/codec_provider.hpp>

namespace devmand {
namespace devices {
namespace cli {

using namespace ydk;
using namespace std;
using namespace ydk::path;

const Model Model::OPENCONFIG_0_1_6 = Model("/usr/share/openconfig@0.1.6");
const Model Model::IETF_0_1_5 = Model("/usr/share/ietf@0.1.5");

static void (*const noop)() = []() {};

Bundle::Bundle(const Model& model)
    : repo(Repository(model.getDir(), ModelCachingOption::COMMON)),
      codecServiceProvider(CodecServiceProvider(repo, EncodingFormat::JSON)),
      jsonSubtreeCodec(JsonSubtreeCodec()) {
  codecServiceProvider.initialize(model.getDir(), model.getDir(), noop);
}

static path::RootSchemaNode& rootSchema(
    CodecServiceProvider& codecServiceProvider) {
  return codecServiceProvider.get_root_schema_for_bundle("");
}

string Bundle::encode(Entity& entity) {
  lock_guard<std::mutex> lg(lock);

  RootSchemaNode& node = rootSchema(codecServiceProvider);
  return jsonSubtreeCodec.encode(entity, node, true);
}

shared_ptr<Entity> Bundle::decode(
    const string& payload,
    shared_ptr<Entity> pointer) {
  lock_guard<std::mutex> lg(lock);

  return jsonSubtreeCodec.decode(payload, pointer);
}

ModelRegistry::~ModelRegistry() {
  cache.clear();
}

ModelRegistry::ModelRegistry() {
  // Set plugin directory for libyang according to:
  // https://github.com/CESNET/libyang/blob/c38295963669219b7aad2618b9f1dd31fa667caa/FAQ.md
  // and CMakeLists.ydk
  setenv("LIBYANG_EXTENSIONS_PLUGINS_DIR", LIBYANG_PLUGINS_DIR, false);
}

ulong ModelRegistry::cacheSize() {
  return cache.size();
}

Bundle& ModelRegistry::getBundle(const Model& model) {
  lock_guard<std::mutex> lg(lock);

  auto it = cache.find(model.getDir());
  if (it != cache.end()) {
    return it->second;
  } else {
    auto pair = cache.emplace(model.getDir(), model);
    return pair.first->second;
  }
}

} // namespace cli
} // namespace devices
} // namespace devmand
