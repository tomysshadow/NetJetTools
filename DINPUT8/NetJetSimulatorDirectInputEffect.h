#pragma once
#include "shared.h"
#include "NetJetSimulatorDirectInputDevice8.h"
#include <windows.h>
#include <dinput.h>
#include <map>

class NetJetSimulatorDirectInputEffect : public IDirectInputEffect {
	public:
	typedef void(NetJetSimulatorDirectInputEffect::*DestroyNetJetSimulatorDirectInputDevice8Pointer)();

	NetJetSimulatorDirectInputEffect(IDirectInputEffect* originalDirectInputEffectPointer, NetJetSimulatorDirectInputEffect::DestroyNetJetSimulatorDirectInputDevice8Pointer &destroyNetJetSimulatorDirectInputDevice8Pointer, NetJetSimulatorDirectInputDevice8* netJetSimulatorDirectInputDevice8Pointer, NetJetSimulatorDirectInputDevice8::SyncMultiplayerState netJetSimulatorDirectInputDevice8SyncMultiplayerState);
	virtual ~NetJetSimulatorDirectInputEffect();

	// BEGIN: the original DirectX function definitions
	/*** IUnknown methods ***/
	STDMETHOD(QueryInterface)(THIS_ REFIID riid, LPVOID * ppvObj);
	STDMETHOD_(ULONG, AddRef)(THIS);
	STDMETHOD_(ULONG, Release)(THIS);

	/*** IDirectInputEffect methods ***/
	STDMETHOD(Initialize)(THIS_ HINSTANCE, DWORD, REFGUID);
	STDMETHOD(GetEffectGuid)(THIS_ LPGUID);
	STDMETHOD(GetParameters)(THIS_ LPDIEFFECT, DWORD);
	STDMETHOD(SetParameters)(THIS_ LPCDIEFFECT, DWORD);
	STDMETHOD(Start)(THIS_ DWORD, DWORD);
	STDMETHOD(Stop)(THIS);
	STDMETHOD(GetEffectStatus)(THIS_ LPDWORD);
	STDMETHOD(Download)(THIS);
	STDMETHOD(Unload)(THIS);
	STDMETHOD(Escape)(THIS_ LPDIEFFESCAPE);
	// END: the original DirectX function definitions

	private:
	// custom
	void destroyNetJetSimulatorDirectInputDevice8Pointer();
	void syncMultiplayerState();

	IDirectInputEffect* originalDirectInputEffectPointer = NULL;
	NetJetSimulatorDirectInputDevice8* netJetSimulatorDirectInputDevice8Pointer = NULL;
	NetJetSimulatorDirectInputDevice8::SyncMultiplayerState netJetSimulatorDirectInputDevice8SyncMultiplayerState = 0;
};

typedef std::map<NetJetSimulatorDirectInputEffect*, NetJetSimulatorDirectInputEffect::DestroyNetJetSimulatorDirectInputDevice8Pointer> NET_JET_SIMULATOR_DIRECT_INPUT_EFFECT_POINTER_MAP;
typedef std::map<NetJetSimulatorDirectInputDevice8*, NET_JET_SIMULATOR_DIRECT_INPUT_EFFECT_POINTER_MAP> NET_JET_SIMULATOR_DIRECT_INPUT_DEVICE_8_POINTER_MAP;

extern NET_JET_SIMULATOR_DIRECT_INPUT_DEVICE_8_POINTER_MAP netJetSimulatorDirectInputDevice8PointerMap;