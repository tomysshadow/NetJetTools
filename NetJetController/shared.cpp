#include "shared.h"
#include <windows.h>

bool readFileSafe(HANDLE file, LPVOID buffer, DWORD numberOfBytesToRead) {
	if (!file || file == INVALID_HANDLE_VALUE) {
		return false;
	}

	if (!buffer) {
		return false;
	}

	DWORD numberOfBytesRead = 0;

	if (!ReadFile(file, buffer, numberOfBytesToRead, &numberOfBytesRead, NULL) || numberOfBytesToRead != numberOfBytesRead) {
		return false;
	}
	return true;
}

bool writeFileSafe(HANDLE file, LPCVOID buffer, DWORD numberOfBytesToWrite) {
	if (!file || file == INVALID_HANDLE_VALUE) {
		return false;
	}

	if (!buffer) {
		return false;
	}

	DWORD numberOfBytesWritten = 0;

	if (!WriteFile(file, buffer, numberOfBytesToWrite, &numberOfBytesWritten, NULL) || numberOfBytesToWrite != numberOfBytesWritten) {
		return false;
	}
	return true;
}