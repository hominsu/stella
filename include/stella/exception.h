//
// Created by Homin Su on 2023/6/9.
//

#ifndef STELLA_INCLUDE_STELLA_EXCEPTION_H_
#define STELLA_INCLUDE_STELLA_EXCEPTION_H_

#include <exception>

#include "stella.h"

namespace stella {

#undef ERR_TABLE
#define ERR_TABLE(_field_error) \
  _field_error(OK, "ok")        \
  _field_error(ROOT_NOT_SINGULAR, "root not singular") \
  _field_error(BAD_VALUE, "bad value")                 \
  _field_error(EXPECT_VALUE, "expect value")           \
  _field_error(USER_STOPPED, "user stopped Parse")     \
  //

namespace error {
enum ParseError {
#undef ERR_KEY
#define ERR_KEY(_err, _str) _err,
  ERR_TABLE(ERR_KEY)
#undef ERR_KEY
};
} // namespace error

inline const char *ParseErrorStr(error::ParseError err) {
  const static char *err_str_table[] = {
#undef ERR_VALUE
#define ERR_VALUE(_err, _str) _str,
      ERR_TABLE(ERR_VALUE)
#undef ERR_VALUE
  };

  STELLA_ASSERT(err >= 0 && err < STELLA_STR_LENGTH(err_str_table));
  return err_str_table[err];
}

#undef ERR_TABLE

class Exception : public ::std::exception {
 private:
  error::ParseError err_;

 public:
  explicit Exception(error::ParseError err) : err_(err) {}

  [[nodiscard]] const char *what() const noexcept override { return ParseErrorStr(err_); }
  [[nodiscard]] error::ParseError err() const { return err_; }
};

} // namespace stella

#endif //STELLA_INCLUDE_STELLA_EXCEPTION_H_
