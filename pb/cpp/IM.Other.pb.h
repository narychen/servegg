// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: IM.Other.proto

#ifndef PROTOBUF_IM_2eOther_2eproto__INCLUDED
#define PROTOBUF_IM_2eOther_2eproto__INCLUDED

#include <string>

#include <google/protobuf/stubs/common.h>

#if GOOGLE_PROTOBUF_VERSION < 2004000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please update
#error your headers.
#endif
#if 2004001 < GOOGLE_PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/repeated_field.h>
#include <google/protobuf/extension_set.h>
// @@protoc_insertion_point(includes)

namespace IM {
namespace Other {

// Internal implementation detail -- do not call these.
void  protobuf_AddDesc_IM_2eOther_2eproto();
void protobuf_AssignDesc_IM_2eOther_2eproto();
void protobuf_ShutdownFile_IM_2eOther_2eproto();

class IMHeartBeat;

// ===================================================================

class IMHeartBeat : public ::google::protobuf::MessageLite {
 public:
  IMHeartBeat();
  virtual ~IMHeartBeat();
  
  IMHeartBeat(const IMHeartBeat& from);
  
  inline IMHeartBeat& operator=(const IMHeartBeat& from) {
    CopyFrom(from);
    return *this;
  }
  
  static const IMHeartBeat& default_instance();
  
  void Swap(IMHeartBeat* other);
  
  // implements Message ----------------------------------------------
  
  IMHeartBeat* New() const;
  void CheckTypeAndMergeFrom(const ::google::protobuf::MessageLite& from);
  void CopyFrom(const IMHeartBeat& from);
  void MergeFrom(const IMHeartBeat& from);
  void Clear();
  bool IsInitialized() const;
  
  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  public:
  
  ::std::string GetTypeName() const;
  
  // nested types ----------------------------------------------------
  
  // accessors -------------------------------------------------------
  
  // @@protoc_insertion_point(class_scope:IM.Other.IMHeartBeat)
 private:
  
  
  mutable int _cached_size_;
  ::google::protobuf::uint32 _has_bits_[1];
  
  friend void  protobuf_AddDesc_IM_2eOther_2eproto();
  friend void protobuf_AssignDesc_IM_2eOther_2eproto();
  friend void protobuf_ShutdownFile_IM_2eOther_2eproto();
  
  void InitAsDefaultInstance();
  static IMHeartBeat* default_instance_;
};
// ===================================================================


// ===================================================================

// IMHeartBeat


// @@protoc_insertion_point(namespace_scope)

}  // namespace Other
}  // namespace IM

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_IM_2eOther_2eproto__INCLUDED
