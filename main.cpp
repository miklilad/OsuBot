#include <iostream>
#include "OsuProcess.h"
#include "assert.h"

//const char * OSUPATH = R"(C:\Users\Ladislav\AppData\Local\osu!\Songs)";

int main() {
  OsuProcess process;
  if(!process.isRunning())
    return 1;
  std::string beatmap = process.getMapName();
  std::cout << process.getWindowRect().top << std::endl;

  return 0;
}