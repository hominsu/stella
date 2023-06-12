//
// Created by Homin Su on 2023/6/8.
//

#ifndef STELLA_INCLUDE_STELLA_READER_H_
#define STELLA_INCLUDE_STELLA_READER_H_

#include <string>

#include "exception.h"
#include "non_copyable.h"
#include "state.h"
#include "value.h"

namespace stella {

class Reader : NonCopyable {
 public:
  template<typename Handler>
  static error::ParseError Parse(State &state, Handler &handler);

 private:
  template<typename Handler>
  static void ParseNil(State &state, Handler &handler);

  template<typename Handler>
  static void ParseBool(State &state, Handler &handler);

  template<typename Handler>
  static void ParseInteger(State &state, Handler &handler, bool is_key);

  template<typename Handler>
  static void ParseNumber(State &state, Handler &handler);

  template<typename Handler>
  static void ParseString(State &state, Handler &handler, bool is_key);

  template<typename Handler>
  static void ParseTable(State &state, Handler &handler);

  template<typename Handler>
  static void ParseValue(State &state, Handler &handler);
};

template<typename Handler>
inline error::ParseError Reader::Parse(State &state, Handler &handler) {
  try {
    ParseValue(state, handler);
//    if (state.HasNext(-1)) { throw Exception(error::ROOT_NOT_SINGULAR); }
    return error::OK;
  } catch (Exception &e) {
    return e.err();
  }
}

#define CALL(expr) if (!(expr)) throw Exception(error::USER_STOPPED)

template<typename Handler>
inline void Reader::ParseNil(State &state, Handler &handler) {
  CALL(handler.Nil());
  state.Pop();
}

template<typename Handler>
inline void Reader::ParseBool(State &state, Handler &handler) {
  if (bool val; state.Top(&val)) { CALL(handler.Bool(val)); }
  state.Pop();
}

template<typename Handler>
inline void Reader::ParseInteger(State &state, Handler &handler, bool is_key) {
  int index = is_key ? -2 : -1;
  if (LUA_INTEGER val; state.Get(&val, index)) {
    if (is_key) { CALL(handler.Key(val)); }
    else { CALL(handler.Integer(val)); }
  }
  if (!is_key) { state.Pop(); }
}

template<typename Handler>
inline void Reader::ParseNumber(State &state, Handler &handler) {
  if (LUA_NUMBER val; state.Top(&val)) { CALL(handler.Number(val)); }
  state.Pop();
}

template<typename Handler>
inline void Reader::ParseString(State &state, Handler &handler, bool is_key) {
  int index = is_key ? -2 : -1;
  if (::std::string val; state.Get(&val, index)) {
    if (is_key) { CALL(handler.Key(val)); }
    else { CALL(handler.String(val)); }
  }
  if (!is_key) { state.Pop(); }
}

template<typename Handler>
inline void Reader::ParseTable(State &state, Handler &handler) {
  CALL(handler.StartTable());
  state.Push(nullptr);
  while (state.HasNext(-2)) {
    state.IsInteger(-2) ? ParseInteger(state, handler, true) : ParseString(state, handler, true);
    ParseValue(state, handler);
  }
  CALL(handler.EndTable());
  state.Pop();
}

#undef CALL

template<typename Handler>
inline void Reader::ParseValue(State &state, Handler &handler) {
//  if (!state.HasNext(-1)) { throw Exception(error::EXPECT_VALUE); }

  switch (state.GetType(-1)) {
    case S_NIL: return ParseNil(state, handler);
    case S_BOOL: return ParseBool(state, handler);
    case S_NUMBER: return ParseNumber(state, handler);
    case S_STRING: return ParseString(state, handler, false);
    case S_TABLE: return ParseTable(state, handler);
    default: throw Exception(error::BAD_VALUE);
  }
}

} // namespace stella

#endif //STELLA_INCLUDE_STELLA_READER_H_
