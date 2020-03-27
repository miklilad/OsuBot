#include "OsuProcess.h"

OsuProcess::OsuProcess() : running(true) {
  uintptr_t ptr = patternMatchMemory("osu!.exe", "A3????????8D65F45B5E5F5DC3DDD8E9"); //"A3????????8D65F45B5E5F5DC3DDD8E9"
  if(!ptr) {
    std::cout << "Osu! is not running!" << std::endl;
    running = false;
    return;
  }
  processHandle = OpenProcess(PROCESS_VM_READ, false, getProcessID("osu!.exe"));

  HWND handle = GetTopWindow(nullptr);
  bool found = false;
  while(!found && handle) {
    char name[200];
    GetWindowTextA(handle, name, sizeof(name));
    std::string s(name);
    if(s.find("osu!") == std::string::npos) {
      handle = GetNextWindow(handle, GW_HWNDNEXT);
    } else {
      found = true;
    }
  }
  if(!found) {
    std::cout << "Could't find the osu! window!" << std::endl;
    running = false;
    return;
  }
  windowHandle = handle;
  ShowWindow(windowHandle, SW_RESTORE);
  SetForegroundWindow(windowHandle);
  timeAddress = (uintptr_t) Read4Bytes(processHandle, ptr);;
}

OsuProcess::~OsuProcess() {
  if(running){
    CloseHandle(windowHandle);
    CloseHandle(processHandle);
  }
}

bool OsuProcess::isRunning() {
  return running;
}

uint64_t OsuProcess::getTime() {
  return isRunning() ? Read4Bytes(processHandle, timeAddress) : 0;
}

RECT OsuProcess::getWindowRect() {
  RECT rect;
  GetClientRect(windowHandle,&rect);
  return rect;
}

std::string OsuProcess::getMapName() {
  if(!isRunning())
    return std::string();
  size_t length = GetWindowTextLengthA(windowHandle) + 2;
  char * name = new char[length];
  GetWindowTextA(windowHandle, name, length);
  std::string res(name);
  delete[] name;
  if(res == "osu!" || res.length() < 9)
    return std::string();
  return res.substr(8);
}
