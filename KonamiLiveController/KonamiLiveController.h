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

typedef DWORD(WINAPI *_XInputGetState)(_In_ DWORD dwUserIndex, _Out_ XINPUT_STATE *pState);

class NetJetSimulator {
	public:
	class Keyboard {
		private:
		KBDLLHOOKSTRUCT keyboardDLLHookStruct;
		static const size_t KEYS_SIZE = 11;
		const DWORD keyCodes[KEYS_SIZE] = { VK_SPACE, 0x41, VK_UP, VK_RIGHT, VK_LEFT, VK_DOWN, 0x43, VK_BACK, 0x44, 0x56, 0x53 };
		public:
		static LRESULT __stdcall backgroundThread(int, WPARAM, LPARAM);
		HHOOK backgroundThreadHook;
		bool keysDown[KEYS_SIZE] = { false, false, false, false, false, false, false, false, false, false, false };
	};

	Keyboard keyboard;
	bool suspended = false;
	void callKonamiLiveGetState(PDWORD, BOOL);
	private:
	inline void setState(PDWORD, bool, DWORD, bool);
	inline void setControllerInserted(PDWORD, BOOL, bool, bool);
	//inline void setCartridgeInserted(PDWORD, bool);
};