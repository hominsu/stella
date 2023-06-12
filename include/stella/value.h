//
// Created by Homin Su on 2023/6/7.
//

#ifndef STELLA_INCLUDE_STELLA_VALUE_H_
#define STELLA_INCLUDE_STELLA_VALUE_H_

#include <cstddef>
#include <cstring>

#include <algorithm>
#include <memory>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include "stella.h"

#include <lua.hpp>

namespace stella {

#undef VALUE
#define VALUE(_field, _suffix) \
  _field(NIL, ::std::monostate)_suffix \
  _field(BOOL, bool)_suffix    \
  _field(INTEGER, LUA_INTEGER)_suffix  \
  _field(NUMBER, LUA_NUMBER)_suffix    \
  _field(STRING, ::std::shared_ptr<::std::string>)_suffix \
  _field(TABLE, ::std::shared_ptr<::std::vector<Member>>) \
  //

struct Member;

enum Type {
#undef VALUE_NAME
#define VALUE_NAME(_name, _type) S_##_name
#undef SUFFIX
#define SUFFIX ,
  VALUE(VALUE_NAME, SUFFIX)
#undef SUFFIX
#undef VALUE_NAME
};

using Data = ::std::variant<
#undef VALUE_TYPE
#define VALUE_TYPE(_name, _type) _type
#undef SUFFIX
#define SUFFIX ,
    VALUE(VALUE_TYPE, SUFFIX)
#undef SUFFIX
#undef VALUE_TYPE
>;

class Document;

class Value {
 public:
  using MemberIterator = ::std::vector<Member>::iterator;
  using ConstMemberIterator = ::std::vector<Member>::const_iterator;

#undef VALUE_TYPE
#define VALUE_TYPE(_name, _type) using S_##_name##_TYPE = _type;
  VALUE(VALUE_TYPE,)
#undef VALUE_TYPE

 private:
  friend class Document;

  using String = ::std::string;
  using Table = ::std::vector<Member>;

  Type type_;
  Data data_;

 public:
  explicit Value(Type type = S_NIL);
  explicit Value(S_BOOL_TYPE b) : type_(S_BOOL), data_(b) {};
  explicit Value(S_INTEGER_TYPE i) : type_(S_INTEGER), data_(i) {};
  explicit Value(S_NUMBER_TYPE n) : type_(S_NUMBER), data_(n) {};
  explicit Value(const char *s)
      : type_(S_STRING), data_(::std::in_place_index<S_STRING>, ::std::make_shared<String>(s, s + strlen(s))) {};
  explicit Value(::std::string_view s)
      : type_(S_STRING), data_(::std::in_place_index<S_STRING>, ::std::make_shared<String>(s.begin(), s.end())) {};
  Value(const Value &value) = default;
  Value(Value &&value) noexcept: type_(value.type_), data_(::std::move(value.data_)) {};
  ~Value() = default;

  [[nodiscard]] bool IsNil() const { return type_ == S_NIL; }
  [[nodiscard]] bool IsBool() const { return type_ == S_BOOL; }
  [[nodiscard]] bool IsInteger() const { return type_ == S_INTEGER; }
  [[nodiscard]] bool IsNumber() const { return type_ == S_NUMBER; }
  [[nodiscard]] bool IsString() const { return type_ == S_STRING; }
  [[nodiscard]] bool IsTable() const { return type_ == S_TABLE; }

  [[nodiscard]] ::std::size_t GetSize() const;
  [[nodiscard]] Type GetType() const { return type_; };

  [[nodiscard]] S_BOOL_TYPE GetBool() const;
  [[nodiscard]] S_INTEGER_TYPE GetInteger() const;
  [[nodiscard]] S_NUMBER_TYPE GetNumber() const;
  [[nodiscard]] ::std::string_view GetStringView() const;
  [[nodiscard]] ::std::string GetString() const;
  [[nodiscard]] const auto &GetTable() const;

  Value &SetBool(S_BOOL_TYPE b);
  Value &SetInteger(S_INTEGER_TYPE i);
  Value &SetNumber(S_NUMBER_TYPE n);
  Value &SetString(::std::string_view s);
  Value &SetTable();

  MemberIterator MemberBegin();
  MemberIterator MemberEnd();
  MemberIterator FindMember(::std::size_t key);
  MemberIterator FindMember(::std::string_view key);

  [[nodiscard]] ConstMemberIterator MemberBegin() const;
  [[nodiscard]] ConstMemberIterator MemberEnd() const;
  [[nodiscard]] ConstMemberIterator FindMember(::std::size_t key) const;
  [[nodiscard]] ConstMemberIterator FindMember(::std::string_view key) const;

  Value &operator=(const Value &val);
  Value &operator=(Value &&val) noexcept;
  Value &operator[](::std::size_t key);
  const Value &operator[](::std::size_t key) const;
  Value &operator[](::std::string_view key);
  const Value &operator[](::std::string_view key) const;

  template<typename T>
  Value &AddMember(::std::size_t key, T &&value);
  template<typename T>
  Value &AddMember(const char *key, T &&value);
  Value &AddMember(Value &&key, Value &&value);

  template<typename Handler>
  bool WriteTo(Handler &handler) const;
};

#undef VALUE

struct Member {
  Member(Value &&_key, Value &&_value) : key_(::std::move(_key)), value_(::std::move(_value)) {}
  Member(::std::string_view _key, Value &&_value) : key_(_key), value_(::std::move(_value)) {}

  Value key_;
  Value value_;
};

inline Value::Value(Type type) : type_(type), data_() {
  switch (type) {
    case S_NIL:
    case S_BOOL:
    case S_INTEGER:
    case S_NUMBER: break;
    case S_STRING: data_ = ::std::make_shared<String>();
      break;
    case S_TABLE: data_ = ::std::make_shared<Table>();
      break;
    default: STELLA_ASSERT(false && "bad type");
  }
}

inline ::std::size_t Value::GetSize() const {
  switch (type_) {
    case S_TABLE: return ::std::get<S_TABLE_TYPE>(data_)->size();
    default: return 1;
  }
}

inline Value::S_BOOL_TYPE Value::GetBool() const {
  STELLA_ASSERT(type_ == S_BOOL);
  return ::std::get<S_BOOL_TYPE>(data_);
}

inline Value::S_INTEGER_TYPE Value::GetInteger() const {
  STELLA_ASSERT(type_ == S_BOOL || type_ == S_INTEGER || type_ == S_NUMBER);
  switch (type_) {
    case S_BOOL:return ::std::get<S_BOOL_TYPE>(data_);
    case S_INTEGER:return ::std::get<S_INTEGER_TYPE>(data_);
    case S_NUMBER:return static_cast<S_INTEGER_TYPE>(::std::get<S_NUMBER_TYPE>(data_));
    default: STELLA_ASSERT(false);
  }
  return {};
}

inline Value::S_NUMBER_TYPE Value::GetNumber() const {
  STELLA_ASSERT(type_ == S_BOOL || type_ == S_INTEGER || type_ == S_NUMBER);
  switch (type_) {
    case S_BOOL:return ::std::get<S_BOOL_TYPE>(data_);
    case S_INTEGER:return static_cast<S_NUMBER_TYPE>(::std::get<S_INTEGER_TYPE>(data_));
    case S_NUMBER:return ::std::get<S_NUMBER_TYPE>(data_);
    default: STELLA_ASSERT(false);
  }
  return {};
}

inline ::std::string_view Value::GetStringView() const {
  STELLA_ASSERT(type_ == S_STRING);
  return *::std::get<S_STRING_TYPE>(data_);
}

inline ::std::string Value::GetString() const {
  STELLA_ASSERT(type_ == S_BOOL || type_ == S_INTEGER || type_ == S_NUMBER || type_ == S_STRING);
  switch (type_) {
    case S_BOOL:return ::std::get<S_BOOL_TYPE>(data_) ? "true" : "false";
    case S_INTEGER:return ::std::to_string(::std::get<S_INTEGER_TYPE>(data_));
    case S_NUMBER:return ::std::to_string(::std::get<S_NUMBER_TYPE>(data_));
    case S_STRING:return ::std::string(GetStringView());
    default: STELLA_ASSERT(false);
  }
  return {};
}

inline const auto &Value::GetTable() const {
  STELLA_ASSERT(type_ == S_TABLE);
  return ::std::get<S_TABLE_TYPE>(data_);
}

inline Value &Value::SetBool(S_BOOL_TYPE b) {
  this->~Value();
  return *new(this) Value(b);
}

inline Value &Value::SetInteger(S_INTEGER_TYPE i) {
  this->~Value();
  return *new(this) Value(i);
}

inline Value &Value::SetNumber(S_NUMBER_TYPE n) {
  this->~Value();
  return *new(this) Value(n);
}

inline Value &Value::SetString(::std::string_view s) {
  this->~Value();
  return *new(this) Value(s);
}

inline Value &Value::SetTable() {
  this->~Value();
  return *new(this) Value(S_TABLE);
}

inline Value::MemberIterator Value::MemberBegin() {
  STELLA_ASSERT(type_ == S_TABLE);
  return ::std::get<S_TABLE_TYPE>(data_)->begin();
}

inline Value::MemberIterator Value::MemberEnd() {
  STELLA_ASSERT(type_ == S_TABLE);
  return ::std::get<S_TABLE_TYPE>(data_)->end();
}

inline Value::MemberIterator Value::FindMember(::std::size_t key) {
  STELLA_ASSERT(type_ == S_TABLE);
  return ::std::find_if(::std::get<S_TABLE_TYPE>(data_)->begin(),
                        ::std::get<S_TABLE_TYPE>(data_)->end(),
                        [key](const Member &member) -> bool {
                          return member.key_.IsInteger()
                              && member.key_.GetInteger() == static_cast<S_INTEGER_TYPE>(key);
                        });
}

inline Value::MemberIterator Value::FindMember(::std::string_view key) {
  STELLA_ASSERT(type_ == S_TABLE);
  return ::std::find_if(::std::get<S_TABLE_TYPE>(data_)->begin(),
                        ::std::get<S_TABLE_TYPE>(data_)->end(),
                        [key](const Member &member) -> bool {
                          return member.key_.IsString() && member.key_.GetStringView() == key;
                        });
}

inline Value::ConstMemberIterator Value::MemberBegin() const {
  STELLA_ASSERT(type_ == S_TABLE);
  return const_cast<Value &>(*this).MemberBegin();
}

inline Value::ConstMemberIterator Value::MemberEnd() const {
  STELLA_ASSERT(type_ == S_TABLE);
  return const_cast<Value &>(*this).MemberEnd();
}

inline Value::ConstMemberIterator Value::FindMember(::std::size_t key) const {
  STELLA_ASSERT(type_ == S_TABLE);
  return const_cast<Value &>(*this).FindMember(key);
}

inline Value::ConstMemberIterator Value::FindMember(::std::string_view key) const {
  STELLA_ASSERT(type_ == S_TABLE);
  return const_cast<Value &>(*this).FindMember(key);
}

inline Value &Value::operator=(const Value &val) {
  STELLA_ASSERT(this != &val);
  type_ = val.type_;
  data_ = val.data_;
  return *this;
}

inline Value &Value::operator=(Value &&val) noexcept {
  STELLA_ASSERT(this != &val);
  type_ = val.type_;
  data_ = ::std::move(val.data_);
  val.type_ = S_NIL;
  return *this;
}

inline Value &Value::operator[](::std::size_t key) {
  STELLA_ASSERT(type_ == S_TABLE);
  auto it = FindMember(key);
  if (it != ::std::get<S_TABLE_TYPE>(data_)->end()) {
    return it->value_;
  }
  STELLA_ASSERT(false && "value not found");
  static Value fake(S_NIL);
  return fake;
}

inline const Value &Value::operator[](::std::size_t key) const {
  STELLA_ASSERT(type_ == S_TABLE);
  return const_cast<Value &>(*this)[key];
}

inline Value &Value::operator[](::std::string_view key) {
  STELLA_ASSERT(type_ == S_TABLE);
  auto it = FindMember(key);
  if (it != ::std::get<S_TABLE_TYPE>(data_)->end()) {
    return it->value_;
  }
  STELLA_ASSERT(false && "value not found");
  static Value fake(S_NIL);
  return fake;
}

inline const Value &Value::operator[](::std::string_view key) const {
  STELLA_ASSERT(type_ == S_TABLE);
  return const_cast<Value &>(*this)[key];
}

template<typename T>
inline Value &Value::AddMember(::std::size_t key, T &&value) {
  return AddMember(Value(static_cast<S_INTEGER_TYPE>(key)), Value(::std::forward<T>(value)));
}

template<typename T>
inline Value &Value::AddMember(const char *key, T &&value) {
  return AddMember(Value(key), Value(::std::forward<T>(value)));
}

inline Value &Value::AddMember(Value &&key, Value &&value) {
  STELLA_ASSERT(type_ == S_TABLE);
  STELLA_ASSERT(key.type_ == S_INTEGER || key.type_ == S_STRING);
  STELLA_ASSERT(
      (key.type_ == S_INTEGER ? FindMember(key.GetInteger()) : FindMember(key.GetStringView())) == MemberEnd()
  );
  auto ptr = ::std::get<S_TABLE_TYPE>(data_);
  ptr->emplace_back(::std::move(key), ::std::move(value));
  return ptr->back().value_;
}

#define CALL_HANDLER(expr) do { if (!(expr)) { return false; } } while(false)

template<typename Handler>
inline bool Value::WriteTo(Handler &handler) const {
  switch (type_) {
    case S_NIL: CALL_HANDLER(handler.Nil());
      break;
    case S_BOOL: CALL_HANDLER(handler.Bool());
      break;
    case S_INTEGER: CALL_HANDLER(handler.Integer());
      break;
    case S_NUMBER: CALL_HANDLER(handler.Number());
      break;
    case S_STRING: CALL_HANDLER(handler.String());
      break;
    case S_TABLE: CALL_HANDLER(handler.StartTable());
      for (auto &member : *GetTable()) {
        member.key_.IsInteger() ? handler.Key(member.key_.GetInteger()) : handler.Key(member.key_.GetStringView());
        CALL_HANDLER(member.value_.WriteTo(handler));
      }
      CALL_HANDLER(handler.EndTable());
      break;
    default: STELLA_ASSERT(false && "bad type");
  }
  return true;
}

#undef CALL_HANDLER

} // namespace stella

#endif //STELLA_INCLUDE_STELLA_VALUE_H_


