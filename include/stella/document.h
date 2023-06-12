//
// Created by Homin Su on 2023/6/11.
//

#ifndef STELLA_INCLUDE_STELLA_DOCUMENT_H_
#define STELLA_INCLUDE_STELLA_DOCUMENT_H_

#include <cstddef>

#include <utility>
#include <vector>

#include "exception.h"
#include "reader.h"
#include "stella.h"
#include "stella/state.h"
#include "value.h"
#include "variant"

#ifdef __GNUC__
STELLA_DIAG_PUSH
STELLA_DIAG_OFF(effc++)
#endif // __GNUC__

namespace stella {

class Document : public Value {
 private:
  struct Level {
    Value *value_;
    int value_count_;

    explicit Level(Value *value) : value_(value), value_count_(0) {}

    [[nodiscard]] Type type() const { return value_->type_; }
    [[nodiscard]] Value *last_value() const;
  };

  ::std::vector<Level> stack_;
  Value key_;
  bool see_value_ = false;

 public:
  error::ParseError Parse(State &state, ::std::string_view name);
  error::ParseError ParseState(State &state);

  // handler
  bool Nil();
  bool Bool(bool b);
  bool Integer(LUA_INTEGER i);
  bool Number(LUA_NUMBER n);
  bool String(::std::string_view str);
  bool Key(LUA_INTEGER i);
  bool Key(::std::string_view str);
  bool StartTable();
  bool EndTable();

 private:
  Value *AddValue(Value &&value);
};

inline Value *Document::Level::last_value() const {
  return &::std::get<S_TABLE>(value_->data_)->back().value_;
}

inline error::ParseError Document::Parse(State &state, ::std::string_view name) {
  state.GetGlobal(name);
  return Reader::Parse(state, *this);
}

inline error::ParseError Document::ParseState(State &state) {
  state.PushGlobalTable();
  return Reader::Parse(state, *this);
}

inline bool Document::Nil() {
  AddValue(Value(S_NIL));
  return true;
}

inline bool Document::Bool(bool b) {
  AddValue(Value(b));
  return true;
}

inline bool Document::Integer(long long int i) {
  AddValue(Value(i));
  return true;
}

inline bool Document::Number(double n) {
  AddValue(Value(n));
  return true;
}

inline bool Document::String(::std::string_view str) {
  AddValue(Value(str));
  return true;
}

inline bool Document::Key(long long int i) {
  AddValue(Value(i));
  return true;
}

inline bool Document::Key(::std::string_view str) {
  AddValue(Value(str));
  return true;
}

inline bool Document::StartTable() {
  stack_.emplace_back(AddValue(Value(S_TABLE)));
  return true;
}

inline bool Document::EndTable() {
  STELLA_ASSERT(!stack_.empty());
  STELLA_ASSERT(stack_.back().type() == S_TABLE);
  stack_.pop_back();
  return true;
}

inline Value *Document::AddValue(Value &&value) {
  auto type = value.GetType();
  (void) type;

  if (see_value_) { STELLA_ASSERT(!stack_.empty() && "root not singular"); }
  else {
    STELLA_ASSERT(type_ == S_NIL);
    see_value_ = true;
    type_ = type;

    switch (type) {
      case S_NIL:break;
      case S_BOOL:data_.emplace<S_BOOL>(::std::get<S_BOOL>(value.data_));
        break;
      case S_INTEGER: data_.emplace<S_INTEGER>(::std::get<S_INTEGER>(value.data_));
        break;
      case S_NUMBER: data_.emplace<S_NUMBER>(::std::get<S_NUMBER>(value.data_));
        break;
      case S_STRING: data_.emplace<S_STRING>(::std::get<S_STRING>(value.data_));
        ::std::get<S_STRING>(value.data_) = nullptr;
        break;
      case S_TABLE: data_.emplace<S_TABLE>(::std::get<S_TABLE>(value.data_));
        ::std::get<S_TABLE>(value.data_) = nullptr;
        break;
      default:break;
    }
    value.type_ = S_NIL;
    return this;
  }

  auto &top = stack_.back();
  STELLA_ASSERT(top.type() == S_TABLE);
  if (top.value_count_ % 2 == 0) {
    STELLA_ASSERT(type == S_INTEGER || type == S_STRING);
    key_ = ::std::move(value);
    ++top.value_count_;
    return &key_;
  } else {
    top.value_->AddMember(::std::move(key_), ::std::move(value));
    ++top.value_count_;
    return top.last_value();
  }
}

} // namespace stella

#endif //STELLA_INCLUDE_STELLA_DOCUMENT_H_
