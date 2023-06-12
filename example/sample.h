//
// Created by Homin Su on 2023/6/12.
//

#ifndef STELLA_EXAMPLE_SAMPLE_H_
#define STELLA_EXAMPLE_SAMPLE_H_

#include <string_view>

constexpr std::string_view kSample[] = {
    R"lua(
Application = {
    Array = { 11, 22, 33, 44, 55 },
    Width = 480,
    Height = 800,
    Name = "Homing So",
    Modified = false,
}
print(Application)
)lua"
};

#endif //STELLA_EXAMPLE_SAMPLE_H_
