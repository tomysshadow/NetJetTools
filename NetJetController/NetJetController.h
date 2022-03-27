#pragma once
#include "shared.h"
#include <map>
#include <unordered_set>
#include <windows.h>
#include <xinput.h>

typedef DWORD(WINAPI *XInputGetStateProc)(_In_ DWORD dwUserIndex, _Out_ XINPUT_STATE *pState);

class NetJetSimulator {
	public:
	typedef bool(NetJetSimulator::*GetMultiplayerState)();

	static const MAPPING BUTTONS_NONE_MAPPING = 0x00000000;
	static const MAPPING THUMB_CENTER_MAPPING = 0x00000020;

	class Xbox360Controller {
		public:
		NetJetSimulator &netJetSimulator;

		typedef bool(Xbox360Controller::*GetMultiplayerState)();

		static const DWORD USER_INDEX = 0;

		Xbox360Controller(NetJetSimulator &netJetSimulator, Xbox360Controller::GetMultiplayerState &getMultiplayerState, bool &netJetSimulatorFileSet, HANDLE netJetSimulatorFile);
		~Xbox360Controller();

		void initialize();
		void shutdown();
		void getState(CONTROL* buttonsControlPointer, CONTROL* thumbRXControlPointer, CONTROL* thumbRYControlPointer, bool &controllerInserted);

		private:
		struct Mappings {
			MAPPING y = 0x00000001;
			MAPPING b = 0x00000002;
			MAPPING x = 0x00000004;
			MAPPING a = 0x00000008;
			MAPPING lb = 0x00000010;
			MAPPING rb = 0x00000020;
			MAPPING lt = 0x00000010;
			MAPPING rt = 0x00000020;
			MAPPING lsb = 0x00000000;
			MAPPING rsb = 0x00000000;
			MAPPING start = 0x00000100;
			MAPPING back = 0x00000000;
			MAPPING dU = 0x00000200;
			MAPPING dD = 0x00000400;
			MAPPING dL = 0x00000800;
			MAPPING dR = 0x00001000;
			MAPPING lsU = 0x00000200;
			MAPPING lsD = 0x00000400;
			MAPPING lsL = 0x00000800;
			MAPPING lsR = 0x00001000;
			MAPPING rs = 0x00000040;
		};

		Mappings mappings;

		XInputGetStateProc xinputGetState = NULL;

		bool throttledTickCountSet = false;
		DWORD throttledTickCount = 0;

		void destroy();
		void getStateThrottled(XINPUT_STATE &state, bool &controllerInserted);
		bool getMultiplayerState();
	};

	class Keyboard {
		public:
		NetJetSimulator &netJetSimulator;

		NetJetSimulator::GetMultiplayerState netJetSimulatorGetMultiplayerState = 0;

		typedef std::map<DWORD, MAPPING> VK_CODE_MAPPING_MAP;

		VK_CODE_MAPPING_MAP buttonsControlVKCodeMappingMap = {
			{0x34, 0x00000001},
			{0x46, 0x00000001},
			{0x32, 0x00000002},
			{0x51, 0x00000002},
			{VK_BACK, 0x00000002},
			{0x33, 0x00000004},
			{0x45, 0x00000004},
			{0x31, 0x00000008},
			{VK_RETURN, 0x00000008},
			{VK_SPACE, 0x00000008},
			{0x4C, 0x00000010},
			{VK_LSHIFT, 0x00000010},
			{0x52, 0x00000020},
			{VK_RSHIFT, 0x00000020},
			{0x30, 0x00000100},
			{VK_ESCAPE, 0x00000100},
			{0x57, 0x00000200},
			{0x53, 0x00000400},
			{0x41, 0x00000800},
			{0x44, 0x00001000}
		};

		VK_CODE_MAPPING_MAP thumbRXControlVKCodeMappingMap = {
			{VK_LEFT, 0x00000000},
			{VK_RIGHT, 0x00000040}
		};

		VK_CODE_MAPPING_MAP thumbRYControlVKCodeMappingMap = {
			{VK_UP, 0x00000000},
			{VK_DOWN, 0x00000040}
		};

		typedef std::unordered_set<DWORD> VK_CODE_SET;

		VK_CODE_SET vkCodeSet = {};

		Keyboard(NetJetSimulator &netJetSimulator, NetJetSimulator::GetMultiplayerState netJetSimulatorGetMultiplayerState, bool &netJetSimulatorFileSet, HANDLE netJetSimulatorFile);
		~Keyboard();

		void initialize();
		void shutdown();
		void getState(CONTROL* buttonsControlPointer, CONTROL* thumbRXControlPointer, CONTROL* thumbRYControlPointer);

		private:
		void destroy();
		bool readVKCodeMappingMap(HANDLE file, VK_CODE_MAPPING_MAP &vkCodeMappingMap);
	};

	NetJetSimulator(bool konamiLive, NetJetSimulator::GetMultiplayerState &getMultiplayerState, bool &netJetSimulatorFileSet, HANDLE netJetSimulatorFile);
	~NetJetSimulator();

	BOOL enableKeyMapping();
	BOOL disableKeyMapping();
	BOOL enableMouseMapping();
	BOOL disableMouseMapping();
	BOOL initialize();
	BOOL suspend();
	BOOL resume();
	BOOL shutdown();
	BOOL setKeyMapping(PWORD keyCodes);
	BOOL setOption(WORD option, WORD value);
	BOOL getState(CONTROL* buttonsControlPointer, CONTROL* thumbRXControlPointer, CONTROL* thumbRYControlPointer);
	BOOL setWindow(HWND windowHandle);
	BOOL getControllerKey(LPVOID controllerKeyPointer);
	BOOL getCartridgeKey(LPVOID cartridgeKeyPointer);
	BOOL run();

	private:
	void destroy();
	bool getMultiplayerState();

	inline void fixThumb(CONTROL* thumbX, CONTROL* thumbY);
	inline void resetState(CONTROL* buttonsControlPointer, CONTROL* thumbRXControlPointer, CONTROL* thumbRYControlPointer);

	Xbox360Controller::GetMultiplayerState xbox360ControllerGetMultiplayerState = 0;

	Xbox360Controller* xbox360ControllerPointer = 0;
	Keyboard* keyboardPointer = 0;

	typedef BOOL(*EnableKeyMappingProc)();
	typedef BOOL(*DisableKeyMappingProc)();
	typedef BOOL(*EnableMouseMappingProc)();
	typedef BOOL(*DisableMouseMappingProc)();
	typedef BOOL(*InitializeProc)();
	typedef BOOL(*SuspendProc)();
	typedef BOOL(*ResumeProc)();
	typedef BOOL(*ShutdownProc)();
	typedef BOOL(*SetKeyMappingProc)(PWORD keys);
	typedef BOOL(*SetOptionProc)(WORD option, WORD value);
	typedef BOOL(*GetStateProc)(CONTROL* buttonsControlPointer);
	typedef BOOL(*GetState2Proc)(CONTROL* buttonsControlPointer, CONTROL* thumbRXControlPointer, CONTROL* thumbRYControlPointer);
	typedef BOOL(*SetWindowProc)(HWND windowHandle);
	typedef BOOL(*GetControllerKeyProc)(LPVOID controllerKeyPointer);
	typedef BOOL(*GetCartridgeKeyProc)(LPVOID cartridgeKeyPointer);
	typedef BOOL(*RunProc)();

	EnableKeyMappingProc originalEnableKeyMapping = NULL;
	DisableKeyMappingProc originalDisableKeyMapping = NULL;
	EnableMouseMappingProc originalEnableMouseMapping = NULL;
	DisableMouseMappingProc originalDisableMouseMapping = NULL;
	InitializeProc originalInitialize = NULL;
	SuspendProc originalSuspend = NULL;
	ResumeProc originalResume = NULL;
	ShutdownProc originalShutdown = NULL;
	SetKeyMappingProc originalSetKeyMapping = NULL;
	SetOptionProc originalSetOption = NULL;
	GetStateProc originalGetState = NULL;
	GetState2Proc originalGetState2 = NULL;
	SetWindowProc originalSetWindow = NULL;
	GetControllerKeyProc originalGetControllerKey = NULL;
	GetCartridgeKeyProc originalGetCartridgeKey = NULL;
	RunProc originalRun = NULL;

	MAPPING controllerInsertedMapping = 0x00010000;
	MAPPING cartridgeInsertedMapping = 0x00100080;
	bool allowMultiplayerState = false;
	BOOL moduleType = FALSE;

	static const size_t KEY_CODES_DEFAULT_SIZE = 16;
	const WORD KEY_CODES_DEFAULT[KEY_CODES_DEFAULT_SIZE] = {
		0x0034,
		0x0032,
		0x0033,
		0x0031,
		0x004C,
		0x0052,
		0x0000,
		0x0000,
		0x0030,
		0x0057,
		0x0053,
		0x0041,
		0x0044,
		0x0000,
		0x0000,
		0x0000
	};

	static const SIZE_T KEY_SIZE = 32;

	bool initialized = false;
	bool suspended = false;
	bool keyMapping = false;
	bool mouseMapping = false;
	bool running = false;
};

typedef std::unordered_set<NetJetSimulator::Keyboard*> KEYBOARD_POINTER_SET;