//
// Created by Homin Su on 2023/6/12.
//

#include <string>

#include "sample.h"
#include "stella/document.h"
#include "stella/exception.h"
#include "stella/non_copyable.h"
#include "stella/state.h"

#include "bencode/file_write_stream.h"
#include "bencode/writer.h"

template<typename Handler>
class ToBencode : stella::NonCopyable {
 private:
  Handler &handler_;

 public:
  explicit ToBencode(Handler &handler) : handler_(handler) {}

  bool Nil() { return handler_.Null(); }
  bool Bool(bool b) { return handler_.Integer(b); }
  bool Integer(LUA_INTEGER i) { return handler_.Integer(i); }
  bool Number(LUA_NUMBER n) { return handler_.Integer(static_cast<LUA_INTEGER>(n)); }
  bool String(std::string_view str) { return handler_.String(str); }
  bool Key(std::string_view str) { return handler_.Key(str); }
  bool Key(LUA_INTEGER i) { return handler_.Key(::std::to_string(i)); }
  bool StartTable() { return handler_.StartDict(); }
  bool EndTable() { return handler_.EndDict(); }
};

int main(int argc, char *argv[]) {
  (void) argc;
  (void) argv;

  stella::State state;
  state.LoadString(kSample[0]);
  state.Call();

  bencode::FileWriteStream out(stdout);
  bencode::Writer writer(out);
  ToBencode to_bencode(writer);

  state.GetGlobal("Application");
  auto err = stella::Reader::Parse(state, to_bencode);
  if (err != stella::error::OK) {
    puts(stella::ParseErrorStr(err));
    return EXIT_FAILURE;
  }

  return 0;
}
