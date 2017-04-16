#include <windows.h>
#include "NetJetController.h"
#include "myXInput.h"

/*
#include <string.h>
#include <fstream>
#include <iostream>
*/

//#pragma comment(lib, "XInput.lib")

HINSTANCE originalNetJetController;
HINSTANCE original360Controller;
//std::basic_ofstream<char> f;
NetJetEmulator NJ;
NetJetEmulator::Keyboard KB;








LRESULT __stdcall NetJetEmulator::Keyboard::backgroundThread(int nCode, WPARAM wParam, LPARAM lParam) {
	// event handler for when keyboard does something
	if (nCode >= 0) {
		// if onKeyDown or onKeyUp
		if (wParam == WM_KEYDOWN || wParam == WM_KEYUP) {
			// get reference to the system's keyboard
			KB.keyboard = *((KBDLLHOOKSTRUCT*)lParam);
			for (byte i = 0;i < keyslength;i++) {
				// if the keyCode matches one of our keycodes
				if (KB.keyboard.vkCode == KB.keycodes[i]) {
					// reflect this in our keysdown array
					KB.keysdown[i] = (wParam == WM_KEYDOWN);
					break;
				}
			}
		}
	}
	// continue watching the keyboard
	return CallNextHookEx(KB.backgroundThreadHook, nCode, wParam, lParam);
}

void NetJetEmulator::SetState(int *a, bool down, int mapping, bool override=false) {
	// if the key/button is down
	if (down)
	{
		if (!override) {
			// set our mapping
			*a |= mapping;
		} else {
			// set our mapping, ignoring previous value (i.e. don't go faster when both arrow keys and DPad is pressed etc.)
			// don't use this on Gamepad_wButtons, it will have the effect of clearing all other buttons
			*a  = mapping;
		}
	}
}

void NetJetEmulator::SetControllerInserted(int *Gamepad_wButtons, int *Gamepad_bThumbRX, int *Gamepad_bThumbRY, bool result = true, bool override = false, bool downthumbstick = false) {
	bool down = (!result || (*Gamepad_wButtons & 0x00010000) != 0x00010000);
	// override previous value
	// considering the controller is apparently not inserted and it could have been anything
	SetState(Gamepad_wButtons, down, 0x00010000, override);
	// centre the thumbstick
	// considering the controller was apparently not inserted
	SetState(Gamepad_bThumbRX, downthumbstick, 0x0000001F, true);
	SetState(Gamepad_bThumbRY, downthumbstick, 0x0000001F, true);
}

void NetJetEmulator::SetCartridgeInserted(int *Gamepad_wButtons, bool override = false) {
	bool down = ((*Gamepad_wButtons & 0x00100080) != 0x00100080);
	SetState(Gamepad_wButtons, down, 0x00100080, override);
}

void NetJetEmulator::FixThumbstick(int *Gamepad_bThumbRX, int *Gamepad_bThumbRY) {
	if (*Gamepad_bThumbRX < 0) {
		*Gamepad_bThumbRX = 0;
	}
	if (*Gamepad_bThumbRX > 64) {
		*Gamepad_bThumbRX = 64;
	}
	if (*Gamepad_bThumbRY < 0) {
		*Gamepad_bThumbRY = 0;
	}
	if (*Gamepad_bThumbRY > 64) {
		*Gamepad_bThumbRY = 64;
	}
}

void NetJetEmulator::callNetJetControllerGetState(int *Gamepad_wButtons, int *Gamepad_bThumbRX, int *Gamepad_bThumbRY, bool result) {
	// call intercepted
	//POINT curpos;
	//int myWidth = GetSystemMetrics(SM_CXSCREEN);
	//int myHeight = GetSystemMetrics(SM_CYSCREEN);

	/*
	if (f.is_open()) {
		f << "NetJetControllerGetState " << *Gamepad_wButtons << " " << *Gamepad_bThumbRX << " " << *Gamepad_bThumbRY << "\n";
	}
	*/

	// ensure the game thinks a controller is inserted
	SetControllerInserted(Gamepad_wButtons, Gamepad_bThumbRX, Gamepad_bThumbRY, result, true, true);
	// ensure the game thinks a cartridge is inserted and valid
	SetCartridgeInserted(Gamepad_wButtons);
	// if the controller isn't suspended...
	if (!suspended) {
		// ignore 360 Controller if user doesn't have XInput
		if (!original360Controller) {
		} else {
			XInputGetState_ originalXInputGetState;
			XINPUT_STATE the360ControllerState;
			DWORD theXbox360ControllerInserted;
			// get the 360 Controller state
			memset(&the360ControllerState, 0x00, sizeof(XINPUT_STATE));
			originalXInputGetState = (XInputGetState_)GetProcAddress(original360Controller, "XInputGetState");
			if (!originalXInputGetState) {
				// do this here to avoid out of date values
				theXbox360ControllerInserted = 1167L;
			}
			else {
				theXbox360ControllerInserted = originalXInputGetState(0, &the360ControllerState);
			}
			// if the 360 Controller is inserted
			if (theXbox360ControllerInserted == 0L) {
				// set currently pressed Gamepad_wButtons as down on NetJet Controller
				SetState(Gamepad_wButtons, the360ControllerState.Gamepad.wButtons      & 0x8000, 0x00000001);
				SetState(Gamepad_wButtons, the360ControllerState.Gamepad.wButtons      & 0x2000, 0x00000002);
				SetState(Gamepad_wButtons, the360ControllerState.Gamepad.wButtons      & 0x4000, 0x00000004);
				SetState(Gamepad_wButtons, the360ControllerState.Gamepad.wButtons      & 0x1000, 0x00000008);
				SetState(Gamepad_wButtons, the360ControllerState.Gamepad.bLeftTrigger  > 0x0000, 0x00000010);
				SetState(Gamepad_wButtons, the360ControllerState.Gamepad.bRightTrigger > 0x0000, 0x00000020);
				SetState(Gamepad_wButtons, the360ControllerState.Gamepad.wButtons      & 0x0100, 0x00000010);
				SetState(Gamepad_wButtons, the360ControllerState.Gamepad.wButtons      & 0x0200, 0x00000020);
				SetState(Gamepad_wButtons, the360ControllerState.Gamepad.wButtons      & 0x0010, 0x00000100);
				SetState(Gamepad_wButtons, the360ControllerState.Gamepad.sThumbLY      >   7849, 0x00000200);
				SetState(Gamepad_wButtons, the360ControllerState.Gamepad.sThumbLY      <  -7849, 0x00000400);
				SetState(Gamepad_wButtons, the360ControllerState.Gamepad.sThumbLX      <  -7849, 0x00000800);
				SetState(Gamepad_wButtons, the360ControllerState.Gamepad.sThumbLX      >   7849, 0x00001000);
				SetState(Gamepad_wButtons, the360ControllerState.Gamepad.wButtons      & 0x0001, 0x00000200);
				SetState(Gamepad_wButtons, the360ControllerState.Gamepad.wButtons      & 0x0002, 0x00000400);
				SetState(Gamepad_wButtons, the360ControllerState.Gamepad.wButtons      & 0x0004, 0x00000800);
				SetState(Gamepad_wButtons, the360ControllerState.Gamepad.wButtons      & 0x0008, 0x00001000);

				SetState(Gamepad_bThumbRX, the360ControllerState.Gamepad.sThumbRX < -8689 || the360ControllerState.Gamepad.sThumbRX > 8689, ( the360ControllerState.Gamepad.sThumbRX / 65536.0 + 0.5) * 64.0, true);
				SetState(Gamepad_bThumbRY, the360ControllerState.Gamepad.sThumbRY < -8689 || the360ControllerState.Gamepad.sThumbRY > 8689, (-the360ControllerState.Gamepad.sThumbRY / 65536.0 + 0.5) * 64.0, true);
			}
		}

		// if the game isn't already key mapping for us...
		if (!keymapping) {
			// set currently pressed keys as down on NetJet Controller
			// Button 4
			SetState(Gamepad_wButtons, KB.keysdown[15], 0x00000001);
			SetState(Gamepad_wButtons, KB.keysdown[0] , 0x00000001);
			// Button 2
			SetState(Gamepad_wButtons, KB.keysdown[18], 0x00000002);
			SetState(Gamepad_wButtons, KB.keysdown[16], 0x00000002);
			SetState(Gamepad_wButtons, KB.keysdown[1] , 0x00000002);
			// Button 3
			SetState(Gamepad_wButtons, KB.keysdown[17], 0x00000004);
			SetState(Gamepad_wButtons, KB.keysdown[2] , 0x00000004);
			// Button 1
			SetState(Gamepad_wButtons, KB.keysdown[20], 0x00000008);
			SetState(Gamepad_wButtons, KB.keysdown[19], 0x00000008);
			SetState(Gamepad_wButtons, KB.keysdown[3] , 0x00000008);
			// Left Shoulder
			SetState(Gamepad_wButtons, KB.keysdown[21], 0x00000010);
			SetState(Gamepad_wButtons, KB.keysdown[4] , 0x00000010);
			// Right Shoulder
			SetState(Gamepad_wButtons, KB.keysdown[22], 0x00000020);
			SetState(Gamepad_wButtons, KB.keysdown[5] , 0x00000020);
			// Start
			SetState(Gamepad_wButtons, KB.keysdown[23], 0x00000100);
			SetState(Gamepad_wButtons, KB.keysdown[6] , 0x00000100);
			// DPad Up
			SetState(Gamepad_wButtons, KB.keysdown[7] , 0x00000200);
			// DPad Down
			SetState(Gamepad_wButtons, KB.keysdown[8] , 0x00000400);
			// DPad Left
			SetState(Gamepad_wButtons, KB.keysdown[9] , 0x00000800);
			// DPad Right
			SetState(Gamepad_wButtons, KB.keysdown[10], 0x00001000);
			// Right Thumbstick
			SetState(Gamepad_bThumbRY, KB.keysdown[11], 0x00000000, true);
			SetState(Gamepad_bThumbRX, KB.keysdown[12], 0x00000000, true);
			SetState(Gamepad_bThumbRY, KB.keysdown[13], 0x00000040, true);
			SetState(Gamepad_bThumbRX, KB.keysdown[14], 0x00000040, true);
		}

		// ignore mouse mapping because game conflicts with it
		/*
		if (!mousemapping) {
			if (GetCursorPos(&curpos)) {
				if (myWidth > 0 && (curpos.x - (myWidth / 2)) != 0) {
					*Gamepad_bThumbRX = 31 + ((curpos.x - (myWidth / 2)) / (double)myWidth * 31);
				}
				if (myHeight > 0 && (curpos.y - (myHeight / 2)) != 0) {
					*Gamepad_bThumbRY = 31 + ((curpos.y - (myHeight / 2)) / (double)myHeight * 31);
				}
			}
			SetCursorPos(myWidth / 2, myHeight / 2);
		}
		*/
	}

	// ensure thumbsticks are not further up/down/left/right than they can be on the controller
	FixThumbstick(Gamepad_bThumbRX, Gamepad_bThumbRY);
	// ensure the game thinks a controller is inserted and a cartridge is inserted and valid one more time just in case
	SetControllerInserted(Gamepad_wButtons, Gamepad_bThumbRX, Gamepad_bThumbRY, true, false, false);
	SetCartridgeInserted(Gamepad_wButtons);
}

void NetJetEmulator::callNetJetControllerGetKey(char *Gamepad_wButtons, bool result) {
	// call intercepted

	/*
	if (f.is_open()) {
		f << "NetJetControllerGetState " << *Gamepad_wButtons << " " << *Gamepad_bThumbRX << " " << *Gamepad_bThumbRY << "\n";
	}
	*/

	// zero key, this is not a keygen
	// replicating real keys is outside the scope of this project
	if (!result) {
		memset(Gamepad_wButtons, 0x00, 0x20);
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

		/*
		remove("console_NetJetController.log");
		f.open("console_NetJetController.log", std::ios::out | std::ios::app);
		*/

		KB.backgroundThreadHook = SetWindowsHookEx(WH_KEYBOARD_LL, KB.backgroundThread, NULL, 0);
		ShowCursor(false);
		break;
	case DLL_PROCESS_DETACH:
		FreeLibrary(originalNetJetController);

		/*
		if (f.is_open()) {
			f.close();
		}
		*/

		UnhookWindowsHookEx(KB.backgroundThreadHook);
		ShowCursor(true);
		break;
	}
	return true;
}

// duplicate functions which call the original and return false
// we always want these calls to appear successful
// oddly returning false means no error ocurred
typedef signed int(*NetJetControllerEnableKeyMapping_)();
extern "C" signed int callNetJetControllerEnableKeyMapping()
{
	NetJetControllerEnableKeyMapping_ originalNetJetControllerEnableKeyMapping;

	originalNetJetControllerEnableKeyMapping = (NetJetControllerEnableKeyMapping_)GetProcAddress(originalNetJetController, "NetJetControllerEnableKeyMapping");
	if (!originalNetJetControllerEnableKeyMapping) {
	} else {
		originalNetJetControllerEnableKeyMapping();
	}

	NJ.keymapping = true;
	UnhookWindowsHookEx(KB.backgroundThreadHook);

	return 0;
}

typedef signed int(*NetJetControllerDisableKeyMapping_)();
extern "C" signed int callNetJetControllerDisableKeyMapping()
{
	NetJetControllerDisableKeyMapping_ originalNetJetControllerDisableKeyMapping;

	originalNetJetControllerDisableKeyMapping = (NetJetControllerDisableKeyMapping_)GetProcAddress(originalNetJetController, "NetJetControllerDisableKeyMapping");
	if (!originalNetJetControllerDisableKeyMapping) {
	} else {
		originalNetJetControllerDisableKeyMapping();
	}

	NJ.keymapping = false;
	KB.backgroundThreadHook = SetWindowsHookEx(WH_KEYBOARD_LL, KB.backgroundThread, NULL, 0);

	return 0;
}

typedef signed int(*NetJetControllerEnableMouseMapping_)();
extern "C" signed int callNetJetControllerEnableMouseMapping()
{
	NetJetControllerEnableMouseMapping_ originalNetJetControllerEnableMouseMapping;

	originalNetJetControllerEnableMouseMapping = (NetJetControllerEnableMouseMapping_)GetProcAddress(originalNetJetController, "NetJetControllerEnableMouseMapping");
	if (!originalNetJetControllerEnableMouseMapping) {
	} else {
		originalNetJetControllerEnableMouseMapping();
	}

	NJ.mousemapping = true;
	ShowCursor(true);

	return 0;
}

typedef signed int(*NetJetControllerDisableMouseMapping_)();
extern "C" signed int callNetJetControllerDisableMouseMapping()
{
	NetJetControllerDisableMouseMapping_ originalNetJetControllerDisableMouseMapping;

	originalNetJetControllerDisableMouseMapping = (NetJetControllerDisableMouseMapping_)GetProcAddress(originalNetJetController, "NetJetControllerDisableMouseMapping");
	if (!originalNetJetControllerDisableMouseMapping) {
	} else {
		originalNetJetControllerDisableMouseMapping();
	}

	NJ.mousemapping = false;
	ShowCursor(false);

	return 0;
}

typedef signed int(*NetJetControllerInitialize_)();
extern "C" signed int callNetJetControllerInitialize()
{
	NetJetControllerInitialize_ originalNetJetControllerInitialize;

	originalNetJetControllerInitialize = (NetJetControllerInitialize_)GetProcAddress(originalNetJetController, "NetJetControllerInitialize");
	if (!originalNetJetControllerInitialize) {
	} else {
		originalNetJetControllerInitialize();
	}

	original360Controller = LoadLibraryA("XINPUT9_1_0.DLL");

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
	NJ.suspended = true;

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
	NJ.suspended = false;

	return false;
}

typedef signed int(*NetJetControllerShutdown_)();
extern "C" signed int callNetJetControllerShutdown()
{
	NetJetControllerShutdown_ originalNetJetControllerShutdown;

	originalNetJetControllerShutdown = (NetJetControllerShutdown_)GetProcAddress(originalNetJetController, "NetJetControllerShutdown");
	if (!originalNetJetControllerShutdown) {
	} else {
		originalNetJetControllerShutdown();
	}

	return 0;
}

typedef signed int(*NetJetControllerSetKeyMapping_)(INT16);
extern "C" signed int callNetJetControllerSetKeyMapping(INT16 Gamepad_wButtons)
{
	NetJetControllerSetKeyMapping_ originalNetJetControllerSetKeyMapping;

	originalNetJetControllerSetKeyMapping = (NetJetControllerSetKeyMapping_)GetProcAddress(originalNetJetController, "NetJetControllerSetKeyMapping");
	if (!originalNetJetControllerSetKeyMapping) {
	} else {
		originalNetJetControllerSetKeyMapping(Gamepad_wButtons);
	}

	return 0;
}

typedef BOOL(__cdecl *NetJetControllerSetOption_)(int, int);
extern "C" BOOL __cdecl callNetJetControllerSetOption(int Gamepad_wButtons, int nPriority)
{
	NetJetControllerSetOption_ originalNetJetControllerSetOption;

	originalNetJetControllerSetOption = (NetJetControllerSetOption_)GetProcAddress(originalNetJetController, "NetJetControllerSetOption");
	if (!originalNetJetControllerSetOption) {
	} else {
		originalNetJetControllerSetOption(Gamepad_wButtons, nPriority);
	}

	return false;
}

typedef BOOL(__cdecl *NetJetControllerGetState_)(int, int, int);
extern "C" BOOL __cdecl callNetJetControllerGetState(int Gamepad_wButtons, int Gamepad_bThumbRX, int Gamepad_bThumbRY)
{
	BOOL result = false;
	NetJetControllerGetState_ originalNetJetControllerGetState;

	originalNetJetControllerGetState = (NetJetControllerGetState_)GetProcAddress(originalNetJetController, "NetJetControllerGetState");
	if (!originalNetJetControllerGetState) {
	}
	else {
		result = !originalNetJetControllerGetState(Gamepad_wButtons, Gamepad_bThumbRX, Gamepad_bThumbRY);
	}

	NJ.callNetJetControllerGetState((int*)Gamepad_wButtons, (int*)Gamepad_bThumbRX, (int*)Gamepad_bThumbRY, result);

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
extern "C" BOOL __cdecl callNetJetControllerGetControllerKey(int Gamepad_wButtons)
{
	BOOL result = false;
	NetJetControllerGetControllerKey_ originalNetJetControllerGetControllerKey;

	originalNetJetControllerGetControllerKey = (NetJetControllerGetControllerKey_)GetProcAddress(originalNetJetController, "NetJetControllerGetControllerKey");
	if (!originalNetJetControllerGetControllerKey) {
	} else {
		result = !originalNetJetControllerGetControllerKey(Gamepad_wButtons);
	}

	NJ.callNetJetControllerGetKey((char *)Gamepad_wButtons, result);

	return false;
}

typedef BOOL(__cdecl *NetJetControlleretCartrdigeKey_)(int);
extern "C" BOOL __cdecl callNetJetControlleretCartrdigeKey(int Gamepad_wButtons)
{
	BOOL result = false;
	NetJetControlleretCartrdigeKey_ originalNetJetControlleretCartrdigeKey;

	originalNetJetControlleretCartrdigeKey = (NetJetControlleretCartrdigeKey_)GetProcAddress(originalNetJetController, "NetJetControlleretCartrdigeKey");
	if (!originalNetJetControlleretCartrdigeKey) {
	} else {
		result = !originalNetJetControlleretCartrdigeKey(Gamepad_wButtons);
	}

	NJ.callNetJetControllerGetKey((char *)Gamepad_wButtons, result);

	return false;
}

typedef signed int(*NetJetControllerRun_)();
extern "C" signed int callNetJetControllerRun()
{
	NetJetControllerRun_ originalNetJetControllerRun;

	originalNetJetControllerRun = (NetJetControllerRun_)GetProcAddress(originalNetJetController, "NetJetControllerRun");
	if (!originalNetJetControllerRun) {
	} else {
		originalNetJetControllerRun();
	}

	return 0;
}