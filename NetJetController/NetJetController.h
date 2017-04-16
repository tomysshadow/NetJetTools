#pragma once
#include <windows.h>

/*
#pragma comment(linker, "/export:NetJetControllerEnableKeyMapping=NetJetController_.DLL.NetJetControllerEnableKeyMapping,@3")
#pragma comment(linker, "/export:NetJetControllerDisableKeyMapping=NetJetController_.DLL.NetJetControllerDisableKeyMapping,@1")
#pragma comment(linker, "/export:NetJetControllerEnableMouseMapping=NetJetController_.DLL.NetJetControllerEnableMouseMapping,@4")
#pragma comment(linker, "/export:NetJetControllerDisableMouseMapping=NetJetController_.DLL.NetJetControllerDisableMouseMapping,@2")
#pragma comment(linker, "/export:NetJetControllerInitialize=NetJetController_.DLL.NetJetControllerInitialize,@7")
#pragma comment(linker, "/export:NetJetControllerSuspend=NetJetController_.DLL.NetJetControllerSuspend,@14")
#pragma comment(linker, "/export:NetJetControllerResume=NetJetController_.DLL.NetJetControllerResume,@8")
#pragma comment(linker, "/export:NetJetControllerShutdown=NetJetController_.DLL.NetJetControllerShutdown,@13")
#pragma comment(linker, "/export:NetJetControllerSetKeyMapping=NetJetController_.DLL.NetJetControllerSetKeyMapping,@10")
#pragma comment(linker, "/export:NetJetControllerSetOption=NetJetController_.DLL.NetJetControllerSetOption,@11")
#pragma comment(linker, "/export:NetJetControllerGetState=NetJetController_.DLL.NetJetControllerGetState,@6")
#pragma comment(linker, "/export:NetJetControllerSetWindow=NetJetController_.DLL.NetJetControllerSetWindow,@12")
#pragma comment(linker, "/export:NetJetControllerGetControllerKey=NetJetController_.DLL.NetJetControllerGetControllerKey,@5")
#pragma comment(linker, "/export:NetJetControlleretCartrdigeKey=NetJetController_.DLL.NetJetControlleretCartrdigeKey,@15")
#pragma comment(linker, "/export:NetJetControllerRun=NetJetController_.DLL.NetJetControllerRun,@9")
*/








class NetJetEmulator {
public:
	bool keymapping = false;
	bool mousemapping = false;
	bool suspended = false;
	void callNetJetControllerGetState(int *, int *, int *, bool);
	void callNetJetControllerGetKey(char *, bool);
	class Keyboard {
	private:
		KBDLLHOOKSTRUCT keyboard;
		static const byte keyslength = 24;
		const DWORD keycodes[keyslength] = { 0x34, 0x32, 0x33, 0x31, 0x4C, 0x52, 0x30, 0x57, 0x53, 0x41, 0x44, VK_UP, VK_LEFT, VK_DOWN, VK_RIGHT, 0x46, 0x51, 0x45, VK_BACK, VK_RETURN, VK_SPACE, VK_LSHIFT, VK_RSHIFT, VK_ESCAPE };
	public:
		static LRESULT __stdcall backgroundThread(int, WPARAM, LPARAM);
		HHOOK backgroundThreadHook;
		bool keysdown[keyslength] = {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false};
	};
private:
	void SetState(int *, bool, int, bool);
	void SetControllerInserted(int *, int *, int *, bool, bool, bool);
	void SetCartridgeInserted(int *, bool);
	void FixThumbstick(int *, int *);
};