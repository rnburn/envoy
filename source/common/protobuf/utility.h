#pragma once

#include <numeric>

#include "envoy/common/exception.h"
#include "envoy/json/json_object.h"

#include "common/common/hash.h"
#include "common/common/utility.h"
#include "common/json/json_loader.h"
#include "common/protobuf/protobuf.h"

// Obtain the value of a wrapped field (e.g. google.protobuf.UInt32Value) if set. Otherwise, return
// the default value.
#define PROTOBUF_GET_WRAPPED_OR_DEFAULT(message, field_name, default_value)                        \
  ((message).has_##field_name() ? (message).field_name().value() : (default_value))

// Obtain the value of a wrapped field (e.g. google.protobuf.UInt32Value) if set. Otherwise, throw
// a MissingFieldException.
#define PROTOBUF_GET_WRAPPED_REQUIRED(message, field_name)                                         \
  ((message).has_##field_name() ? (message).field_name().value()                                   \
                                : throw MissingFieldException(#field_name, (message)))

// Obtain the milliseconds value of a google.protobuf.Duration field if set. Otherwise, return the
// default value.
#define PROTOBUF_GET_MS_OR_DEFAULT(message, field_name, default_value)                             \
  ((message).has_##field_name()                                                                    \
       ? Protobuf::util::TimeUtil::DurationToMilliseconds((message).field_name())                  \
       : (default_value))

// Obtain the milliseconds value of a google.protobuf.Duration field if set. Otherwise, throw a
// MissingFieldException.
#define PROTOBUF_GET_MS_REQUIRED(message, field_name)                                              \
  ((message).has_##field_name()                                                                    \
       ? Protobuf::util::TimeUtil::DurationToMilliseconds((message).field_name())                  \
       : throw MissingFieldException(#field_name, (message)))

// Obtain the seconds value of a google.protobuf.Duration field if set. Otherwise, throw a
// MissingFieldException.
#define PROTOBUF_GET_SECONDS_REQUIRED(message, field_name)                                         \
  ((message).has_##field_name()                                                                    \
       ? Protobuf::util::TimeUtil::DurationToSeconds((message).field_name())                       \
       : throw MissingFieldException(#field_name, (message)))

namespace Envoy {

class MissingFieldException : public EnvoyException {
public:
  MissingFieldException(const std::string& field_name, const Protobuf::Message& message);
};

class RepeatedPtrUtil {
public:
  static std::string join(const Protobuf::RepeatedPtrField<ProtobufTypes::String>& source,
                          const std::string& delimiter) {
    return StringUtil::join(std::vector<std::string>(source.begin(), source.end()), delimiter);
  }

  template <class ProtoType>
  static std::string debugString(const Protobuf::RepeatedPtrField<ProtoType>& source) {
    if (source.empty()) {
      return "[]";
    }
    return std::accumulate(std::next(source.begin()), source.end(), "[" + source[0].DebugString(),
                           [](std::string debug_string, const Protobuf::Message& message) {
                             return debug_string + ", " + message.DebugString();
                           }) +
           "]";
  }
};

class MessageUtil {
public:
  static std::size_t hash(const Protobuf::Message& message) {
    // Use Protobuf::io::CodedOutputStream to force deterministic serialization, so that the same
    // message doesn't hash to different values.
    ProtobufTypes::String text;
    {
      // For memory safety, the StringOutputStream needs to be destroyed before
      // we read the string.
      Protobuf::io::StringOutputStream string_stream(&text);
      Protobuf::io::CodedOutputStream coded_stream(&string_stream);
      coded_stream.SetSerializationDeterministic(true);
      message.SerializeToCodedStream(&coded_stream);
    }
    return HashUtil::xxHash64(text);
  }

  static void loadFromJson(const std::string& json, Protobuf::Message& message);
  static void loadFromYaml(const std::string& yaml, Protobuf::Message& message);
  static void loadFromFile(const std::string& path, Protobuf::Message& message);

  /**
   * Convert from google.protobuf.Any to a typed message.
   * @param message source google.protobuf.Any message.
   * @return MessageType the typed message inside the Any.
   */
  template <class MessageType>
  static inline MessageType anyConvert(const ProtobufWkt::Any& message) {
    MessageType typed_message;
    if (!message.UnpackTo(&typed_message)) {
      throw EnvoyException("Unable to unpack " + message.DebugString());
    }
    return typed_message;
  };

  /**
   * Convert between two protobufs via a JSON round-trip. This is used to translate arbitrary
   * messages to/from google.protobuf.Struct.
   * TODO(htuch): Avoid round-tripping via JSON strings by doing whatever
   * Protobuf::util::MessageToJsonString does but generating a google.protobuf.Struct instead.
   * @param source message.
   * @param dest message.
   */
  static void jsonConvert(const Protobuf::Message& source, Protobuf::Message& dest);

  /**
   * Extract JSON as string from a google.protobuf.Message.
   * @param message message of type type.googleapis.com/google.protobuf.Message.
   * @return std::string of JSON object.
   */
  static std::string getJsonStringFromMessage(const Protobuf::Message& message);

  /**
   * Extract JSON object from a google.protobuf.Message.
   * @param message message of type type.googleapis.com/google.protobuf.Message.
   * @return Json::ObjectSharedPtr of JSON object or nullptr if unable to extract.
   */
  static Json::ObjectSharedPtr getJsonObjectFromMessage(const Protobuf::Message& message) {
    return Json::Factory::loadFromString(MessageUtil::getJsonStringFromMessage(message));
  }
};

class ValueUtil {
public:
  static std::size_t hash(const ProtobufWkt::Value& value) { return MessageUtil::hash(value); }

  /**
   * Compare two ProtobufWkt::Values for equality.
   * @param v1 message of type type.googleapis.com/google.protobuf.Value
   * @param v2 message of type type.googleapis.com/google.protobuf.Value
   * @return true if v1 and v2 are identical
   */
  static bool equal(const ProtobufWkt::Value& v1, const ProtobufWkt::Value& v2);
};

/**
 * HashedValue is a wrapper around ProtobufWkt::Value that computes
 * and stores a hash code for the Value at construction.
 */
class HashedValue {
public:
  HashedValue(const ProtobufWkt::Value& value) : value_(value), hash_(ValueUtil::hash(value)){};
  HashedValue(const HashedValue& v) : value_(v.value_), hash_(v.hash_){};

  const ProtobufWkt::Value& value() const { return value_; }
  std::size_t hash() const { return hash_; }

  bool operator==(const HashedValue& rhs) const {
    return hash_ == rhs.hash_ && ValueUtil::equal(value_, rhs.value_);
  }

  bool operator!=(const HashedValue& rhs) const { return !(*this == rhs); }

private:
  const ProtobufWkt::Value value_;
  const std::size_t hash_;
};

} // namespace Envoy

namespace std {
// Inject an implementation of std::hash for Envoy::HashedValue into the std namespace.
template <> struct hash<Envoy::HashedValue> {
  std::size_t operator()(Envoy::HashedValue const& v) const { return v.hash(); }
};

} // namespace std
