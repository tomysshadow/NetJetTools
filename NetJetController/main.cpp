#include "shared.h"
#include "NetJetController.h"
#include <windows.h>

static const BOOL MODULE_TYPE_DEFAULT = FALSE;

static NetJetSimulator* netJetSimulatorPointer = 0;
NetJetSimulator::GetMultiplayerState netJetSimulatorGetMultiplayerState = 0;

BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved) {
	if (reason == DLL_PROCESS_ATTACH) {
		return DisableThreadLibraryCalls(instance);
	}

	netJetSimulatorGetMultiplayerState = 0;

	if (netJetSimulatorPointer) {
		delete netJetSimulatorPointer;
		netJetSimulatorPointer = 0;
	}
	return TRUE;
}

extern "C" BOOL EnableKeyMapping() {
	if (netJetSimulatorPointer) {
		try {
			return netJetSimulatorPointer->enableKeyMapping();
		} catch (BOOL moduleType) {
			return !moduleType;
		}
	}
	return MODULE_TYPE_DEFAULT;
}

extern "C" BOOL DisableKeyMapping() {
	if (netJetSimulatorPointer) {
		return netJetSimulatorPointer->disableKeyMapping();
	}
	return MODULE_TYPE_DEFAULT;
}

extern "C" BOOL EnableMouseMapping() {
	if (netJetSimulatorPointer) {
		return netJetSimulatorPointer->enableMouseMapping();
	}
	return MODULE_TYPE_DEFAULT;
}

extern "C" BOOL DisableMouseMapping() {
	if (netJetSimulatorPointer) {
		return netJetSimulatorPointer->disableMouseMapping();
	}
	return MODULE_TYPE_DEFAULT;
}

BOOL Initialize(bool konamiLive = false) {
	BOOL moduleType = MODULE_TYPE_DEFAULT;

	bool netJetSimulatorFileSet = false;
	HANDLE netJetSimulatorFile = CreateFile("NetJetSimulator.BIN", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (netJetSimulatorFile && netJetSimulatorFile != INVALID_HANDLE_VALUE) {
		netJetSimulatorFileSet = true;
	}

	if (!netJetSimulatorPointer) {
		netJetSimulatorPointer = new NetJetSimulator(konamiLive, netJetSimulatorGetMultiplayerState, netJetSimulatorFileSet, netJetSimulatorFile);
	}

	bool initialized = false;

	if (netJetSimulatorPointer) {
		try {
			moduleType = netJetSimulatorPointer->initialize();
			initialized = true;
		} catch (BOOL ex) {
			moduleType = ex;
		}
	}

	if (netJetSimulatorFile && netJetSimulatorFile != INVALID_HANDLE_VALUE) {
		if (!CloseHandle(netJetSimulatorFile)) {
			return !moduleType;
		}

		netJetSimulatorFile = NULL;
	}
	return initialized ? moduleType : !moduleType;
}

extern "C" BOOL NetJetControllerInitialize() {
	return Initialize(false);
}

extern "C" BOOL KonamiLiveControllerInitialize() {
	return Initialize(true);
}

extern "C" BOOL Suspend() {
	if (netJetSimulatorPointer) {
		return netJetSimulatorPointer->suspend();
	}
	return MODULE_TYPE_DEFAULT;
}

extern "C" BOOL Resume() {
	if (netJetSimulatorPointer) {
		return netJetSimulatorPointer->resume();
	}
	return MODULE_TYPE_DEFAULT;
}

extern "C" BOOL Shutdown() {
	BOOL moduleType = MODULE_TYPE_DEFAULT;

	bool shutDown = false;

	if (netJetSimulatorPointer) {
		try {
			moduleType = netJetSimulatorPointer->shutdown();
			shutDown = true;
		} catch (BOOL ex) {
			moduleType = ex;
		}

		delete netJetSimulatorPointer;
		netJetSimulatorPointer = 0;
	}
	return shutDown ? moduleType : !moduleType;
}

extern "C" BOOL SetKeyMapping(PWORD keys) {
	if (netJetSimulatorPointer) {
		try {
			return netJetSimulatorPointer->setKeyMapping(keys);
		} catch (BOOL moduleType) {
			return !moduleType;
		}
	}
	return MODULE_TYPE_DEFAULT;
}

extern "C" BOOL SetOption(WORD option, WORD value) {
	if (netJetSimulatorPointer) {
		return netJetSimulatorPointer->setOption(option, value);
	}
	return MODULE_TYPE_DEFAULT;
}

extern "C" BOOL NetJetControllerGetState(CONTROL* buttonsControlPointer, CONTROL* thumbRXControlPointer, CONTROL* thumbRYControlPointer) {
	if (netJetSimulatorPointer) {
		return netJetSimulatorPointer->getState(buttonsControlPointer, thumbRXControlPointer, thumbRYControlPointer);
	}
	return MODULE_TYPE_DEFAULT;
}

extern "C" BOOL KonamiLiveControllerGetState(CONTROL* buttonsControlPointer) {
	if (netJetSimulatorPointer) {
		return netJetSimulatorPointer->getState(buttonsControlPointer, NULL, NULL);
	}
	return MODULE_TYPE_DEFAULT;
}

extern "C" BOOL SetWindow(HWND windowHandle) {
	if (netJetSimulatorPointer) {
		return netJetSimulatorPointer->setWindow(windowHandle);
	}
	return MODULE_TYPE_DEFAULT;
}

extern "C" BOOL GetControllerKey(LPVOID controllerKeyPointer) {
	if (netJetSimulatorPointer) {
		return netJetSimulatorPointer->getControllerKey(controllerKeyPointer);
	}
	return MODULE_TYPE_DEFAULT;
}

extern "C" BOOL GetCartridgeKey(LPVOID cartridgeKeyPointer) {
	if (netJetSimulatorPointer) {
		return netJetSimulatorPointer->getCartridgeKey(cartridgeKeyPointer);
	}
	return MODULE_TYPE_DEFAULT;
}

extern "C" BOOL Run() {
	if (netJetSimulatorPointer) {
		try {
			return netJetSimulatorPointer->run();
		} catch (BOOL moduleType) {
			return !moduleType;
		}
	}
	return MODULE_TYPE_DEFAULT;
}

extern "C" bool getMultiplayerState() {
	if (netJetSimulatorPointer && netJetSimulatorGetMultiplayerState) {
		return (netJetSimulatorPointer->*netJetSimulatorGetMultiplayerState)();
	}
	return false;
}