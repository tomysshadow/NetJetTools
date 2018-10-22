#pragma once
#include <windows.h>








class NetJetEmulator {
	public:
	bool keyMapping = false;
	bool mouseMapping = false;
	bool suspended = false;
	void callNetJetControllerGetState(PDWORD, PDWORD, PDWORD, BOOL);
	void callNetJetControllerGetKey(PVOID, BOOL);
	class Keyboard {
		private:
		KBDLLHOOKSTRUCT keyboardDLLHookStruct;
		static const size_t KEYS_LENGTH = 24;
		const DWORD keyCodes[KEYS_LENGTH] = {0x34, 0x32, 0x33, 0x31, 0x4C, 0x52, 0x30, 0x57, 0x53, 0x41, 0x44, VK_UP, VK_LEFT, VK_DOWN, VK_RIGHT, 0x46, 0x51, 0x45, VK_BACK, VK_RETURN, VK_SPACE, VK_LSHIFT, VK_RSHIFT, VK_ESCAPE};
		public:
		static LRESULT __stdcall backgroundThread(int, WPARAM, LPARAM);
		HHOOK backgroundThreadHook;
		bool keysDown[KEYS_LENGTH] = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false};
	};
	private:
	inline void fixThumbstick(PDWORD, PDWORD);
	inline void centerThumbstick(PDWORD, PDWORD, bool);
	inline void setState(PDWORD, bool, DWORD, bool);
	inline void setControllerInserted(PDWORD, PDWORD, PDWORD, BOOL, bool, bool);
	inline void setCartridgeInserted(PDWORD, bool);
};