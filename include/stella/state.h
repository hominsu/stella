//
// Created by Homin Su on 2023/6/8.
//

#ifndef STELLA_INCLUDE_STELLA_STATE_H_
#define STELLA_INCLUDE_STELLA_STATE_H_

#include <cstddef>
#include <cstdio>
#include <cstring>

#include <memory>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

#include "exception.h"
#include "stella.h"
#include "value.h"

#include <lua.hpp>

namespace stella {

class State {
 private:
  lua_State *lua_state_ = nullptr;

 public:
  State() = default;
  State(const State &other) = default;
  State &operator=(State other);
  State(State &&other) noexcept: lua_state_(other.lua_state_) {};
  State &operator=(State &&other) noexcept;
  friend void swap(State &s1, State &s2);

  void LoadFile(::std::string_view file);
  void LoadString(::std::string_view script);
  void Call();
  void Destroy();

  [[nodiscard]] ::std::string Version() const;
  [[nodiscard]] ::std::size_t StackSize() const;

  bool HasNext(int index);
  Type GetType(int index);
  void GetGlobal(::std::string_view name);
  void PushGlobalTable();

  bool IsNil(int index);
  bool IsBool(int index);
  bool IsNumber(int index);
  bool IsInteger(int index);
  bool IsString(int index);
  bool IsTable(int index);

  void Push(::std::nullptr_t val);
  void Push(bool val);
  void Push(LUA_INTEGER val);
  void Push(LUA_NUMBER val);
  void Push(const char *val);
  void Push(::std::string_view val);
  void Push(lua_CFunction val);
  void Push(void *val);

  template<typename T>
  ::std::enable_if_t<::std::is_integral_v<T> && !::std::is_same_v<T, bool>> Push(T val);

  template<typename T>
  ::std::enable_if_t<::std::is_floating_point_v<T>> Push(T val);

  bool Get(bool *val, int index);
  bool Get(LUA_INTEGER *val, int index);
  bool Get(LUA_NUMBER *val, int index);
  bool Get(::std::string *val, int index);

  template<typename T>
  ::std::enable_if_t<::std::is_integral_v<T> && !::std::is_same_v<T, bool>, bool> Get(T *val, int index);

  template<typename T>
  ::std::enable_if_t<::std::is_floating_point_v<T>, bool> Get(T *val, int index);

  bool Top(bool *val);
  bool Top(LUA_INTEGER *val);
  bool Top(LUA_NUMBER *val);
  bool Top(::std::string *val);

  template<typename T>
  ::std::enable_if_t<::std::is_integral_v<T> && !::std::is_same_v<T, bool>, bool> Top(T *val);

  template<typename T>
  ::std::enable_if_t<::std::is_floating_point_v<T>, bool> Top(T *val);

  void Pop(::std::size_t size);
  void Pop();

 private:
  static int error_handling(lua_State *lua_state);
};

inline State &State::operator=(State other) {
  swap(*this, other);
  return *this;
}

inline State &State::operator=(State &&other) noexcept {
  swap(*this, other);
  return *this;
}

inline void swap(State &s1, State &s2) {
  ::std::swap(s1.lua_state_, s2.lua_state_);
}

inline void State::Call() {
  if (auto status = lua_pcall(lua_state_, 0, 0, 1); status != LUA_OK) {
    fprintf(stderr, "error code:%d, msg:%s\n", status, lua_tostring(lua_state_, -1));
  }
}

inline void State::LoadFile(::std::string_view file) {
  STELLA_ASSERT(lua_state_ == nullptr && "lua state not closed");
  lua_state_ = luaL_newstate();
  luaL_openlibs(lua_state_);
  lua_pushcfunction(lua_state_, error_handling);
  luaL_loadfile(lua_state_, file.data());
}

inline void State::LoadString(::std::string_view script) {
  STELLA_ASSERT(lua_state_ == nullptr && "lua state not closed");
  lua_state_ = luaL_newstate();
  luaL_openlibs(lua_state_);
  lua_pushcfunction(lua_state_, error_handling);
  luaL_loadstring(lua_state_, script.data());
}

inline void State::Destroy() {
  STELLA_ASSERT(lua_state_ != nullptr && "lua state already closed");
  lua_close(lua_state_);
  lua_state_ = nullptr;
}

inline ::std::string State::Version() const {
  char buf[32]{};
  int n = snprintf(buf, sizeof(buf), "%.17g", lua_version(lua_state_));
  (void) n;
  STELLA_ASSERT(n > 0 && n < 32);
  return {buf};
}

inline ::std::size_t State::StackSize() const {
  return lua_gettop(lua_state_);
}

inline bool State::HasNext(int index) {
  return lua_next(lua_state_, index);
}

inline Type State::GetType(int index) {
  switch (lua_type(lua_state_, index)) {
    case LUA_TNIL:return S_NIL;
    case LUA_TBOOLEAN:return S_BOOL;
    case LUA_TNUMBER:return S_NUMBER;
    case LUA_TSTRING:return S_STRING;
    case LUA_TTABLE:return S_TABLE;
    default: throw Exception(error::BAD_VALUE);
  }
}

inline void State::GetGlobal(::std::string_view name) {
  lua_getglobal(lua_state_, ::std::string(name).c_str());
}

inline void State::PushGlobalTable() {
  lua_pushglobaltable(lua_state_);
}

inline bool State::IsNil(int index) {
  return lua_isnil(lua_state_, index);
}

inline bool State::IsBool(int index) {
  return lua_isboolean(lua_state_, index);
}

inline bool State::IsNumber(int index) {
  return lua_isnumber(lua_state_, index);
}

inline bool State::IsInteger(int index) {
  return lua_isinteger(lua_state_, index);
}

inline bool State::IsString(int index) {
  return lua_isstring(lua_state_, index);
}

inline bool State::IsTable(int index) {
  return lua_istable(lua_state_, index);
}

inline void State::Push(::std::nullptr_t val) {
  (void) val;
  lua_pushnil(lua_state_);
}

inline void State::Push(bool val) {
  lua_pushboolean(lua_state_, val);
}

inline void State::Push(LUA_INTEGER val) {
  lua_pushinteger(lua_state_, val);
}

inline void State::Push(LUA_NUMBER val) {
  lua_pushnumber(lua_state_, val);
}

inline void State::Push(const char *val) {
  lua_pushlstring(lua_state_, val, ::std::strlen(val));
}

inline void State::Push(::std::string_view val) {
  lua_pushlstring(lua_state_, val.data(), val.size());
}

inline void State::Push(lua_CFunction val) {
  lua_pushcfunction(lua_state_, val);
}

inline void State::Push(void *val) {
  lua_pushlightuserdata(lua_state_, val);
}

template<typename T>
inline ::std::enable_if_t<::std::is_integral_v<T> && !::std::is_same_v<T, bool>> State::Push(T val) {
  Push(static_cast<LUA_INTEGER>(val));
}

template<typename T>
inline ::std::enable_if_t<::std::is_floating_point_v<T>> State::Push(T val) {
  Push(static_cast<LUA_NUMBER>(val));
}

inline bool State::Get(bool *val, int index) {
  if (lua_isboolean(lua_state_, index)) {
    *val = lua_toboolean(lua_state_, index);
    return true;
  }
  return false;
}

inline bool State::Get(LUA_INTEGER *val, int index) {
  if (lua_isinteger(lua_state_, index)) {
    *val = lua_tointeger(lua_state_, index);
    return true;
  }
  return false;
}

inline bool State::Get(LUA_NUMBER *val, int index) {
  if (lua_isnumber(lua_state_, index)) {
    *val = lua_tonumber(lua_state_, index);
    return true;
  }
  return false;
}

inline bool State::Get(std::string *val, int index) {
  if (lua_isstring(lua_state_, index)) {
    *val = lua_tostring(lua_state_, index);
    return true;
  }
  return false;
}

template<typename T>
inline ::std::enable_if_t<::std::is_integral_v<T> && !::std::is_same_v<T, bool>, bool> State::Get(T *val, int index) {
  if (lua_isinteger(lua_state_, index)) {
    *val = static_cast<T>(lua_tointeger(lua_state_, index));
    return true;
  }
  return false;
}

template<typename T>
inline ::std::enable_if_t<::std::is_floating_point_v<T>, bool> State::Get(T *val, int index) {
  if (lua_isnumber(lua_state_, index)) {
    *val = static_cast<T>(lua_tonumber(lua_state_, index));
    return true;
  }
  return false;
}

inline bool State::Top(bool *val) {
  return Get(val, -1);
}

inline bool State::Top(LUA_INTEGER *val) {
  return Get(val, -1);
}

inline bool State::Top(LUA_NUMBER *val) {
  return Get(val, -1);
}

inline bool State::Top(std::string *val) {
  return Get(val, -1);
}

template<typename T>
inline ::std::enable_if_t<::std::is_integral_v<T> && !::std::is_same_v<T, bool>, bool> State::Top(T *val) {
  return Get(val, -1);
}

template<typename T>
inline ::std::enable_if_t<::std::is_floating_point_v<T>, bool> State::Top(T *val) {
  return Get(val, -1);
}

inline void State::Pop(::std::size_t size) {
  lua_pop(lua_state_, size);
}

inline void State::Pop() {
  Pop(1);
}

inline int State::error_handling(lua_State *lua_state) {
  fprintf(stderr, "error: %s\n", lua_tostring(lua_state, -1));
  lua_pop(lua_state, 1);

  lua_Debug debug;
  int level = 0;
  ::std::string ret;

  while (lua_getstack(lua_state, level, &debug)) {
    auto len = ::std::snprintf(nullptr, 0,
                               "\tat %s:%d in function '%s'\n", debug.short_src, debug.currentline, debug.name);
    auto buf = ::std::make_unique<char[]>(len + 1);
    ::std::memset(buf.get(), '\0', len + 1);
    ::std::snprintf(buf.get(), len,
                    "\tat %s:%d in function '%s'\n", debug.short_src, debug.currentline, debug.name);
    ++level;
  }

  ::std::string res = debug.short_src;
  res += ":";
  res += ::std::to_string(debug.currentline);
  if (debug.name != nullptr) {
    res += "(";
    res += debug.namewhat;
    res += " ";
    res += debug.name;
    res += ")";
  }
  lua_pushstring(lua_state, res.c_str());

  return 1;
}

} // namespace stella

#endif //STELLA_INCLUDE_STELLA_STATE_H_
