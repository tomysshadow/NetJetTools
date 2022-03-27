#include "shared.h"
#include "NetJetSimulatorDirectInput8.h"
#include "NetJetSimulatorDirectInputDevice8.h"
#include <windows.h>
#include <dinput.h>
#include <string>

typedef HRESULT (WINAPI* DirectInputCreate8Proc)(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID * ppvOut, LPUNKNOWN punkOuter);

DirectInputCreate8Proc originalDirectInput8Create = NULL;

BOOL APIENTRY DllMain(HMODULE instance, DWORD reason, LPVOID reserved) {
	if (reason == DLL_PROCESS_ATTACH) {
		return DisableThreadLibraryCalls(instance);
	}
    return TRUE;
}

HRESULT WINAPI DirectInput8Create(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, LPVOID * ppvOut, LPUNKNOWN punkOuter) {
	if (!originalDirectInput8Create) {
		CHAR systemDirectory[MAX_PATH] = "";

		if (!GetSystemDirectory(systemDirectory, MAX_PATH - 1)) {
			return DIERR_OUTOFMEMORY;
		}

		HMODULE originalDINPUT8ModuleHandle = LoadLibrary((std::string((char*)systemDirectory) + "\\DINPUT8.DLL").c_str());

		if (!originalDINPUT8ModuleHandle) {
			return DIERR_OUTOFMEMORY;
		}

		originalDirectInput8Create = (DirectInputCreate8Proc)GetProcAddress(originalDINPUT8ModuleHandle, "DirectInput8Create");

		if (!originalDirectInput8Create) {
			return DIERR_OUTOFMEMORY;
		}
	}
	
	HRESULT err = originalDirectInput8Create(hinst, dwVersion, riidltf, ppvOut, punkOuter);

	if (err != DI_OK) {
		return err;
	}

	if (!ppvOut || !*ppvOut) {
		return DIERR_INVALIDPARAM;
	}

	extern GetMultiplayerStateProc getMultiplayerState;

	if (!getMultiplayerState) {
		// we do this here so we don't call LoadLibrary for every device created
		// (or worse, every device acquisition)
		HMODULE netJetSimulatorModuleHandle = LoadLibrary("NetJetController.DLL");

		if (!netJetSimulatorModuleHandle) {
			netJetSimulatorModuleHandle = LoadLibrary("KonamiLiveController.DLL");
		}

		if (netJetSimulatorModuleHandle) {
			getMultiplayerState = (GetMultiplayerStateProc)GetProcAddress(netJetSimulatorModuleHandle, "getMultiplayerState");
		}
	}

	NetJetSimulatorDirectInput8* netJetSimulatorDirectInput8Pointer = new NetJetSimulatorDirectInput8(*(IDirectInput8**)ppvOut);

	if (!netJetSimulatorDirectInput8Pointer) {
		(*(IDirectInput8**)ppvOut)->Release();
		*ppvOut = NULL;
		return DIERR_OUTOFMEMORY;
	}
	
	// the object will delete itself once Ref count is zero (similar to COM objects)
	*ppvOut = netJetSimulatorDirectInput8Pointer;
	return DI_OK;
}