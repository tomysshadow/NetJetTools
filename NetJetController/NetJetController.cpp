#include "NetJetController.h"

#include <windows.h>
#include "myXInput.h"

HINSTANCE originalNetJetController;
HINSTANCE original360Controller;
NetJetEmulator netJetEmulator;
NetJetEmulator::Keyboard netJetEmulatorKeyboard;








LRESULT __stdcall NetJetEmulator::Keyboard::backgroundThread(int nCode, WPARAM wParam, LPARAM lParam) {
	// event handler for when keyboard does something
	if (lParam && nCode >= 0) {
		// if onKeyDown or onKeyUp
		if (wParam == WM_KEYDOWN || wParam == WM_KEYUP) {
			// get reference to the system's keyboard
			netJetEmulatorKeyboard.keyboardDLLHookStruct = *(KBDLLHOOKSTRUCT*)lParam;
			for (size_t i = 0;i < KEYS_LENGTH;i++) {
				// if the keyCode matches one of our keycodes
				if (netJetEmulatorKeyboard.keyCodes[i] == netJetEmulatorKeyboard.keyboardDLLHookStruct.vkCode) {
					// reflect this in our keysDown array
					netJetEmulatorKeyboard.keysDown[i] = (wParam == WM_KEYDOWN);
					break;
				}
			}
		}
	}
	// continue watching the keyboard
	return CallNextHookEx(netJetEmulatorKeyboard.backgroundThreadHook, nCode, wParam, lParam);
}

inline void NetJetEmulator::fixThumbstick(PDWORD bThumbRX, PDWORD bThumbRY) {
	if (bThumbRX) {
		if (*bThumbRX < 0) {
			*bThumbRX = 0;
		}
		if (*bThumbRX > 64) {
			*bThumbRX = 64;
		}
	}
	if (bThumbRY) {
		if (*bThumbRY < 0) {
			*bThumbRY = 0;
		}
		if (*bThumbRY > 64) {
			*bThumbRY = 64;
		}
	}
}

inline void NetJetEmulator::centerThumbstick(PDWORD bThumbRX, PDWORD bThumbRY, bool thumbstickDown = false) {
	// centre the thumbstick
	// considering the controller was apparently not inserted
	const DWORD THUMBSTICK_CENTER = 0x0000001F;
	if (bThumbRX) {
		setState(bThumbRX, thumbstickDown, THUMBSTICK_CENTER, true);
	}
	if (bThumbRY) {
		setState(bThumbRY, thumbstickDown, THUMBSTICK_CENTER, true);
	}
}

inline void NetJetEmulator::setState(PDWORD a, bool down, DWORD mapping, bool replace = false) {
	// if the key/button is down
	if (a && down) {
		if (!replace) {
			// set our mapping
			*a |= mapping;
		} else {
			// set our mapping, ignoring previous value (i.e. don't go faster when both arrow keys and DPad is pressed etc.)
			// don't use this on wButtons, it will have the effect of clearing all other buttons
			*a  = mapping;
		}
	}
}

inline void NetJetEmulator::setControllerInserted(PDWORD wButtons, PDWORD bThumbRX, PDWORD bThumbRY, BOOL result = TRUE, bool replace = false, bool thumbstickDown = false) {
	if (wButtons) {
		bool down = (!result || (*wButtons & 0x00010000) != 0x00010000);
		// replace previous value
		// considering the controller is apparently not inserted and it could have been anything
		setState(wButtons, down, 0x00010000, replace);
	}
	centerThumbstick(bThumbRX, bThumbRY, thumbstickDown);
}

inline void NetJetEmulator::setCartridgeInserted(PDWORD wButtons, bool replace = false) {
	bool down = (*wButtons & 0x00100080) != 0x00100080;
	setState(wButtons, down, 0x00100080, replace);
}

void NetJetEmulator::callNetJetControllerGetState(PDWORD wButtons, PDWORD bThumbRX, PDWORD bThumbRY, BOOL result) {
	// call intercepted
	//POINT curpos;
	//int myWidth = GetSystemMetrics(SM_CXSCREEN);
	//int myHeight = GetSystemMetrics(SM_CYSCREEN);

	// ensure the game thinks a controller is inserted
	setControllerInserted(wButtons, bThumbRX, bThumbRY, result, true, true);
	// ensure the game thinks a cartridge is inserted and valid
	setCartridgeInserted(wButtons);
	// if the controller isn't suspended...
	if (!suspended) {
		// ignore 360 Controller if user doesn't have XInput
		if (original360Controller) {
			XInputGetState_ originalXInputGetState;
			XINPUT_STATE the360ControllerState;
			DWORD theXbox360ControllerInserted;
			// get the 360 Controller state
			ZeroMemory(&the360ControllerState, sizeof(XINPUT_STATE));
			originalXInputGetState = (XInputGetState_)GetProcAddress(original360Controller, "XInputGetState");
			if (!originalXInputGetState) {
				// do this here to avoid out of date values
				theXbox360ControllerInserted = 1167L;
			} else {
				theXbox360ControllerInserted = originalXInputGetState(0, &the360ControllerState);
			}
			// if the 360 Controller is inserted
			if (theXbox360ControllerInserted == 0L) {
				// set currently pressed wButtons as down on NetJet Controller
				setState(wButtons, the360ControllerState.Gamepad.wButtons      & 0x8000, 0x00000001);
				setState(wButtons, the360ControllerState.Gamepad.wButtons      & 0x2000, 0x00000002);
				setState(wButtons, the360ControllerState.Gamepad.wButtons      & 0x4000, 0x00000004);
				setState(wButtons, the360ControllerState.Gamepad.wButtons      & 0x1000, 0x00000008);
				setState(wButtons, the360ControllerState.Gamepad.bLeftTrigger  > 0x0000, 0x00000010);
				setState(wButtons, the360ControllerState.Gamepad.bRightTrigger > 0x0000, 0x00000020);
				setState(wButtons, the360ControllerState.Gamepad.wButtons      & 0x0100, 0x00000010);
				setState(wButtons, the360ControllerState.Gamepad.wButtons      & 0x0200, 0x00000020);
				setState(wButtons, the360ControllerState.Gamepad.wButtons      & 0x0010, 0x00000100);
				setState(wButtons, the360ControllerState.Gamepad.sThumbLY      >   7849, 0x00000200);
				setState(wButtons, the360ControllerState.Gamepad.sThumbLY      <  -7849, 0x00000400);
				setState(wButtons, the360ControllerState.Gamepad.sThumbLX      <  -7849, 0x00000800);
				setState(wButtons, the360ControllerState.Gamepad.sThumbLX      >   7849, 0x00001000);
				setState(wButtons, the360ControllerState.Gamepad.wButtons      & 0x0001, 0x00000200);
				setState(wButtons, the360ControllerState.Gamepad.wButtons      & 0x0002, 0x00000400);
				setState(wButtons, the360ControllerState.Gamepad.wButtons      & 0x0004, 0x00000800);
				setState(wButtons, the360ControllerState.Gamepad.wButtons      & 0x0008, 0x00001000);

				setState(bThumbRX, the360ControllerState.Gamepad.sThumbRX < -8689 || the360ControllerState.Gamepad.sThumbRX > 8689, ( the360ControllerState.Gamepad.sThumbRX / 65536.0 + 0.5) * 64.0, true);
				setState(bThumbRY, the360ControllerState.Gamepad.sThumbRY < -8689 || the360ControllerState.Gamepad.sThumbRY > 8689, (-the360ControllerState.Gamepad.sThumbRY / 65536.0 + 0.5) * 64.0, true);
			}
		}

		// if the game isn't already key mapping for us...
		if (!keyMapping) {
			// set currently pressed keys as down on NetJet Controller
			// Button 4
			setState(wButtons, netJetEmulatorKeyboard.keysDown[15], 0x00000001);
			setState(wButtons, netJetEmulatorKeyboard.keysDown[0] , 0x00000001);
			// Button 2
			setState(wButtons, netJetEmulatorKeyboard.keysDown[18], 0x00000002);
			setState(wButtons, netJetEmulatorKeyboard.keysDown[16], 0x00000002);
			setState(wButtons, netJetEmulatorKeyboard.keysDown[1] , 0x00000002);
			// Button 3
			setState(wButtons, netJetEmulatorKeyboard.keysDown[17], 0x00000004);
			setState(wButtons, netJetEmulatorKeyboard.keysDown[2] , 0x00000004);
			// Button 1
			setState(wButtons, netJetEmulatorKeyboard.keysDown[20], 0x00000008);
			setState(wButtons, netJetEmulatorKeyboard.keysDown[19], 0x00000008);
			setState(wButtons, netJetEmulatorKeyboard.keysDown[3] , 0x00000008);
			// Left Shoulder
			setState(wButtons, netJetEmulatorKeyboard.keysDown[21], 0x00000010);
			setState(wButtons, netJetEmulatorKeyboard.keysDown[4] , 0x00000010);
			// Right Shoulder
			setState(wButtons, netJetEmulatorKeyboard.keysDown[22], 0x00000020);
			setState(wButtons, netJetEmulatorKeyboard.keysDown[5] , 0x00000020);
			// Start
			setState(wButtons, netJetEmulatorKeyboard.keysDown[23], 0x00000100);
			setState(wButtons, netJetEmulatorKeyboard.keysDown[6] , 0x00000100);
			// DPad Up
			setState(wButtons, netJetEmulatorKeyboard.keysDown[7] , 0x00000200);
			// DPad Down
			setState(wButtons, netJetEmulatorKeyboard.keysDown[8] , 0x00000400);
			// DPad Left
			setState(wButtons, netJetEmulatorKeyboard.keysDown[9] , 0x00000800);
			// DPad Right
			setState(wButtons, netJetEmulatorKeyboard.keysDown[10], 0x00001000);
			// Right Thumbstick
			setState(bThumbRY, netJetEmulatorKeyboard.keysDown[11], 0x00000000, true);
			setState(bThumbRX, netJetEmulatorKeyboard.keysDown[12], 0x00000000, true);
			setState(bThumbRY, netJetEmulatorKeyboard.keysDown[13], 0x00000040, true);
			setState(bThumbRX, netJetEmulatorKeyboard.keysDown[14], 0x00000040, true);
		}

		// ignore mouse mapping because game conflicts with it
		/*
		if (!mousemapping) {
			if (GetCursorPos(&curpos)) {
				if (myWidth > 0 && (curpos.x - (myWidth / 2)) != 0) {
					*bThumbRX = 31 + ((curpos.x - (myWidth / 2)) / (double)myWidth * 31);
				}
				if (myHeight > 0 && (curpos.y - (myHeight / 2)) != 0) {
					*bThumbRY = 31 + ((curpos.y - (myHeight / 2)) / (double)myHeight * 31);
				}
			}
			SetCursorPos(myWidth / 2, myHeight / 2);
		}
		*/
	}

	// ensure thumbsticks are not further up/down/left/right than they can be on the controller
	fixThumbstick(bThumbRX, bThumbRY);
	// ensure the game thinks a controller is inserted and a cartridge is inserted and valid one more time just in case
	setControllerInserted(wButtons, bThumbRX, bThumbRY, true, false, false);
	setCartridgeInserted(wButtons);
}

void NetJetEmulator::callNetJetControllerGetKey(PVOID key, BOOL result) {
	// call intercepted

	// zero key, this is not a keygen
	// replicating real keys is outside the scope of this project
	if (key && !result) {
		ZeroMemory(key, 0x20);
	}
}








BOOL WINAPI DllMain(HINSTANCE hInst, DWORD fdwReason, LPVOID lpvReserved)
{
	switch (fdwReason) {
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hInst);
		originalNetJetController = LoadLibraryA("NetJetController_.DLL");
		if (!originalNetJetController) {
			return false;
		}

		netJetEmulatorKeyboard.backgroundThreadHook = SetWindowsHookEx(WH_KEYBOARD_LL, netJetEmulatorKeyboard.backgroundThread, NULL, 0);
		ShowCursor(FALSE);
		break;
	case DLL_PROCESS_DETACH:
		FreeLibrary(originalNetJetController);

		UnhookWindowsHookEx(netJetEmulatorKeyboard.backgroundThreadHook);
		ShowCursor(TRUE);
		break;
	}
	return true;
}

// duplicate functions which call the original and return false
// we always want these calls to appear successful
// returning false means no error ocurred
typedef DWORD(*NetJetControllerEnableKeyMapping_)();
extern "C" DWORD callNetJetControllerEnableKeyMapping()
{
	NetJetControllerEnableKeyMapping_ originalNetJetControllerEnableKeyMapping;

	originalNetJetControllerEnableKeyMapping = (NetJetControllerEnableKeyMapping_)GetProcAddress(originalNetJetController, "NetJetControllerEnableKeyMapping");
	if (!originalNetJetControllerEnableKeyMapping) {
	} else {
		originalNetJetControllerEnableKeyMapping();
	}

	netJetEmulator.keyMapping = true;
	UnhookWindowsHookEx(netJetEmulatorKeyboard.backgroundThreadHook);

	return 0;
}

typedef DWORD(*NetJetControllerDisableKeyMapping_)();
extern "C" DWORD callNetJetControllerDisableKeyMapping()
{
	NetJetControllerDisableKeyMapping_ originalNetJetControllerDisableKeyMapping;

	originalNetJetControllerDisableKeyMapping = (NetJetControllerDisableKeyMapping_)GetProcAddress(originalNetJetController, "NetJetControllerDisableKeyMapping");
	if (!originalNetJetControllerDisableKeyMapping) {
	} else {
		originalNetJetControllerDisableKeyMapping();
	}

	netJetEmulator.keyMapping = false;
	netJetEmulatorKeyboard.backgroundThreadHook = SetWindowsHookEx(WH_KEYBOARD_LL, netJetEmulatorKeyboard.backgroundThread, NULL, 0);

	return 0;
}

typedef DWORD(*NetJetControllerEnableMouseMapping_)();
extern "C" DWORD callNetJetControllerEnableMouseMapping()
{
	NetJetControllerEnableMouseMapping_ originalNetJetControllerEnableMouseMapping;

	originalNetJetControllerEnableMouseMapping = (NetJetControllerEnableMouseMapping_)GetProcAddress(originalNetJetController, "NetJetControllerEnableMouseMapping");
	if (!originalNetJetControllerEnableMouseMapping) {
	} else {
		originalNetJetControllerEnableMouseMapping();
	}

	netJetEmulator.mouseMapping = true;
	ShowCursor(true);

	return 0;
}

typedef DWORD(*NetJetControllerDisableMouseMapping_)();
extern "C" DWORD callNetJetControllerDisableMouseMapping()
{
	NetJetControllerDisableMouseMapping_ originalNetJetControllerDisableMouseMapping;

	originalNetJetControllerDisableMouseMapping = (NetJetControllerDisableMouseMapping_)GetProcAddress(originalNetJetController, "NetJetControllerDisableMouseMapping");
	if (!originalNetJetControllerDisableMouseMapping) {
	} else {
		originalNetJetControllerDisableMouseMapping();
	}

	netJetEmulator.mouseMapping = false;
	ShowCursor(false);

	return 0;
}

typedef DWORD(*NetJetControllerInitialize_)();
extern "C" DWORD callNetJetControllerInitialize()
{
	NetJetControllerInitialize_ originalNetJetControllerInitialize;

	originalNetJetControllerInitialize = (NetJetControllerInitialize_)GetProcAddress(originalNetJetController, "NetJetControllerInitialize");
	if (!originalNetJetControllerInitialize) {
	} else {
		originalNetJetControllerInitialize();
	}

	original360Controller = LoadLibrary(L"XINPUT9_1_0.DLL");

	return 0;
}

typedef BOOL(*NetJetControllerSuspend_)();
extern "C" BOOL callNetJetControllerSuspend()
{
	NetJetControllerSuspend_ originalNetJetControllerSuspend;

	originalNetJetControllerSuspend = (NetJetControllerSuspend_)GetProcAddress(originalNetJetController, "NetJetControllerSuspend");
	if (!originalNetJetControllerSuspend) {
	} else {
		originalNetJetControllerSuspend();
	}
	netJetEmulator.suspended = true;

	return false;
}

typedef BOOL(*NetJetControllerResume_)();
extern "C" BOOL callNetJetControllerResume()
{
	NetJetControllerResume_ originalNetJetControllerResume;

	originalNetJetControllerResume = (NetJetControllerSuspend_)GetProcAddress(originalNetJetController, "NetJetControllerResume");
	if (!originalNetJetControllerResume) {
	} else {
		originalNetJetControllerResume();
	}
	netJetEmulator.suspended = false;

	return false;
}

typedef DWORD(*NetJetControllerShutdown_)();
extern "C" DWORD callNetJetControllerShutdown()
{
	NetJetControllerShutdown_ originalNetJetControllerShutdown;

	originalNetJetControllerShutdown = (NetJetControllerShutdown_)GetProcAddress(originalNetJetController, "NetJetControllerShutdown");
	if (!originalNetJetControllerShutdown) {
	} else {
		originalNetJetControllerShutdown();
	}

	return 0;
}

typedef DWORD(*NetJetControllerSetKeyMapping_)(WORD);
extern "C" DWORD callNetJetControllerSetKeyMapping(WORD wButtons)
{
	NetJetControllerSetKeyMapping_ originalNetJetControllerSetKeyMapping;

	originalNetJetControllerSetKeyMapping = (NetJetControllerSetKeyMapping_)GetProcAddress(originalNetJetController, "NetJetControllerSetKeyMapping");
	if (!originalNetJetControllerSetKeyMapping) {
	} else {
		originalNetJetControllerSetKeyMapping(wButtons);
	}

	return 0;
}

typedef BOOL(__cdecl *NetJetControllerSetOption_)(WORD, WORD);
extern "C" BOOL __cdecl callNetJetControllerSetOption(WORD wButtons, WORD nPriority)
{
	NetJetControllerSetOption_ originalNetJetControllerSetOption;

	originalNetJetControllerSetOption = (NetJetControllerSetOption_)GetProcAddress(originalNetJetController, "NetJetControllerSetOption");
	if (!originalNetJetControllerSetOption) {
	} else {
		originalNetJetControllerSetOption(wButtons, nPriority);
	}

	return false;
}

typedef BOOL(__cdecl *NetJetControllerGetState_)(PDWORD, PDWORD, PDWORD);
extern "C" BOOL __cdecl callNetJetControllerGetState(PDWORD wButtons, PDWORD bThumbRX, PDWORD bThumbRY)
{
	BOOL result = false;
	NetJetControllerGetState_ originalNetJetControllerGetState;

	originalNetJetControllerGetState = (NetJetControllerGetState_)GetProcAddress(originalNetJetController, "NetJetControllerGetState");
	if (!originalNetJetControllerGetState) {
	}
	else {
		result = !originalNetJetControllerGetState(wButtons, bThumbRX, bThumbRY);
	}

	netJetEmulator.callNetJetControllerGetState(wButtons, bThumbRX, bThumbRY, result);

	return false;
}

typedef BOOL(__cdecl *NetJetControllerSetWindow_)(HWND);
extern "C" BOOL __cdecl callNetJetControllerSetWindow(HWND hWnd)
{
	NetJetControllerSetWindow_ originalNetJetControllerSetWindow;

	originalNetJetControllerSetWindow = (NetJetControllerSetWindow_)GetProcAddress(originalNetJetController, "NetJetControllerSetWindow");
	if (!originalNetJetControllerSetWindow) {
	} else {
		originalNetJetControllerSetWindow(hWnd);
	}

	return false;
}

typedef BOOL(__cdecl *NetJetControllerGetControllerKey_)(int);
extern "C" BOOL __cdecl callNetJetControllerGetControllerKey(int wButtons)
{
	BOOL result = false;
	NetJetControllerGetControllerKey_ originalNetJetControllerGetControllerKey;

	originalNetJetControllerGetControllerKey = (NetJetControllerGetControllerKey_)GetProcAddress(originalNetJetController, "NetJetControllerGetControllerKey");
	if (!originalNetJetControllerGetControllerKey) {
	} else {
		result = !originalNetJetControllerGetControllerKey(wButtons);
	}

	netJetEmulator.callNetJetControllerGetKey((char* )wButtons, result);

	return false;
}

typedef BOOL(__cdecl *NetJetControlleretCartrdigeKey_)(int);
extern "C" BOOL __cdecl callNetJetControlleretCartrdigeKey(int wButtons)
{
	BOOL result = false;
	NetJetControlleretCartrdigeKey_ originalNetJetControlleretCartrdigeKey;

	originalNetJetControlleretCartrdigeKey = (NetJetControlleretCartrdigeKey_)GetProcAddress(originalNetJetController, "NetJetControlleretCartrdigeKey");
	if (!originalNetJetControlleretCartrdigeKey) {
	} else {
		result = !originalNetJetControlleretCartrdigeKey(wButtons);
	}

	netJetEmulator.callNetJetControllerGetKey((char* )wButtons, result);

	return false;
}

typedef DWORD(*NetJetControllerRun_)();
extern "C" DWORD callNetJetControllerRun()
{
	NetJetControllerRun_ originalNetJetControllerRun;

	originalNetJetControllerRun = (NetJetControllerRun_)GetProcAddress(originalNetJetController, "NetJetControllerRun");
	if (!originalNetJetControllerRun) {
	} else {
		originalNetJetControllerRun();
	}

	return 0;
}