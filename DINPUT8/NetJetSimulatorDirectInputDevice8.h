#pragma once
#include "shared.h"
#include <windows.h>
#include <dinput.h>

enum INPUT_DEVICE {
	INPUT_DEVICE_MOUSE,
	INPUT_DEVICE_KEYBOARD,
	INPUT_DEVICE_JOYSTICK
};

typedef bool(*GetMultiplayerStateProc)();

class NetJetSimulatorDirectInputDevice8 : public IDirectInputDevice8 {
	public:
	typedef void(NetJetSimulatorDirectInputDevice8::*SyncMultiplayerState)();

	NetJetSimulatorDirectInputDevice8(IDirectInputDevice8* originalDirectInputDevice8Pointer, INPUT_DEVICE inputDevice);
	virtual ~NetJetSimulatorDirectInputDevice8();

	// BEGIN: the original DirectX function definitions
	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj);
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	/*** IDirectInputDevice8 methods ***/
	STDMETHOD(GetCapabilities)(THIS_ LPDIDEVCAPS);
	STDMETHOD(EnumObjects)(THIS_ LPDIENUMDEVICEOBJECTSCALLBACK, LPVOID, DWORD);
	STDMETHOD(GetProperty)(THIS_ REFGUID, LPDIPROPHEADER);
	STDMETHOD(SetProperty)(THIS_ REFGUID, LPCDIPROPHEADER);
	STDMETHOD(Acquire)(THIS);
	STDMETHOD(Unacquire)(THIS);
	STDMETHOD(GetDeviceState)(THIS_ DWORD, LPVOID);
	STDMETHOD(GetDeviceData)(THIS_ DWORD, LPDIDEVICEOBJECTDATA, LPDWORD, DWORD);
	STDMETHOD(SetDataFormat)(THIS_ LPCDIDATAFORMAT);
	STDMETHOD(SetEventNotification)(THIS_ HANDLE);
	STDMETHOD(SetCooperativeLevel)(THIS_ HWND, DWORD);
	STDMETHOD(GetObjectInfo)(THIS_ LPDIDEVICEOBJECTINSTANCE, DWORD, DWORD);
	STDMETHOD(GetDeviceInfo)(THIS_ LPDIDEVICEINSTANCE);
	STDMETHOD(RunControlPanel)(THIS_ HWND, DWORD);
	STDMETHOD(Initialize)(THIS_ HINSTANCE, DWORD, REFGUID);
	STDMETHOD(CreateEffect)(THIS_ REFGUID, LPCDIEFFECT, LPDIRECTINPUTEFFECT *, LPUNKNOWN);
	STDMETHOD(EnumEffects)(THIS_ LPDIENUMEFFECTSCALLBACK, LPVOID, DWORD);
	STDMETHOD(GetEffectInfo)(THIS_ LPDIEFFECTINFO, REFGUID);
	STDMETHOD(GetForceFeedbackState)(THIS_ LPDWORD);
	STDMETHOD(SendForceFeedbackCommand)(THIS_ DWORD);
	STDMETHOD(EnumCreatedEffectObjects)(THIS_ LPDIENUMCREATEDEFFECTOBJECTSCALLBACK, LPVOID, DWORD);
	STDMETHOD(Escape)(THIS_ LPDIEFFESCAPE);
	STDMETHOD(Poll)(THIS);
	STDMETHOD(SendDeviceData)(THIS_ DWORD, LPCDIDEVICEOBJECTDATA, LPDWORD, DWORD);
	STDMETHOD(EnumEffectsInFile)(THIS_ LPCSTR, LPDIENUMEFFECTSINFILECALLBACK, LPVOID, DWORD);
	STDMETHOD(WriteEffectToFile)(THIS_ LPCSTR, DWORD, LPDIFILEEFFECT, DWORD);
	STDMETHOD(BuildActionMap)(THIS_ LPDIACTIONFORMAT, LPCSTR, DWORD);
	STDMETHOD(SetActionMap)(THIS_ LPDIACTIONFORMAT, LPCSTR, DWORD);
	STDMETHOD(GetImageInfo)(THIS_ LPDIDEVICEIMAGEINFOHEADER);
	// END: the original DirectX function definitions

	private:
	// custom
	void syncMultiplayerState();

	IDirectInputDevice8* originalDirectInputDevice8Pointer = NULL;
	INPUT_DEVICE inputDevice = INPUT_DEVICE_JOYSTICK;

	bool multiplayerState = false;
};