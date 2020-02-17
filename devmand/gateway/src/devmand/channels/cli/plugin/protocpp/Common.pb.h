// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: Common.proto

#ifndef PROTOBUF_Common_2eproto__INCLUDED
#define PROTOBUF_Common_2eproto__INCLUDED

#include <string>

#include <google/protobuf/stubs/common.h>

#if GOOGLE_PROTOBUF_VERSION < 3000000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please update
#error your headers.
#endif
#if 3000000 < GOOGLE_PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/arena.h>
#include <google/protobuf/arenastring.h>
#include <google/protobuf/extension_set.h>
#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/message.h>
#include <google/protobuf/metadata.h>
#include <google/protobuf/repeated_field.h>
#include <google/protobuf/unknown_field_set.h>
// @@protoc_insertion_point(includes)

namespace devmand {
namespace channels {
namespace cli {
namespace plugin {

// Internal implementation detail -- do not call these.
void protobuf_AddDesc_Common_2eproto();
void protobuf_AssignDesc_Common_2eproto();
void protobuf_ShutdownFile_Common_2eproto();

class CliRequest;
class CliResponse;

// ===================================================================

class CliRequest : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:devmand.channels.cli.plugin.CliRequest)
                                                       */
{
 public:
  CliRequest();
  virtual ~CliRequest();

  CliRequest(const CliRequest& from);

  inline CliRequest& operator=(const CliRequest& from) {
    CopyFrom(from);
    return *this;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const CliRequest& default_instance();

  void Swap(CliRequest* other);

  // implements Message ----------------------------------------------

  inline CliRequest* New() const {
    return New(NULL);
  }

  CliRequest* New(::google::protobuf::Arena* arena) const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const CliRequest& from);
  void MergeFrom(const CliRequest& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* InternalSerializeWithCachedSizesToArray(
      bool deterministic,
      ::google::protobuf::uint8* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(
      ::google::protobuf::uint8* output) const {
    return InternalSerializeWithCachedSizesToArray(false, output);
  }
  int GetCachedSize() const {
    return _cached_size_;
  }

 private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  void InternalSwap(CliRequest* other);

 private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return _internal_metadata_.arena();
  }
  inline void* MaybeArenaPtr() const {
    return _internal_metadata_.raw_arena_ptr();
  }

 public:
  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // optional int32 id = 1;
  void clear_id();
  static const int kIdFieldNumber = 1;
  ::google::protobuf::int32 id() const;
  void set_id(::google::protobuf::int32 value);

  // optional string cmd = 2;
  void clear_cmd();
  static const int kCmdFieldNumber = 2;
  const ::std::string& cmd() const;
  void set_cmd(const ::std::string& value);
  void set_cmd(const char* value);
  void set_cmd(const char* value, size_t size);
  ::std::string* mutable_cmd();
  ::std::string* release_cmd();
  void set_allocated_cmd(::std::string* cmd);

  // optional bool write = 3;
  void clear_write();
  static const int kWriteFieldNumber = 3;
  bool write() const;
  void set_write(bool value);

  // @@protoc_insertion_point(class_scope:devmand.channels.cli.plugin.CliRequest)
 private:
  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  bool _is_default_instance_;
  ::google::protobuf::internal::ArenaStringPtr cmd_;
  ::google::protobuf::int32 id_;
  bool write_;
  mutable int _cached_size_;
  friend void protobuf_AddDesc_Common_2eproto();
  friend void protobuf_AssignDesc_Common_2eproto();
  friend void protobuf_ShutdownFile_Common_2eproto();

  void InitAsDefaultInstance();
  static CliRequest* default_instance_;
};
// -------------------------------------------------------------------

class CliResponse : public ::google::protobuf::Message /* @@protoc_insertion_point(class_definition:devmand.channels.cli.plugin.CliResponse)
                                                        */
{
 public:
  CliResponse();
  virtual ~CliResponse();

  CliResponse(const CliResponse& from);

  inline CliResponse& operator=(const CliResponse& from) {
    CopyFrom(from);
    return *this;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const CliResponse& default_instance();

  void Swap(CliResponse* other);

  // implements Message ----------------------------------------------

  inline CliResponse* New() const {
    return New(NULL);
  }

  CliResponse* New(::google::protobuf::Arena* arena) const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const CliResponse& from);
  void MergeFrom(const CliResponse& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* InternalSerializeWithCachedSizesToArray(
      bool deterministic,
      ::google::protobuf::uint8* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(
      ::google::protobuf::uint8* output) const {
    return InternalSerializeWithCachedSizesToArray(false, output);
  }
  int GetCachedSize() const {
    return _cached_size_;
  }

 private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  void InternalSwap(CliResponse* other);

 private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return _internal_metadata_.arena();
  }
  inline void* MaybeArenaPtr() const {
    return _internal_metadata_.raw_arena_ptr();
  }

 public:
  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // optional int32 id = 1;
  void clear_id();
  static const int kIdFieldNumber = 1;
  ::google::protobuf::int32 id() const;
  void set_id(::google::protobuf::int32 value);

  // optional string output = 2;
  void clear_output();
  static const int kOutputFieldNumber = 2;
  const ::std::string& output() const;
  void set_output(const ::std::string& value);
  void set_output(const char* value);
  void set_output(const char* value, size_t size);
  ::std::string* mutable_output();
  ::std::string* release_output();
  void set_allocated_output(::std::string* output);

  // @@protoc_insertion_point(class_scope:devmand.channels.cli.plugin.CliResponse)
 private:
  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  bool _is_default_instance_;
  ::google::protobuf::internal::ArenaStringPtr output_;
  ::google::protobuf::int32 id_;
  mutable int _cached_size_;
  friend void protobuf_AddDesc_Common_2eproto();
  friend void protobuf_AssignDesc_Common_2eproto();
  friend void protobuf_ShutdownFile_Common_2eproto();

  void InitAsDefaultInstance();
  static CliResponse* default_instance_;
};
// ===================================================================

// ===================================================================

#if !PROTOBUF_INLINE_NOT_IN_HEADERS
// CliRequest

// optional int32 id = 1;
inline void CliRequest::clear_id() {
  id_ = 0;
}
inline ::google::protobuf::int32 CliRequest::id() const {
  // @@protoc_insertion_point(field_get:devmand.channels.cli.plugin.CliRequest.id)
  return id_;
}
inline void CliRequest::set_id(::google::protobuf::int32 value) {
  id_ = value;
  // @@protoc_insertion_point(field_set:devmand.channels.cli.plugin.CliRequest.id)
}

// optional string cmd = 2;
inline void CliRequest::clear_cmd() {
  cmd_.ClearToEmptyNoArena(
      &::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& CliRequest::cmd() const {
  // @@protoc_insertion_point(field_get:devmand.channels.cli.plugin.CliRequest.cmd)
  return cmd_.GetNoArena(
      &::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void CliRequest::set_cmd(const ::std::string& value) {
  cmd_.SetNoArena(
      &::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:devmand.channels.cli.plugin.CliRequest.cmd)
}
inline void CliRequest::set_cmd(const char* value) {
  cmd_.SetNoArena(
      &::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(value));
  // @@protoc_insertion_point(field_set_char:devmand.channels.cli.plugin.CliRequest.cmd)
}
inline void CliRequest::set_cmd(const char* value, size_t size) {
  cmd_.SetNoArena(
      &::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:devmand.channels.cli.plugin.CliRequest.cmd)
}
inline ::std::string* CliRequest::mutable_cmd() {
  // @@protoc_insertion_point(field_mutable:devmand.channels.cli.plugin.CliRequest.cmd)
  return cmd_.MutableNoArena(
      &::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* CliRequest::release_cmd() {
  // @@protoc_insertion_point(field_release:devmand.channels.cli.plugin.CliRequest.cmd)

  return cmd_.ReleaseNoArena(
      &::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void CliRequest::set_allocated_cmd(::std::string* cmd) {
  if (cmd != NULL) {
  } else {
  }
  cmd_.SetAllocatedNoArena(
      &::google::protobuf::internal::GetEmptyStringAlreadyInited(), cmd);
  // @@protoc_insertion_point(field_set_allocated:devmand.channels.cli.plugin.CliRequest.cmd)
}

// optional bool write = 3;
inline void CliRequest::clear_write() {
  write_ = false;
}
inline bool CliRequest::write() const {
  // @@protoc_insertion_point(field_get:devmand.channels.cli.plugin.CliRequest.write)
  return write_;
}
inline void CliRequest::set_write(bool value) {
  write_ = value;
  // @@protoc_insertion_point(field_set:devmand.channels.cli.plugin.CliRequest.write)
}

// -------------------------------------------------------------------

// CliResponse

// optional int32 id = 1;
inline void CliResponse::clear_id() {
  id_ = 0;
}
inline ::google::protobuf::int32 CliResponse::id() const {
  // @@protoc_insertion_point(field_get:devmand.channels.cli.plugin.CliResponse.id)
  return id_;
}
inline void CliResponse::set_id(::google::protobuf::int32 value) {
  id_ = value;
  // @@protoc_insertion_point(field_set:devmand.channels.cli.plugin.CliResponse.id)
}

// optional string output = 2;
inline void CliResponse::clear_output() {
  output_.ClearToEmptyNoArena(
      &::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline const ::std::string& CliResponse::output() const {
  // @@protoc_insertion_point(field_get:devmand.channels.cli.plugin.CliResponse.output)
  return output_.GetNoArena(
      &::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void CliResponse::set_output(const ::std::string& value) {
  output_.SetNoArena(
      &::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:devmand.channels.cli.plugin.CliResponse.output)
}
inline void CliResponse::set_output(const char* value) {
  output_.SetNoArena(
      &::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(value));
  // @@protoc_insertion_point(field_set_char:devmand.channels.cli.plugin.CliResponse.output)
}
inline void CliResponse::set_output(const char* value, size_t size) {
  output_.SetNoArena(
      &::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:devmand.channels.cli.plugin.CliResponse.output)
}
inline ::std::string* CliResponse::mutable_output() {
  // @@protoc_insertion_point(field_mutable:devmand.channels.cli.plugin.CliResponse.output)
  return output_.MutableNoArena(
      &::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* CliResponse::release_output() {
  // @@protoc_insertion_point(field_release:devmand.channels.cli.plugin.CliResponse.output)

  return output_.ReleaseNoArena(
      &::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void CliResponse::set_allocated_output(::std::string* output) {
  if (output != NULL) {
  } else {
  }
  output_.SetAllocatedNoArena(
      &::google::protobuf::internal::GetEmptyStringAlreadyInited(), output);
  // @@protoc_insertion_point(field_set_allocated:devmand.channels.cli.plugin.CliResponse.output)
}

#endif // !PROTOBUF_INLINE_NOT_IN_HEADERS
// -------------------------------------------------------------------

// @@protoc_insertion_point(namespace_scope)

} // namespace plugin
} // namespace cli
} // namespace channels
} // namespace devmand

// @@protoc_insertion_point(global_scope)

#endif // PROTOBUF_Common_2eproto__INCLUDED
