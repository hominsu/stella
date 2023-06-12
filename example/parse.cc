//
// Created by Homin Su on 2023/6/7.
//

#include <cstdio>
#include <cstdlib>

#include "sample.h"
#include "stella/document.h"
#include "stella/state.h"

int main(int argc, char *argv[]) {
  (void) argc;
  (void) argv;

  stella::State state;
  state.LoadString(kSample[0]);
  state.Call();

  // 1. Parse a Bencode string into DOM.
  stella::Document doc;
  auto err = doc.Parse(state, "Application");
  if (err != stella::error::OK) {
    puts(stella::ParseErrorStr(err));
    return EXIT_FAILURE;
  }

  // 2. Modify it by DOM.
  auto &m = doc["Modified"];
  m.SetBool(true);

  // 3. Output the DOM
  auto array = doc["Array"];
  fprintf(stdout,
          "Array: [ %f, %f, %f, %f, %s ]\n",
          array[1].GetNumber(),
          array[2].GetNumber(),
          array[3].GetNumber(),
          array[4].GetNumber(),
          array[5].GetString().c_str()
  );
  fprintf(stdout, "Width: %lld\n", doc["Width"].GetInteger());
  fprintf(stdout, "Height: %lld\n", doc["Height"].GetInteger());
  fprintf(stdout, "Name: %s\n", doc["Name"].GetStringView().data());
  fprintf(stdout, "Modified: %s\n", doc["Modified"].GetBool() ? "true" : "false");

  state.Destroy();

  return 0;
}
