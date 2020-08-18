#pragma once
#include <windows.h>








typedef struct _XINPUT_GAMEPAD {
	WORD  wButtons;
	BYTE  bLeftTrigger;
	BYTE  bRightTrigger;
	SHORT sThumbLX;
	SHORT sThumbLY;
	SHORT sThumbRX;
	SHORT sThumbRY;
} XINPUT_GAMEPAD, *PXINPUT_GAMEPAD;

typedef struct _XINPUT_STATE {
	DWORD dwPacketNumber;
	XINPUT_GAMEPAD Gamepad;
} XINPUT_STATE, *PXINPUT_STATE;

typedef DWORD(*_XInputGetState)(_In_ DWORD dwUserIndex, _Out_ XINPUT_STATE *pState);

class NetJetSimulator {
	public:
	class Keyboard {
		private:
		KBDLLHOOKSTRUCT keyboardDLLHookStruct;
		static const size_t KEYS_SIZE = 24;
		const DWORD keyCodes[KEYS_SIZE] = { 0x34, 0x32, 0x33, 0x31, 0x4C, 0x52, 0x30, 0x57, 0x53, 0x41, 0x44, VK_UP, VK_LEFT, VK_DOWN, VK_RIGHT, 0x46, 0x51, 0x45, VK_BACK, VK_RETURN, VK_SPACE, VK_LSHIFT, VK_RSHIFT, VK_ESCAPE };
		public:
		static LRESULT __stdcall backgroundThread(int, WPARAM, LPARAM);
		HHOOK backgroundThreadHook;
		bool keysDown[KEYS_SIZE] = { false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false };
	};

	Keyboard keyboard;
	bool keyMapping = false;
	bool mouseMapping = false;
	bool suspended = false;
	void callNetJetControllerGetState(PDWORD, PDWORD, PDWORD, BOOL);
	void callNetJetControllerGetKey(PVOID);
	private:
	inline void fixThumbstick(PDWORD, PDWORD);
	inline void centerThumbstick(PDWORD, PDWORD, bool);
	inline void setState(PDWORD, bool, DWORD, bool);
	inline void setControllerInserted(PDWORD, PDWORD, PDWORD, BOOL, bool, bool);
	inline void setCartridgeInserted(PDWORD, bool);
};