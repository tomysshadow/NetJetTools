#pragma once
#include "shared.h"
#include "NetJetSimulatorDirectInputDevice8.h"
#include <windows.h>
#include <dinput.h>

class NetJetSimulatorDirectInput8 : public IDirectInput8 {
	public:
	NetJetSimulatorDirectInput8(IDirectInput8* originalDirectInput8Pointer);
	virtual ~NetJetSimulatorDirectInput8();

	// BEGIN: the original DirectX function definitions
	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj);
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	/*** IDirectInput8 methods ***/
	STDMETHOD(CreateDevice)(THIS_ REFGUID, LPDIRECTINPUTDEVICE8 *, LPUNKNOWN);
	STDMETHOD(EnumDevices)(THIS_ DWORD, LPDIENUMDEVICESCALLBACK, LPVOID, DWORD);
	STDMETHOD(GetDeviceStatus)(THIS_ REFGUID);
	STDMETHOD(RunControlPanel)(THIS_ HWND, DWORD);
	STDMETHOD(Initialize)(THIS_ HINSTANCE, DWORD);
	STDMETHOD(FindDevice)(THIS_ REFGUID, LPCSTR, LPGUID);
	STDMETHOD(EnumDevicesBySemantics)(THIS_ LPCSTR, LPDIACTIONFORMAT, LPDIENUMDEVICESBYSEMANTICSCB, LPVOID, DWORD);
	STDMETHOD(ConfigureDevices)(THIS_ LPDICONFIGUREDEVICESCALLBACK, LPDICONFIGUREDEVICESPARAMS, DWORD, LPVOID);
	// END: the original DirectX function definitions

	private:
	// custom
	IDirectInput8* originalDirectInput8Pointer = NULL;
};