//
// Created by Homin Su on 2023/6/8.
//

#ifndef STELLA_INCLUDE_STELLA_NON_COPYABLE_H_
#define STELLA_INCLUDE_STELLA_NON_COPYABLE_H_

namespace stella {

class NonCopyable {
 public:
  NonCopyable(const NonCopyable &) = delete;
  NonCopyable &operator=(const NonCopyable &) = delete;

 protected:
  NonCopyable() = default;
  ~NonCopyable() = default;
};

} // namespace stella

#endif //STELLA_INCLUDE_STELLA_NON_COPYABLE_H_
