
#ifndef OSUBOT_MEMTOOLS_H
#define OSUBOT_MEMTOOLS_H

#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <windows.h>
#include <tlhelp32.h>


DWORD getProcessID(const char * name);

MODULEENTRY32 getModule(DWORD procID,
                        const char * name);

uint64_t Read4Bytes(HANDLE hProcess,
                    uintptr_t ptr);

/**
 * Taken from https://forum.cheatengine.org/viewtopic.php?p=5705743
 *
 * Locates a pattern of data within a large block of memory.
 *
 * @param {std::vector} data - The data to scan for the pattern within.
 * @param {uintptr_t} baseAddress - The base address to add to the offset when the pattern is found.
 * @param {const char*} pattern - The pattern to scan for.
 * @param {uintptr_t} offset - The offset to add after the pattern has been found.
 * @param {uintptr_t} count - The result count to use if the pattern is found multiple times.
 * @returns {uintptr_t} The address where the pattern was located, 0 otherwise.
 */
uintptr_t FindPattern(const std::vector<uint8_t>& data,
                      uintptr_t baseAddress,
                      const char * pattern,
                      uintptr_t offset,
                      uintptr_t count);

uintptr_t patternMatchMemory(const char * procName,
                             const char * pattern);

void printModules(const char * name);

#endif //OSUBOT_MEMTOOLS_H
