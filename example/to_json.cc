//
// Created by Homin Su on 2023/6/12.
//

#include <string>

#include "sample.h"
#include "stella/document.h"
#include "stella/exception.h"
#include "stella/non_copyable.h"
#include "stella/state.h"

#include "neujson/file_write_stream.h"
#include "neujson/pretty_writer.h"

template<typename Handler>
class ToJSON : stella::NonCopyable {
 private:
  Handler &handler_;

 public:
  explicit ToJSON(Handler &handler) : handler_(handler) {}

  bool Nil() { return handler_.Null(); }
  bool Bool(bool b) { return handler_.Bool(b); }
  bool Integer(LUA_INTEGER i) { return handler_.Int64(i); }
  bool Number(LUA_NUMBER n) { return handler_.Double(n); }
  bool String(std::string_view str) { return handler_.String(str); }
  bool Key(std::string_view str) { return handler_.Key(str); }
  bool Key(LUA_INTEGER i) { return handler_.Key(::std::to_string(i)); }
  bool StartTable() { return handler_.StartObject(); }
  bool EndTable() { return handler_.EndObject(); }
};

int main(int argc, char *argv[]) {
  (void) argc;
  (void) argv;

  stella::State state;
  state.LoadString(kSample[0]);
  state.Call();

  neujson::FileWriteStream out(stdout);
  neujson::PrettyWriter pretty_writer(out);
  pretty_writer.SetIndent(' ', 2);
  ToJSON to_json(pretty_writer);

  state.GetGlobal("Application");
  auto err = stella::Reader::Parse(state, to_json);
  if (err != stella::error::OK) {
    puts(stella::ParseErrorStr(err));
    return EXIT_FAILURE;
  }

  return 0;
}
