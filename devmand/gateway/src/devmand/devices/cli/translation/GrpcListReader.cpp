// Copyright (c) 2020-present, Facebook, Inc.
// All rights reserved.
//
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree. An additional grant
// of patent rights can be found in the PATENTS file in the same directory.

#include <devmand/devices/cli/translation/GrpcListReader.h>

namespace devmand {
namespace devices {
namespace cli {

using namespace folly;
using namespace std;
using namespace grpc;
using namespace devmand::channels::cli::plugin;
using namespace devmand::channels::cli;

GrpcListReader::GrpcListReader(
    shared_ptr<grpc::Channel> channel,
    const string _id,
    shared_ptr<Executor> _executor)
    :
    GrpcCliHandler(_id, _executor),
    stub_(devmand::channels::cli::plugin::ReaderPlugin::NewStub(channel))
{}

Future<vector<dynamic>> GrpcListReader::readKeys(
    const Path& path,
    const DeviceAccess& device) const {

  ActualReadRequest* actualRequest = new ActualReadRequest();
  actualRequest->set_path(path.str());
  ReadRequest request;
  request.set_allocated_actualreadrequest(actualRequest);

  return finish<ReadRequest, ReadResponse, vector<dynamic>>(
      request,
      device,
      [this](auto context) { return stub_->Read(context); },
      [](auto response) {
        // TODO: return array instead of {"keys":[]}
        vector<dynamic> values;
        if (response.actualreadresponse().json() == "") {
          return values;
        }
        auto responseDyn = parseJson(response.actualreadresponse().json());
        if (responseDyn.find("keys") == responseDyn.items().end()) {
          return values;
        }
        if (!responseDyn["keys"].isArray()) {
          return values;
        }
        for (auto& k : responseDyn["keys"]) {
          values.push_back(k);
        }
        return values;
      });
}

} // namespace cli
} // namespace devices
} // namespace devmand
