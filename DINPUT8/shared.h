#pragma once
#define _WIN32_WINNT 0x0500
#include <windows.h>
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

inline bool memoryEqual(const void* buffer, const void* buffer2, size_t bufferSize) {
	return !memcmp(buffer, buffer2, bufferSize);
}