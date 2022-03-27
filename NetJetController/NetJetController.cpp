#include "NetJetController.h"
#include <windows.h>
#include <xinput.h>

#define XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE  7849
#define XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE 8689
#define XINPUT_GAMEPAD_TRIGGER_THRESHOLD    30

static HHOOK backgroundThreadHook = NULL;
static KEYBOARD_POINTER_SET keyboardPointerSet = {};

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
	// we want to return as soon as possible if this is empty
	// (don't want to spend too long holding up key events!)
	if (!keyboardPointerSet.empty()) {
		if (lParam && nCode == HC_ACTION) {
			bool down = (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN);
			bool up = (wParam == WM_KEYUP || wParam == WM_SYSKEYUP);

			if (down || up) {
				NetJetSimulator::Keyboard* keyboardPointer = 0;

				DWORD vkCode = ((LPKBDLLHOOKSTRUCT)lParam)->vkCode;

				bool vkCodeFound = false;
				bool multiplayerState = false;

				for (KEYBOARD_POINTER_SET::iterator keyboardPointerSetIterator = keyboardPointerSet.begin(); keyboardPointerSetIterator != keyboardPointerSet.end(); keyboardPointerSetIterator++) {
					keyboardPointer = *keyboardPointerSetIterator;

					if (keyboardPointer) {
						NetJetSimulator::Keyboard::VK_CODE_MAPPING_MAP &buttonsControlVKCodeMappingMap = keyboardPointer->buttonsControlVKCodeMappingMap;
						NetJetSimulator::Keyboard::VK_CODE_MAPPING_MAP &thumbRXControlVKCodeMappingMap = keyboardPointer->thumbRXControlVKCodeMappingMap;
						NetJetSimulator::Keyboard::VK_CODE_MAPPING_MAP &thumbRYControlVKCodeMappingMap = keyboardPointer->thumbRYControlVKCodeMappingMap;

						if (buttonsControlVKCodeMappingMap.find(vkCode) != buttonsControlVKCodeMappingMap.end()
							|| thumbRXControlVKCodeMappingMap.find(vkCode) != thumbRXControlVKCodeMappingMap.end()
							|| thumbRYControlVKCodeMappingMap.find(vkCode) != thumbRYControlVKCodeMappingMap.end()) {
							NetJetSimulator::Keyboard::VK_CODE_SET &vkCodeSet = keyboardPointer->vkCodeSet;

							if (down) {
								vkCodeSet.insert(vkCode);
							} else {
								vkCodeSet.erase(vkCode);
							}

							vkCodeFound = true;
						}

						// this needs to happen last
						// so we still register key up events
						if (!multiplayerState) {
							if (keyboardPointer->netJetSimulatorGetMultiplayerState) {
								multiplayerState = (keyboardPointer->netJetSimulator.*(keyboardPointer->netJetSimulatorGetMultiplayerState))();
							}
						}
					}
				}

				// if the VK Code was found and we're not in multiplayer state, do not call the next hook
				// we do not call the next hook if the VK Code was found because the state would be sent twice (from the NetJet Simulator and the real Keyboard)
				// we do call the next hook if we're in multiplayer state because it expects real Keyboard events
				if (vkCodeFound && !multiplayerState) {
					// need to return a non-zero number
					return 1;
				}
			}
		}
	}
	return CallNextHookEx(backgroundThreadHook, nCode, wParam, lParam);
}

NetJetSimulator::Xbox360Controller::Xbox360Controller(NetJetSimulator &netJetSimulator, Xbox360Controller::GetMultiplayerState &getMultiplayerState, bool &netJetSimulatorFileSet, HANDLE netJetSimulatorFile) : netJetSimulator(netJetSimulator) {
	getMultiplayerState = &Xbox360Controller::getMultiplayerState;

	if (netJetSimulatorFileSet) {
		netJetSimulatorFileSet = (netJetSimulatorFile && netJetSimulatorFile != INVALID_HANDLE_VALUE);
	}

	if (netJetSimulatorFileSet) {
		const size_t MAPPINGS_SIZE = sizeof(mappings);

		netJetSimulatorFileSet = readFileSafe(netJetSimulatorFile, &mappings, MAPPINGS_SIZE);
	}
}

NetJetSimulator::Xbox360Controller::~Xbox360Controller() {
	destroy();
}

void NetJetSimulator::Xbox360Controller::initialize() {
	// we just need some version of XInput that has XInputGetState
	// legacy XInput is what most people are most likely to have
	HMODULE xinputModuleHandle = LoadLibrary("XINPUT9_1_0.DLL");

	// otherwise we start with the newest version and go down the list to the oldest
	if (!xinputModuleHandle) {
		xinputModuleHandle = LoadLibrary("XINPUT1_4.DLL");
	}

	if (!xinputModuleHandle) {
		xinputModuleHandle = LoadLibrary("XINPUT1_3.DLL");
	}

	if (!xinputModuleHandle) {
		xinputModuleHandle = LoadLibrary("XINPUT1_2.DLL");
	}

	if (!xinputModuleHandle) {
		xinputModuleHandle = LoadLibrary("XINPUT1_1.DLL");
	}

	if (xinputModuleHandle) {
		xinputGetState = (XInputGetStateProc)GetProcAddress(xinputModuleHandle, "XInputGetState");
	}
}

void NetJetSimulator::Xbox360Controller::shutdown() {
}

void NetJetSimulator::Xbox360Controller::getState(CONTROL* buttonsControlPointer, CONTROL* thumbRXControlPointer, CONTROL* thumbRYControlPointer, bool &controllerInserted) {
	XINPUT_STATE state = {};
	getStateThrottled(state, controllerInserted);

	if (!controllerInserted) {
		return;
	}

	if (buttonsControlPointer) {
		CONTROL &buttonsControl = *buttonsControlPointer;

		setButtonsControlMapping(buttonsControl, mappings.y, state.Gamepad.wButtons       & XINPUT_GAMEPAD_Y);
		setButtonsControlMapping(buttonsControl, mappings.b, state.Gamepad.wButtons       & XINPUT_GAMEPAD_B);
		setButtonsControlMapping(buttonsControl, mappings.x, state.Gamepad.wButtons       & XINPUT_GAMEPAD_X);
		setButtonsControlMapping(buttonsControl, mappings.a, state.Gamepad.wButtons       & XINPUT_GAMEPAD_A);
		setButtonsControlMapping(buttonsControl, mappings.lb, state.Gamepad.wButtons      & XINPUT_GAMEPAD_LEFT_SHOULDER);
		setButtonsControlMapping(buttonsControl, mappings.rb, state.Gamepad.wButtons      & XINPUT_GAMEPAD_RIGHT_SHOULDER);
		setButtonsControlMapping(buttonsControl, mappings.lt, state.Gamepad.bLeftTrigger  > XINPUT_GAMEPAD_TRIGGER_THRESHOLD);
		setButtonsControlMapping(buttonsControl, mappings.rt, state.Gamepad.bRightTrigger > XINPUT_GAMEPAD_TRIGGER_THRESHOLD);
		setButtonsControlMapping(buttonsControl, mappings.lsb, state.Gamepad.wButtons     & XINPUT_GAMEPAD_LEFT_THUMB);
		setButtonsControlMapping(buttonsControl, mappings.rsb, state.Gamepad.wButtons     & XINPUT_GAMEPAD_RIGHT_THUMB);
		setButtonsControlMapping(buttonsControl, mappings.start, state.Gamepad.wButtons   & XINPUT_GAMEPAD_START);
		setButtonsControlMapping(buttonsControl, mappings.back, state.Gamepad.wButtons    & XINPUT_GAMEPAD_BACK);
		setButtonsControlMapping(buttonsControl, mappings.dU, state.Gamepad.wButtons      & XINPUT_GAMEPAD_DPAD_UP);
		setButtonsControlMapping(buttonsControl, mappings.dD, state.Gamepad.wButtons      & XINPUT_GAMEPAD_DPAD_DOWN);
		setButtonsControlMapping(buttonsControl, mappings.dL, state.Gamepad.wButtons      & XINPUT_GAMEPAD_DPAD_LEFT);
		setButtonsControlMapping(buttonsControl, mappings.dR, state.Gamepad.wButtons      & XINPUT_GAMEPAD_DPAD_RIGHT);
		setButtonsControlMapping(buttonsControl, mappings.lsU, state.Gamepad.sThumbLY     > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
		setButtonsControlMapping(buttonsControl, mappings.lsD, state.Gamepad.sThumbLY     < ~XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
		setButtonsControlMapping(buttonsControl, mappings.lsL, state.Gamepad.sThumbLX     < ~XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
		setButtonsControlMapping(buttonsControl, mappings.lsR, state.Gamepad.sThumbLX     > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
	}

	if (mappings.rs) {
		double rightThumbRange = (double)mappings.rs / (SHRT_MAX - SHRT_MIN);

		if (thumbRXControlPointer) {
			setThumbControlMapping(*thumbRXControlPointer,
				(MAPPING)(rightThumbRange * (state.Gamepad.sThumbRX - SHRT_MIN)),
				state.Gamepad.sThumbRX    < ~XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE
				|| state.Gamepad.sThumbRX > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
		}

		if (thumbRYControlPointer) {
			setThumbControlMapping(*thumbRYControlPointer,
				(MAPPING)(rightThumbRange * (~state.Gamepad.sThumbRY - SHRT_MIN)),
				state.Gamepad.sThumbRY    < ~XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE
				|| state.Gamepad.sThumbRY > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
		}
	}
}

void NetJetSimulator::Xbox360Controller::destroy() {
	xinputGetState = NULL;
}

void NetJetSimulator::Xbox360Controller::getStateThrottled(XINPUT_STATE &state, bool &controllerInserted) {
	if (!xinputGetState) {
		controllerInserted = false;
		return;
	}

	// test whether a second has passed
	// but only if the throttled tick count is set
	if (throttledTickCountSet) {
		const DWORD SECOND = 1000;

		// we use GetTickCount because
		// the GetTickCount64 function is only available from Vista onwards
		// (and the NetJet is Windows XP compatible)
		// obviously this needs to be done before calling xinputGetState
		// so this value isn't useful after that call (because it will consume ticks)
		// the following method of comparison avoids overflow errors
		// be careful if modifying it
		if (GetTickCount() - throttledTickCount < SECOND) {
			controllerInserted = false;
			return;
		}
	}

	// it's important to throttle this because
	// it can introduce lag if the controller is not inserted
	// because of device enumeration
	// our object doesn't have a seperate controllerInserted property
	// because that could cause issues with multiple threads
	// instead it's taken as an argument to this function
	controllerInserted = (xinputGetState(USER_INDEX, &state) == ERROR_SUCCESS);

	// set the throttled tick count
	// but only if the controller is not inserted
	throttledTickCountSet = !controllerInserted;

	if (throttledTickCountSet) {
		// xinputGetState may have made us wait some ticks
		// so we call GetTickCount again here
		// this is fine because it's pretty fast
		// and will only occur at most once a second
		throttledTickCount = GetTickCount();
	}
}

bool NetJetSimulator::Xbox360Controller::getMultiplayerState() {
	XINPUT_STATE state = {};
	bool controllerInserted = false;

	getStateThrottled(state, controllerInserted);
	return controllerInserted;
}

NetJetSimulator::Keyboard::Keyboard(NetJetSimulator &netJetSimulator, NetJetSimulator::GetMultiplayerState netJetSimulatorGetMultiplayerState, bool &netJetSimulatorFileSet, HANDLE netJetSimulatorFile) : netJetSimulator(netJetSimulator), netJetSimulatorGetMultiplayerState(netJetSimulatorGetMultiplayerState) {
	keyboardPointerSet.insert(this);

	if (netJetSimulatorFileSet) {
		netJetSimulatorFileSet = (netJetSimulatorFile && netJetSimulatorFile != INVALID_HANDLE_VALUE);
	}

	if (netJetSimulatorFileSet) {
		netJetSimulatorFileSet = readVKCodeMappingMap(netJetSimulatorFile, buttonsControlVKCodeMappingMap);
	}

	if (netJetSimulatorFileSet) {
		netJetSimulatorFileSet = readVKCodeMappingMap(netJetSimulatorFile, thumbRXControlVKCodeMappingMap);
	}

	if (netJetSimulatorFileSet) {
		netJetSimulatorFileSet = readVKCodeMappingMap(netJetSimulatorFile, thumbRYControlVKCodeMappingMap);
	}
}

NetJetSimulator::Keyboard::~Keyboard() {
	destroy();
}

void NetJetSimulator::Keyboard::initialize() {
}

void NetJetSimulator::Keyboard::shutdown() {
}

void NetJetSimulator::Keyboard::getState(CONTROL* buttonsControlPointer, CONTROL* thumbRXControlPointer, CONTROL* thumbRYControlPointer) {
	VK_CODE_MAPPING_MAP::iterator vkCodeMappingMapIterator = {};

	if (buttonsControlPointer) {
		for (vkCodeMappingMapIterator = buttonsControlVKCodeMappingMap.begin(); vkCodeMappingMapIterator != buttonsControlVKCodeMappingMap.end(); vkCodeMappingMapIterator++) {
			setButtonsControlMapping(*buttonsControlPointer, vkCodeMappingMapIterator->second, vkCodeSet.find(vkCodeMappingMapIterator->first) != vkCodeSet.end());
		}
	}

	// disallow multiple thumb directions
	// (we don't want to average them in case multiple keys are mapped to the thumb)
	MAPPING thumbMapping = NetJetSimulator::THUMB_CENTER_MAPPING;
	bool state = false;

	if (thumbRXControlPointer) {
		for (vkCodeMappingMapIterator = thumbRXControlVKCodeMappingMap.begin(); vkCodeMappingMapIterator != thumbRXControlVKCodeMappingMap.end(); vkCodeMappingMapIterator++) {
			if (vkCodeSet.find(vkCodeMappingMapIterator->first) != vkCodeSet.end()) {
				if (state) {
					if (thumbMapping == vkCodeMappingMapIterator->second) {
						continue;
					}

					state = false;
					break;
				}

				thumbMapping = vkCodeMappingMapIterator->second;
				state = true;
			}
		}

		setThumbControlMapping(*thumbRXControlPointer, thumbMapping, state);
	}

	if (thumbRYControlPointer) {
		state = false;

		for (vkCodeMappingMapIterator = thumbRYControlVKCodeMappingMap.begin(); vkCodeMappingMapIterator != thumbRYControlVKCodeMappingMap.end(); vkCodeMappingMapIterator++) {
			if (vkCodeSet.find(vkCodeMappingMapIterator->first) != vkCodeSet.end()) {
				if (state) {
					if (thumbMapping == vkCodeMappingMapIterator->second) {
						continue;
					}

					state = false;
					break;
				}

				thumbMapping = vkCodeMappingMapIterator->second;
				state = true;
			}
		}

		setThumbControlMapping(*thumbRYControlPointer, thumbMapping, state);
	}
}

void NetJetSimulator::Keyboard::destroy() {
	netJetSimulatorGetMultiplayerState = 0;

	keyboardPointerSet.erase(this);
}

bool NetJetSimulator::Keyboard::readVKCodeMappingMap(HANDLE file, VK_CODE_MAPPING_MAP &vkCodeControlMappingMap) {
	DWORD vkCode = 0x00;
	MAPPING mapping = 0x00000000;

	// the distance from begin to end
	// as calculated by std::distance(vkCodeMappingMap.begin(), vkCodeMappingMap.end())
	VK_CODE_MAPPING_MAP::difference_type distance = 0;

	const size_t VKCODE_SIZE = sizeof(vkCode);
	const size_t MAPPING_SIZE = sizeof(mapping);
	const size_t DISTANCE_SIZE = sizeof(distance);

	if (!readFileSafe(file, &distance, DISTANCE_SIZE)) {
		return false;
	}

	vkCodeControlMappingMap = {};

	for (VK_CODE_MAPPING_MAP::difference_type i = 0; i < distance; i++) {
		if (!readFileSafe(file, &vkCode, VKCODE_SIZE)) {
			return false;
		}

		if (!readFileSafe(file, &mapping, MAPPING_SIZE)) {
			return false;
		}

		vkCodeControlMappingMap[vkCode] = mapping;
	};
	return true;
}

NetJetSimulator::NetJetSimulator(bool konamiLive, NetJetSimulator::GetMultiplayerState &getMultiplayerState, bool &netJetSimulatorFileSet, HANDLE netJetSimulatorFile) {
	getMultiplayerState = &NetJetSimulator::getMultiplayerState;

	if (netJetSimulatorFileSet) {
		netJetSimulatorFileSet = (netJetSimulatorFile && netJetSimulatorFile != INVALID_HANDLE_VALUE);
	}

	{
		const size_t LONG_CHUNK_ID_SIZE = sizeof(LONG_CHUNK_ID);

		LONG_CHUNK_ID longChunkID = 0;

		if (netJetSimulatorFileSet) {
			netJetSimulatorFileSet = readFileSafe(netJetSimulatorFile, &longChunkID, LONG_CHUNK_ID_SIZE);
		}

		const LONG_CHUNK_ID NJ3TSBIN_CHUNK_ID = 0x4E49425354334A4E;

		if (netJetSimulatorFileSet) {
			netJetSimulatorFileSet = (longChunkID == NJ3TSBIN_CHUNK_ID);
		}
	}

	if (xbox360ControllerPointer) {
		delete xbox360ControllerPointer;
	}

	xbox360ControllerPointer = new Xbox360Controller(*this, this->xbox360ControllerGetMultiplayerState, netJetSimulatorFileSet, netJetSimulatorFile);

	if (keyboardPointer) {
		delete keyboardPointer;
	}

	keyboardPointer = new Keyboard(*this, &NetJetSimulator::getMultiplayerState, netJetSimulatorFileSet, netJetSimulatorFile);

	{
		const size_t MAPPING_SIZE = sizeof(MAPPING);

		if (netJetSimulatorFileSet) {
			netJetSimulatorFileSet = readFileSafe(netJetSimulatorFile, &controllerInsertedMapping, MAPPING_SIZE);
		}

		if (netJetSimulatorFileSet) {
			netJetSimulatorFileSet = readFileSafe(netJetSimulatorFile, &cartridgeInsertedMapping, MAPPING_SIZE);
		}
	}

	if (netJetSimulatorFileSet) {
		const size_t ALLOW_MULTIPLAYER_STATE_SIZE = sizeof(allowMultiplayerState);

		netJetSimulatorFileSet = readFileSafe(netJetSimulatorFile, &allowMultiplayerState, ALLOW_MULTIPLAYER_STATE_SIZE);
	}

	if (netJetSimulatorFileSet) {
		const size_t MODULE_TYPE_SIZE = sizeof(moduleType);

		netJetSimulatorFileSet = readFileSafe(netJetSimulatorFile, &moduleType, MODULE_TYPE_SIZE);
	}

	if (moduleType) {
		HMODULE originalModuleHandle = GetModuleHandle(NULL);

		if (originalModuleHandle) {
			originalEnableKeyMapping = NULL;
			originalDisableKeyMapping = (DisableKeyMappingProc)GetProcAddress(originalModuleHandle, "Controller_DisableKeyMapping");
			originalEnableMouseMapping = (EnableMouseMappingProc)GetProcAddress(originalModuleHandle, "Controller_EnableMouseMapping");
			originalDisableMouseMapping = (DisableMouseMappingProc)GetProcAddress(originalModuleHandle, "Controller_DisableMouseMapping");
			originalInitialize = (InitializeProc)GetProcAddress(originalModuleHandle, "Controller_Initialize");
			originalSuspend = (SuspendProc)GetProcAddress(originalModuleHandle, "Controller_Suspend");
			originalResume = (ResumeProc)GetProcAddress(originalModuleHandle, "Controller_Resume");
			originalShutdown = (ShutdownProc)GetProcAddress(originalModuleHandle, "Controller_Shutdown");
			originalSetKeyMapping = (SetKeyMappingProc)GetProcAddress(originalModuleHandle, "Controller_SetKeyMapping");
			originalSetOption = (SetOptionProc)GetProcAddress(originalModuleHandle, "Controller_SetOption");

			if (konamiLive) {
				originalGetState = (GetStateProc)GetProcAddress(originalModuleHandle, "Controller_GetState");
				originalGetState2 = NULL;
			} else {
				originalGetState = NULL;
				originalGetState2 = (GetState2Proc)GetProcAddress(originalModuleHandle, "Controller_GetState");
			}

			originalSetWindow = (SetWindowProc)GetProcAddress(originalModuleHandle, "Controller_SetWindow");
			originalGetControllerKey = (GetControllerKeyProc)GetProcAddress(originalModuleHandle, "Controller_GetControllerKey");
			originalGetCartridgeKey = (GetCartridgeKeyProc)GetProcAddress(originalModuleHandle, "Controller_GetCartridgeKey");
			originalRun = NULL;
		}
	} else {
		if (konamiLive) {
			HMODULE originalKonamiLiveControllerModuleHandle = LoadLibrary("KonamiLiveController_orig.DLL");

			if (originalKonamiLiveControllerModuleHandle) {
				originalEnableKeyMapping = (EnableKeyMappingProc)GetProcAddress(originalKonamiLiveControllerModuleHandle, "KonamiLiveControllerEnableKeyMapping");
				originalDisableKeyMapping = (DisableKeyMappingProc)GetProcAddress(originalKonamiLiveControllerModuleHandle, "KonamiLiveControllerDisableKeyMapping");
				originalEnableMouseMapping = (EnableMouseMappingProc)GetProcAddress(originalKonamiLiveControllerModuleHandle, "KonamiLiveControllerEnableMouseMapping");
				originalDisableMouseMapping = (DisableMouseMappingProc)GetProcAddress(originalKonamiLiveControllerModuleHandle, "KonamiLiveControllerDisableMouseMapping");
				originalInitialize = (InitializeProc)GetProcAddress(originalKonamiLiveControllerModuleHandle, "KonamiLiveControllerInitialize");
				originalSuspend = (SuspendProc)GetProcAddress(originalKonamiLiveControllerModuleHandle, "KonamiLiveControllerSuspend");
				originalResume = (ResumeProc)GetProcAddress(originalKonamiLiveControllerModuleHandle, "KonamiLiveControllerResume");
				originalShutdown = (ShutdownProc)GetProcAddress(originalKonamiLiveControllerModuleHandle, "KonamiLiveControllerShutdown");
				originalSetKeyMapping = (SetKeyMappingProc)GetProcAddress(originalKonamiLiveControllerModuleHandle, "KonamiLiveControllerSetKeyMapping");
				originalSetOption = (SetOptionProc)GetProcAddress(originalKonamiLiveControllerModuleHandle, "KonamiLiveControllerSetOption");
				originalGetState = (GetStateProc)GetProcAddress(originalKonamiLiveControllerModuleHandle, "KonamiLiveControllerGetState");
				originalGetState2 = NULL;
				originalSetWindow = (SetWindowProc)GetProcAddress(originalKonamiLiveControllerModuleHandle, "KonamiLiveControllerSetWindow");
				originalGetControllerKey = (GetControllerKeyProc)GetProcAddress(originalKonamiLiveControllerModuleHandle, "KonamiLiveControllerGetControllerKey");
				originalGetCartridgeKey = (GetCartridgeKeyProc)GetProcAddress(originalKonamiLiveControllerModuleHandle, "KonamiLiveControlleretCartrdigeKey");
				originalRun = (RunProc)GetProcAddress(originalKonamiLiveControllerModuleHandle, "KonamiLiveControllerRun");
			}
		} else {
			HMODULE originalNetJetControllerModuleHandle = LoadLibrary("NetJetController_orig.DLL");

			if (originalNetJetControllerModuleHandle) {
				originalEnableKeyMapping = (EnableKeyMappingProc)GetProcAddress(originalNetJetControllerModuleHandle, "NetJetControllerEnableKeyMapping");
				originalDisableKeyMapping = (DisableKeyMappingProc)GetProcAddress(originalNetJetControllerModuleHandle, "NetJetControllerDisableKeyMapping");
				originalEnableMouseMapping = (EnableMouseMappingProc)GetProcAddress(originalNetJetControllerModuleHandle, "NetJetControllerEnableMouseMapping");
				originalDisableMouseMapping = (DisableMouseMappingProc)GetProcAddress(originalNetJetControllerModuleHandle, "NetJetControllerDisableMouseMapping");
				originalInitialize = (InitializeProc)GetProcAddress(originalNetJetControllerModuleHandle, "NetJetControllerInitialize");
				originalSuspend = (SuspendProc)GetProcAddress(originalNetJetControllerModuleHandle, "NetJetControllerSuspend");
				originalResume = (ResumeProc)GetProcAddress(originalNetJetControllerModuleHandle, "NetJetControllerResume");
				originalShutdown = (ShutdownProc)GetProcAddress(originalNetJetControllerModuleHandle, "NetJetControllerShutdown");
				originalSetKeyMapping = (SetKeyMappingProc)GetProcAddress(originalNetJetControllerModuleHandle, "NetJetControllerSetKeyMapping");
				originalSetOption = (SetOptionProc)GetProcAddress(originalNetJetControllerModuleHandle, "NetJetControllerSetOption");
				originalGetState = NULL;
				originalGetState2 = (GetState2Proc)GetProcAddress(originalNetJetControllerModuleHandle, "NetJetControllerGetState");
				originalSetWindow = (SetWindowProc)GetProcAddress(originalNetJetControllerModuleHandle, "NetJetControllerSetWindow");
				originalGetControllerKey = (GetControllerKeyProc)GetProcAddress(originalNetJetControllerModuleHandle, "NetJetControllerGetControllerKey");
				originalGetCartridgeKey = (GetCartridgeKeyProc)GetProcAddress(originalNetJetControllerModuleHandle, "NetJetControlleretCartrdigeKey");
				originalRun = (RunProc)GetProcAddress(originalNetJetControllerModuleHandle, "NetJetControllerRun");
			}
		}
	}
}

NetJetSimulator::~NetJetSimulator() {
	destroy();
}

BOOL NetJetSimulator::enableKeyMapping() {
	if (originalEnableKeyMapping) {
		originalEnableKeyMapping();
	}

	PWORD keyCodesDefault = new WORD[KEY_CODES_DEFAULT_SIZE];

	if (!keyCodesDefault) {
		shutdown();
		throw moduleType;
	}

	if (memcpy_s(keyCodesDefault, sizeof(keyCodesDefault), KEY_CODES_DEFAULT, sizeof(KEY_CODES_DEFAULT))) {
		shutdown();
		throw moduleType;
	}

	running = true;

	setKeyMapping(keyCodesDefault);

	running = false;

	delete[] keyCodesDefault;
	keyCodesDefault = NULL;
	return moduleType;
}

BOOL NetJetSimulator::disableKeyMapping() {
	if (originalDisableKeyMapping) {
		originalDisableKeyMapping();
	}

	if (keyMapping) {
		if (!backgroundThreadHook) {
			backgroundThreadHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, NULL, 0);
		}
	}

	keyMapping = false;
	return moduleType;
}

BOOL NetJetSimulator::enableMouseMapping() {
	if (originalEnableMouseMapping) {
		originalEnableMouseMapping();
	}

	// TODO: it'd be nice to map the Xbox 360 Controller and Keyboard to the Mouse here someday
	mouseMapping = true;
	return moduleType;
}

BOOL NetJetSimulator::disableMouseMapping() {
	if (originalDisableMouseMapping) {
		originalDisableMouseMapping();
	}

	mouseMapping = false;
	return moduleType;
}

BOOL NetJetSimulator::initialize() {
	if (!running) {
		if (originalInitialize) {
			originalInitialize();
		}
	}

	// we can't return !moduleType for failure because
	// the caller needs to know it, so it can also
	// return success or failure
	// so we throw it to signal an error instead
	if (initialized) {
		throw moduleType;
	}

	initialized = true;

	if (!backgroundThreadHook) {
		backgroundThreadHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, NULL, 0);
	}

	if (xbox360ControllerPointer) {
		xbox360ControllerPointer->initialize();
	}

	if (keyboardPointer) {
		keyboardPointer->initialize();
	}
	return moduleType;
}

BOOL NetJetSimulator::suspend() {
	if (originalSuspend) {
		originalSuspend();
	}

	suspended = true;
	return moduleType;
}

BOOL NetJetSimulator::resume() {
	if (originalResume) {
		originalResume();
	}

	suspended = false;
	return moduleType;
}

BOOL NetJetSimulator::shutdown() {
	if (originalShutdown) {
		originalShutdown();
	}

	if (!initialized) {
		throw moduleType;
	}

	initialized = false;

	if (xbox360ControllerPointer) {
		xbox360ControllerPointer->shutdown();
	}

	if (keyboardPointer) {
		keyboardPointer->shutdown();
	}

	if (backgroundThreadHook) {
		if (UnhookWindowsHookEx(backgroundThreadHook)) {
			backgroundThreadHook = NULL;
		}
	}
	return moduleType;
}

BOOL NetJetSimulator::setKeyMapping(PWORD keyCodes) {
	if (!running) {
		PWORD keyCodesDefault = NULL;

		if (!keyCodes) {
			keyCodesDefault = new WORD[KEY_CODES_DEFAULT_SIZE];

			if (!keyCodesDefault) {
				shutdown();
				throw moduleType;
			}

			if (memcpy_s(keyCodesDefault, sizeof(keyCodesDefault), KEY_CODES_DEFAULT, sizeof(KEY_CODES_DEFAULT))) {
				shutdown();
				throw moduleType;
			}

			keyCodes = keyCodesDefault;
		}

		if (originalSetKeyMapping) {
			originalSetKeyMapping(keyCodes);
		}

		if (keyCodesDefault) {
			delete[] keyCodesDefault;
			keyCodesDefault = NULL;
		}
	}

	// TODO: it'd be nice to map the Xbox 360 Controller to the Keyboard here someday
	if (!keyMapping) {
		if (backgroundThreadHook) {
			if (UnhookWindowsHookEx(backgroundThreadHook)) {
				backgroundThreadHook = NULL;
			}
		}
	}

	keyMapping = true;
	return moduleType;
}

BOOL NetJetSimulator::setOption(WORD option, WORD value) {
	if (originalSetOption) {
		originalSetOption(option, value);
	}
	return moduleType;
}

BOOL NetJetSimulator::getState(CONTROL* buttonsControlPointer, CONTROL* thumbRXControlPointer, CONTROL* thumbRYControlPointer) {
	bool netJetControllerInserted = false;
	bool xbox360ControllerInserted = false;

	{
		BOOL state = !moduleType;

		// note that we do not validate the pointers before calling these
		// why should we? If they're invalid, they'll throw an error or an exception
		// which the original program must then be able to handle, because
		// that would have happened without this library to intervene
		if (originalGetState) {
			state = originalGetState(buttonsControlPointer);
		} else if (originalGetState2) {
			state = originalGetState2(buttonsControlPointer, thumbRXControlPointer, thumbRYControlPointer);
		}

		if (state == moduleType) {
			if (buttonsControlPointer) {
				if (testButtonsControlMapping(*buttonsControlPointer, controllerInsertedMapping)) {
					netJetControllerInserted = true;
				}
			}
		}
	}

	if (!netJetControllerInserted) {
		resetState(buttonsControlPointer, thumbRXControlPointer, thumbRYControlPointer);
	}

	// if it isn't suspended...
	if (!suspended) {
		if (xbox360ControllerPointer) {
			xbox360ControllerPointer->getState(buttonsControlPointer, thumbRXControlPointer, thumbRYControlPointer, xbox360ControllerInserted);
		}

		// if it isn't already key mapping for us...
		if (!keyMapping) {
			// if it isn't multiplayer, we need to get the keyboard state
			// if it is multiplayer, but there is no player one controller, we need to get the keyboard state
			if (!allowMultiplayerState || (!netJetControllerInserted && !xbox360ControllerInserted)) {
				if (keyboardPointer) {
					keyboardPointer->getState(buttonsControlPointer, thumbRXControlPointer, thumbRYControlPointer);
				}
			}
		}
	}

	fixThumb(thumbRXControlPointer, thumbRYControlPointer);

	{
		if (buttonsControlPointer) {
			const bool INSERTED_STATE = true;

			CONTROL &buttonsControl = *buttonsControlPointer;

			setButtonsControlMapping(buttonsControl, controllerInsertedMapping, INSERTED_STATE);
			setButtonsControlMapping(buttonsControl, cartridgeInsertedMapping,  INSERTED_STATE);
		}
	}
	return moduleType;
}

BOOL NetJetSimulator::setWindow(HWND windowHandle) {
	// we don't want to post real messages to the window
	/*
	if (originalSetWindow) {
		originalSetWindow(windowHandle);
	}
	*/
	return moduleType;
}

BOOL NetJetSimulator::getControllerKey(LPVOID controllerKeyPointer) {
	if (originalGetControllerKey) {
		if (originalGetControllerKey(controllerKeyPointer) == moduleType) {
			return moduleType;
		}
	}

	// zero key, this is not a keygen
	// replicating real keys is outside the scope of this project
	if (controllerKeyPointer) {
		ZeroMemory(controllerKeyPointer, KEY_SIZE);
	}
	return moduleType;
}

BOOL NetJetSimulator::getCartridgeKey(LPVOID cartridgeKeyPointer) {
	if (originalGetCartridgeKey) {
		if (originalGetCartridgeKey(cartridgeKeyPointer) == moduleType) {
			return moduleType;
		}
	}

	if (cartridgeKeyPointer) {
		ZeroMemory(cartridgeKeyPointer, KEY_SIZE);
	}
	return moduleType;
}

BOOL NetJetSimulator::run() {
	if (originalRun) {
		originalRun();
	}

	PWORD keyCodesDefault = new WORD[KEY_CODES_DEFAULT_SIZE];

	if (!keyCodesDefault) {
		shutdown();
		throw moduleType;
	}

	if (memcpy_s(keyCodesDefault, sizeof(keyCodesDefault), KEY_CODES_DEFAULT, sizeof(KEY_CODES_DEFAULT))) {
		shutdown();
		throw moduleType;
	}

	running = true;

	initialize();
	setKeyMapping(keyCodesDefault);
	enableMouseMapping();

	running = false;

	delete[] keyCodesDefault;
	keyCodesDefault = NULL;
	return moduleType;
}

void NetJetSimulator::destroy() {
	xbox360ControllerGetMultiplayerState = 0;

	if (xbox360ControllerPointer) {
		delete xbox360ControllerPointer;
		xbox360ControllerPointer = 0;
	}

	if (keyboardPointer) {
		delete keyboardPointer;
		keyboardPointer = 0;
	}

	originalEnableKeyMapping = NULL;
	originalDisableKeyMapping = NULL;
	originalEnableMouseMapping = NULL;
	originalDisableMouseMapping = NULL;
	originalInitialize = NULL;
	originalSuspend = NULL;
	originalResume = NULL;
	originalShutdown = NULL;
	originalSetKeyMapping = NULL;
	originalSetOption = NULL;
	originalGetState = NULL;
	originalGetState2 = NULL;
	originalSetWindow = NULL;
	originalGetControllerKey = NULL;
	originalGetCartridgeKey = NULL;
	originalRun = NULL;
}

bool NetJetSimulator::getMultiplayerState() {
	if (!allowMultiplayerState) {
		return false;
	}

	{
		// the original Get State function is fast, so
		// it's safe to call it here
		BOOL state = !moduleType;

		CONTROL buttonsControl = BUTTONS_NONE_MAPPING;

		if (originalGetState) {
			state = originalGetState(&buttonsControl);
		} else if (originalGetState2) {
			CONTROL thumbRXControl = THUMB_CENTER_MAPPING;
			CONTROL thumbRYControl = THUMB_CENTER_MAPPING;

			state = originalGetState2(&buttonsControl, &thumbRXControl, &thumbRYControl);
		}

		if (state == moduleType && testButtonsControlMapping(buttonsControl, controllerInsertedMapping)) {
			return true;
		}
	}

	if (xbox360ControllerPointer && this->xbox360ControllerGetMultiplayerState) {
		if ((xbox360ControllerPointer->*(this->xbox360ControllerGetMultiplayerState))()) {
			return true;
		}
	}
	return false;
}

inline void NetJetSimulator::fixThumb(CONTROL* thumbXControlPointer, CONTROL* thumbYControlPointer) {
	const MAPPING THUMB_MIN_MAPPING = 0x00000000;
	const MAPPING THUMB_MAX_MAPPING = 0x00000040;

	if (thumbXControlPointer) {
		*thumbXControlPointer = min(max(*thumbXControlPointer, THUMB_MIN_MAPPING), THUMB_MAX_MAPPING);
	}

	if (thumbYControlPointer) {
		*thumbYControlPointer = min(max(*thumbYControlPointer, THUMB_MIN_MAPPING), THUMB_MAX_MAPPING);
	}
}

inline void NetJetSimulator::resetState(CONTROL* buttonsControlPointer, CONTROL* thumbRXControlPointer, CONTROL* thumbRYControlPointer) {
	if (buttonsControlPointer) {
		*buttonsControlPointer = BUTTONS_NONE_MAPPING;
	}

	if (thumbRXControlPointer) {
		*thumbRXControlPointer = THUMB_CENTER_MAPPING;
	}

	if (thumbRYControlPointer) {
		*thumbRYControlPointer = THUMB_CENTER_MAPPING;
	}
}