// Generated by the gRPC protobuf plugin.
// If you make any local change, they will be lost.
// source: ReaderPlugin.proto

#include "ReaderPlugin.grpc.pb.h"
#include "ReaderPlugin.pb.h"

#include <grpc++/impl/codegen/async_stream.h>
#include <grpc++/impl/codegen/async_unary_call.h>
#include <grpc++/impl/codegen/channel_interface.h>
#include <grpc++/impl/codegen/client_unary_call.h>
#include <grpc++/impl/codegen/method_handler_impl.h>
#include <grpc++/impl/codegen/rpc_service_method.h>
#include <grpc++/impl/codegen/service_type.h>
#include <grpc++/impl/codegen/sync_stream.h>
namespace devmand {
namespace channels {
namespace cli {
namespace plugin {

static const char* ReaderPlugin_method_names[] = {
    "/devmand.channels.cli.plugin.ReaderPlugin/Read",
};

std::unique_ptr<ReaderPlugin::Stub> ReaderPlugin::NewStub(
    const std::shared_ptr<::grpc::ChannelInterface>& channel,
    const ::grpc::StubOptions& options) {
  (void)options;
  std::unique_ptr<ReaderPlugin::Stub> stub(new ReaderPlugin::Stub(channel));
  return stub;
}

ReaderPlugin::Stub::Stub(
    const std::shared_ptr<::grpc::ChannelInterface>& channel)
    : channel_(channel),
      rpcmethod_Read_(
          ReaderPlugin_method_names[0],
          ::grpc::RpcMethod::BIDI_STREAMING,
          channel) {}

::grpc::ClientReaderWriter<
    ::devmand::channels::cli::plugin::ReadRequest,
    ::devmand::channels::cli::plugin::ReadResponse>*
ReaderPlugin::Stub::ReadRaw(::grpc::ClientContext* context) {
  return new ::grpc::ClientReaderWriter<
      ::devmand::channels::cli::plugin::ReadRequest,
      ::devmand::channels::cli::plugin::ReadResponse>(
      channel_.get(), rpcmethod_Read_, context);
}

::grpc::ClientAsyncReaderWriter<
    ::devmand::channels::cli::plugin::ReadRequest,
    ::devmand::channels::cli::plugin::ReadResponse>*
ReaderPlugin::Stub::AsyncReadRaw(
    ::grpc::ClientContext* context,
    ::grpc::CompletionQueue* cq,
    void* tag) {
  return new ::grpc::ClientAsyncReaderWriter<
      ::devmand::channels::cli::plugin::ReadRequest,
      ::devmand::channels::cli::plugin::ReadResponse>(
      channel_.get(), cq, rpcmethod_Read_, context, tag);
}

ReaderPlugin::Service::Service() {
  (void)ReaderPlugin_method_names;
  AddMethod(new ::grpc::RpcServiceMethod(
      ReaderPlugin_method_names[0],
      ::grpc::RpcMethod::BIDI_STREAMING,
      new ::grpc::BidiStreamingHandler<
          ReaderPlugin::Service,
          ::devmand::channels::cli::plugin::ReadRequest,
          ::devmand::channels::cli::plugin::ReadResponse>(
          std::mem_fn(&ReaderPlugin::Service::Read), this)));
}

ReaderPlugin::Service::~Service() {}

::grpc::Status ReaderPlugin::Service::Read(
    ::grpc::ServerContext* context,
    ::grpc::ServerReaderWriter<
        ::devmand::channels::cli::plugin::ReadResponse,
        ::devmand::channels::cli::plugin::ReadRequest>* stream) {
  (void)context;
  (void)stream;
  return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
}

} // namespace plugin
} // namespace cli
} // namespace channels
} // namespace devmand
