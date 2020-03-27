
#ifndef OSUBOT_OSUPROCESS_H
#define OSUBOT_OSUPROCESS_H

#include "memtools.h"

class OsuProcess {
  bool running;
  HWND windowHandle;
  HANDLE processHandle;
  uintptr_t timeAddress;
public:
  OsuProcess();
  ~OsuProcess();
  bool isRunning();
  uint64_t getTime();
  std::string getMapName();
  RECT getWindowRect();
};

#endif //OSUBOT_OSUPROCESS_H
