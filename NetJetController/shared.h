#pragma once
#define _WIN32_WINNT 0x0500
#include <windows.h>

typedef DWORD CHUNK_ID;
typedef __int64 LONG_CHUNK_ID;

typedef DWORD CONTROL;
typedef CONTROL MAPPING;

inline void setButtonsControlMapping(CONTROL &control, MAPPING mapping, bool state = false) {
	if (state) {
		control |= mapping;
	}
}

inline bool testButtonsControlMapping(CONTROL control, MAPPING mapping) {
	return control & mapping;
}

inline void setThumbControlMapping(CONTROL &control, MAPPING mapping, bool state = false) {
	if (state) {
		control = mapping;
	}
}

bool readFileSafe(HANDLE file, LPVOID buffer, DWORD numberOfBytesToRead);
bool writeFileSafe(HANDLE file, LPCVOID buffer, DWORD numberOfBytesToWrite);