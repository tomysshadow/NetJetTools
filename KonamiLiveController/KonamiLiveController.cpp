#include "KonamiLiveController.h"
#include <windows.h>

HMODULE originalNetJetController;
HMODULE originalXbox360Controller;
NetJetSimulator netJetSimulator;








LRESULT __stdcall NetJetSimulator::Keyboard::backgroundThread(int nCode, WPARAM wParam, LPARAM lParam) {
	// event handler for when keyboard does something
	if (lParam && nCode >= 0) {
		// if onKeyDown or onKeyUp
		if (wParam == WM_KEYDOWN || wParam == WM_KEYUP) {
			// get reference to the system's keyboard
			netJetSimulator.keyboard.keyboardDLLHookStruct = *(KBDLLHOOKSTRUCT*)lParam;

			for (size_t i = 0;i < KEYS_SIZE;i++) {
				// if the keyCode matches one of our keycodes
				if (netJetSimulator.keyboard.keyCodes[i] == netJetSimulator.keyboard.keyboardDLLHookStruct.vkCode) {
					// reflect this in our keysDown array
					netJetSimulator.keyboard.keysDown[i] = (wParam == WM_KEYDOWN);
					break;
				}
			}
		}
	}
	// continue watching the keyboard
	return CallNextHookEx(netJetSimulator.keyboard.backgroundThreadHook, nCode, wParam, lParam);
}

inline void NetJetSimulator::setState(PDWORD a, bool down, DWORD mapping, bool replace = false) {
	// if the key/button is down
	if (a && down) {
		if (replace) {
			// set our mapping, ignoring previous value (i.e. don't go faster when both arrow keys and DPad is pressed etc.)
			// don't use this on buttons, it will have the effect of clearing all other buttons
			*a = mapping;
		} else {
			// set our mapping
			*a |= mapping;
		}
	}
}

inline void NetJetSimulator::setControllerInserted(PDWORD buttons, BOOL originalResult = TRUE, bool replace = false, bool thumbstickDown = false) {
	if (buttons) {
		bool down = (originalResult || (*buttons & 0x00010000) != 0x00010000);
		// replace previous value
		// considering the controller is apparently not inserted and it could have been anything
		setState(buttons, down, 0x00010000, replace);
	}

	//centerThumbstick(thumbRX, thumbRY, thumbstickDown);
}

void NetJetSimulator::callKonamiLiveGetState(PDWORD buttons, BOOL originalResult) {
	// call intercepted
	//POINT curpos;
	//int myWidth = GetSystemMetrics(SM_CXSCREEN);
	//int myHeight = GetSystemMetrics(SM_CYSCREEN);

	// ensure the game thinks a controller is inserted
	setControllerInserted(buttons, originalResult, true, true);
	// ensure the game thinks a cartridge is inserted and valid
	//setCartridgeInserted(buttons);

	// if the controller isn't suspended...
	if (!suspended) {
		// ignore 360 Controller if user doesn't have XInput
		if (originalXbox360Controller) {
			_XInputGetState originalXInputGetState;
			XINPUT_STATE xbox360ControllerState;
			DWORD xbox360ControllerInserted;

			// get the 360 Controller state
			ZeroMemory(&xbox360ControllerState, sizeof(XINPUT_STATE));
			originalXInputGetState = (_XInputGetState)GetProcAddress(originalXbox360Controller, "XInputGetState");

			if (originalXInputGetState) {
				xbox360ControllerInserted = originalXInputGetState(0, &xbox360ControllerState);
			} else {
				// do this here to avoid out of date values
				xbox360ControllerInserted = 1167L;
			}

			// if the 360 Controller is inserted
			if (xbox360ControllerInserted == 0L) {
				// set currently pressed buttons as down on KonamiLive Controller
				setState(buttons, xbox360ControllerState.Gamepad.wButtons      & 0x8000, 0x00000400);
				setState(buttons, xbox360ControllerState.Gamepad.wButtons      & 0x2000, 0x00000100);
				setState(buttons, xbox360ControllerState.Gamepad.wButtons      & 0x4000, 0x00000800);
				setState(buttons, xbox360ControllerState.Gamepad.wButtons      & 0x1000, 0x00000080);
				setState(buttons, xbox360ControllerState.Gamepad.bLeftTrigger  > 0x0000, 0x00000004);
				setState(buttons, xbox360ControllerState.Gamepad.bRightTrigger > 0x0000, 0x00000200);
				setState(buttons, xbox360ControllerState.Gamepad.wButtons      & 0x0100, 0x00000004);
				setState(buttons, xbox360ControllerState.Gamepad.wButtons      & 0x0200, 0x00000200);
				setState(buttons, xbox360ControllerState.Gamepad.wButtons      & 0x0010, 0x00000001);
				setState(buttons, xbox360ControllerState.Gamepad.sThumbLY      >   7849, 0x00000008);
				setState(buttons, xbox360ControllerState.Gamepad.sThumbLY      <  -7849, 0x00000040);
				setState(buttons, xbox360ControllerState.Gamepad.sThumbLX      <  -7849, 0x00000020);
				setState(buttons, xbox360ControllerState.Gamepad.sThumbLX      >   7849, 0x00000010);
				setState(buttons, xbox360ControllerState.Gamepad.wButtons      & 0x0001, 0x00000008);
				setState(buttons, xbox360ControllerState.Gamepad.wButtons      & 0x0002, 0x00000040);
				setState(buttons, xbox360ControllerState.Gamepad.wButtons      & 0x0004, 0x00000020);
				setState(buttons, xbox360ControllerState.Gamepad.wButtons      & 0x0008, 0x00000010);

				//setState(thumbRX, xbox360ControllerState.Gamepad.sThumbRX < -8689 || xbox360ControllerState.Gamepad.sThumbRX > 8689, ( xbox360ControllerState.Gamepad.sThumbRX / 65536.0 + 0.5) * 64.0, true);
				//setState(thumbRY, xbox360ControllerState.Gamepad.sThumbRY < -8689 || xbox360ControllerState.Gamepad.sThumbRY > 8689, (-xbox360ControllerState.Gamepad.sThumbRY / 65536.0 + 0.5) * 64.0, true);
			}
		}

		// if the game isn't already key mapping for us...
		//if (!keyMapping) {
			// set currently pressed keys as down on KonamiLive Controller
			// Pause
			setState(buttons, netJetSimulator.keyboard.keysDown[0] , 0x00000001);
			// Rotate Left
			setState(buttons, netJetSimulator.keyboard.keysDown[1] , 0x00000004);
			// MOVE UP
			setState(buttons, netJetSimulator.keyboard.keysDown[2] , 0x00000008);
			// MOVE RIGHT
			setState(buttons, netJetSimulator.keyboard.keysDown[3] , 0x00000010);
			// MOVE LEFT
			setState(buttons, netJetSimulator.keyboard.keysDown[4] , 0x00000020);
			// MOVE DOWN
			setState(buttons, netJetSimulator.keyboard.keysDown[5] , 0x00000040);
			// Super Hop (A)
			setState(buttons, netJetSimulator.keyboard.keysDown[6] , 0x00000080);
			// B
			setState(buttons, netJetSimulator.keyboard.keysDown[7] , 0x00000100);
			// Rotate Right
			setState(buttons, netJetSimulator.keyboard.keysDown[8] , 0x00000200);
			// High Hop
			setState(buttons, netJetSimulator.keyboard.keysDown[9] , 0x00000400);
			// Tongue Grab
			setState(buttons, netJetSimulator.keyboard.keysDown[10], 0x00000800);
		//}

		// ignore mouse mapping because game conflicts with it
		/*
		if (!mousemapping) {
			if (GetCursorPos(&curpos)) {
				if (myWidth > 0 && (curpos.x - (myWidth / 2)) != 0) {
					*thumbRX = 31 + ((curpos.x - (myWidth / 2)) / (double)myWidth * 31);
				}
				if (myHeight > 0 && (curpos.y - (myHeight / 2)) != 0) {
					*thumbRY = 31 + ((curpos.y - (myHeight / 2)) / (double)myHeight * 31);
				}
			}
			SetCursorPos(myWidth / 2, myHeight / 2);
		}
		*/
	}

	// ensure thumbsticks are not further up/down/left/right than they can be on the controller
	//fixThumbstick(thumbRX, thumbRY);
	// ensure the game thinks a controller is inserted and a cartridge is inserted and valid one more time just in case
	setControllerInserted(buttons);
	//setCartridgeInserted(buttons);
}








BOOL WINAPI DllMain(HINSTANCE hInst, DWORD fdwReason, LPVOID lpvReserved) {
	if (fdwReason == DLL_PROCESS_ATTACH) {
		DisableThreadLibraryCalls(hInst);
	}
	return TRUE;
}

typedef BOOL(*_KonamiLiveControllerInitialize)();
extern "C" BOOL callKonamiLiveControllerInitialize() {
	if (!originalNetJetController) {
		originalNetJetController = GetModuleHandle(L"Frogger.exe");

		if (!originalNetJetController) {
			return FALSE;
		}

		netJetSimulator.keyboard.backgroundThreadHook = SetWindowsHookEx(WH_KEYBOARD_LL, netJetSimulator.keyboard.backgroundThread, NULL, 0);
		ShowCursor(FALSE);
		originalXbox360Controller = LoadLibrary(L"XINPUT9_1_0.DLL");
	}

	_KonamiLiveControllerInitialize originalKonamiLiveControllerInitialize;
	//originalKonamiLiveControllerInitialize = (_KonamiLiveControllerInitialize)GetProcAddress(originalNetJetController, "KonamiLiveControllerInitialize");
	originalKonamiLiveControllerInitialize = (_KonamiLiveControllerInitialize)((PBYTE)originalNetJetController + 0x00185690);

	if (originalKonamiLiveControllerInitialize) {
		originalKonamiLiveControllerInitialize();
	}
	return TRUE;
}

typedef BOOL(*_KonamiLiveControllerSuspend)();
extern "C" BOOL callKonamiLiveControllerSuspend() {
	if (!originalNetJetController) {
		return FALSE;
	}

	_KonamiLiveControllerSuspend originalKonamiLiveControllerSuspend;
	//originalKonamiLiveControllerSuspend = (_KonamiLiveControllerSuspend)GetProcAddress(originalNetJetController, "KonamiLiveControllerSuspend");
	originalKonamiLiveControllerSuspend = (_KonamiLiveControllerSuspend)((PBYTE)originalNetJetController + 0x00185700);

	if (originalKonamiLiveControllerSuspend) {
		originalKonamiLiveControllerSuspend();
	}

	netJetSimulator.suspended = true;
	return TRUE;
}

typedef BOOL(*_KonamiLiveControllerResume)();
extern "C" BOOL callKonamiLiveControllerResume() {
	if (!originalNetJetController) {
		return FALSE;
	}

	_KonamiLiveControllerResume originalKonamiLiveControllerResume;
	//originalKonamiLiveControllerResume = (_KonamiLiveControllerResume)GetProcAddress(originalNetJetController, "KonamiLiveControllerResume");
	originalKonamiLiveControllerResume = (_KonamiLiveControllerResume)((PBYTE)originalNetJetController + 0x00185720);

	if (originalKonamiLiveControllerResume) {
		originalKonamiLiveControllerResume();
	}

	netJetSimulator.suspended = false;
	return TRUE;
}

typedef BOOL(__cdecl *_KonamiLiveControllerGetState)(PDWORD);
extern "C" BOOL __cdecl callKonamiLiveControllerGetState(PDWORD buttons) {
	if (!originalNetJetController) {
		return FALSE;
	}

	_KonamiLiveControllerGetState originalKonamiLiveControllerGetState;
	//originalKonamiLiveControllerGetState = (_KonamiLiveControllerGetState)GetProcAddress(originalNetJetController, "KonamiLiveControllerGetState");
	originalKonamiLiveControllerGetState = (_KonamiLiveControllerGetState)((PBYTE)originalNetJetController + 0x00185740);
	BOOL originalResult = false;

	if (originalKonamiLiveControllerGetState) {
		originalResult = originalKonamiLiveControllerGetState(buttons);
	}

	netJetSimulator.callKonamiLiveGetState(buttons, originalResult);
	return TRUE;
}

typedef BOOL(__cdecl *_KonamiLiveControllerSetWindow)(HWND);
extern "C" BOOL __cdecl callKonamiLiveControllerSetWindow(HWND hWnd) {
	return TRUE;
}