#include "memtools.h"

DWORD getProcessID(const char * name) {
  HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); //all processes
  if(!snap)
    return 0;
  PROCESSENTRY32 entry = {0};
  entry.dwSize = sizeof(entry);
  if(!Process32First(snap, &entry))
    return 0;
  do {
    if(!strcmp(entry.szExeFile, name)) {
      CloseHandle(snap);
      return entry.th32ProcessID;
    }
  } while(Process32Next(snap, &entry));
  CloseHandle(snap);
  return 0;
}

HANDLE getProcessHandle(const char * name) {
  DWORD id = getProcessID(name);
  return OpenProcess(PROCESS_VM_READ, false, id);
}

MODULEENTRY32 getModule(DWORD procID, const char * name) {
  MODULEENTRY32 entry = {0};
  HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procID);
  if(snap == INVALID_HANDLE_VALUE)
    return entry;
  entry.dwSize = sizeof(entry);
  if(!Module32First(snap, &entry)) {
    CloseHandle(snap);
    return entry;
  }
  while(Module32Next(snap, &entry)) {
    //std::cout << entry.szModule << " - " << std::hex << (uintptr_t)entry.modBaseAddr << std::dec <<
    //", " << entry.modBaseSize << std::endl;
    if(!strcmp(entry.szModule, name)) {
      CloseHandle(snap);
      return entry;
    }
  }
  CloseHandle(snap);
  return {0};
}

uintptr_t FindPattern(const std::vector<uint8_t>& data, uintptr_t baseAddress, const char * pattern, uintptr_t offset,
                      uintptr_t count) {
  // Ensure the incoming pattern is properly aligned..
  if(strlen(pattern) % 2 > 0)
    return 0;

  // Convert the pattern to a vector of data..
  std::vector<std::pair<uint8_t, bool>> vpattern;
  for(size_t x = 0, y = strlen(pattern) / 2; x < y; x++) {
    // Obtain the current byte..
    std::stringstream stream(std::string(pattern + (x * 2), 2));

    // Check if this is a wildcard..
    if(stream.str() == "??")
      vpattern.push_back(std::make_pair(00, false));
    else {
      auto byte = strtol(stream.str().c_str(), nullptr, 16);
      vpattern.push_back(std::make_pair((uint8_t) byte, true));
    }
  }

  auto scanStart = data.begin();
  auto result = (uintptr_t) 0;

  while(true) {
    // Search for the pattern..
    auto ret = std::search(scanStart, data.end(), vpattern.begin(), vpattern.end(),
                           [&](uint8_t curr, std::pair<uint8_t, bool> currPattern) {
                             return (!currPattern.second) || curr == currPattern.first;
                           });

    // Did we find a match..
    if(ret != data.end()) {
      // If we hit the usage count, return the result..
      if(result == count || count == 0)
        return (std::distance(data.begin(), ret) + baseAddress) + offset;

      // Increment the found count and scan again..
      ++result;
      scanStart = ++ret;
    } else
      break;
  }
  return 0;
}

uintptr_t patternMatchMemory(const char * procName, const char * pattern) {
  DWORD id = getProcessID(procName);
  if(!id)
    return 0;
  MODULEENTRY32 module = getModule(id, procName);
  HANDLE handle = OpenProcess(PROCESS_VM_READ, false, id);
  if(!handle) {
    return 0;
  }
  auto current = (uintptr_t) module.modBaseAddr;
  uintptr_t end = current + module.modBaseSize;
  SIZE_T bytesRead;
  while(current <= 0x7fffffff) {
    uint8_t buffer[4096];
    DWORD oldProtect;
    VirtualProtectEx(handle, (void *) current, sizeof(buffer), PROCESS_VM_READ, &oldProtect);
    ReadProcessMemory(handle, (void *) current, &buffer, sizeof(buffer), &bytesRead);
    VirtualProtectEx(handle, (void *) current, sizeof(buffer), oldProtect, nullptr);

    if(bytesRead == 0) {
      current += 4096;
      continue;
    }
    std::vector<uint8_t> vec;
    for(int i = 0; i < bytesRead; ++i) {
      vec.push_back(buffer[i]);
    }
    uintptr_t ptr = FindPattern(vec, current, pattern, 1, 0);
    if(ptr) {
      CloseHandle(handle);
      return ptr;
    }
    current += bytesRead;
  }
  CloseHandle(handle);
  return 0;
}

void printModules(const char * name) {
  DWORD id = getProcessID(name);
  MODULEENTRY32 entry = {0};
  HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, id);
  if(snap == INVALID_HANDLE_VALUE)
    return;
  entry.dwSize = sizeof(entry);
  if(!Module32First(snap, &entry)) {
    CloseHandle(snap);
    return;
  }
  while(Module32Next(snap, &entry)) {
    std::cout << entry.szModule << " - " << std::hex << (uintptr_t)entry.modBaseAddr << std::dec <<
    ", " << entry.modBaseSize << std::endl;
  }
  CloseHandle(snap);
}

uint64_t Read4Bytes(HANDLE hProcess, uintptr_t ptr) {
  unsigned char buffer[4];
  SIZE_T bytesRead;
  ReadProcessMemory(hProcess, (void *) ptr, &buffer, sizeof(buffer), &bytesRead);
  if(bytesRead == 0)
    return 0;
  uint64_t value = 0;
  for(int i = sizeof(buffer) - 1; i >= 0; --i) {
    value <<= 8;
    value += (uint64_t)buffer[i];
  }
  return value;
}
