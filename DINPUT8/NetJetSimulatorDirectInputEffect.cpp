#include "shared.h"
#include "NetJetSimulatorDirectInputEffect.h"
#include <windows.h>
#include <dinput.h>

NET_JET_SIMULATOR_DIRECT_INPUT_DEVICE_8_POINTER_MAP netJetSimulatorDirectInputDevice8PointerMap = {};

NetJetSimulatorDirectInputEffect::NetJetSimulatorDirectInputEffect(IDirectInputEffect* originalDirectInputEffectPointer, NetJetSimulatorDirectInputEffect::DestroyNetJetSimulatorDirectInputDevice8Pointer &destroyNetJetSimulatorDirectInputDevice8Pointer, NetJetSimulatorDirectInputDevice8* netJetSimulatorDirectInputDevice8Pointer, NetJetSimulatorDirectInputDevice8::SyncMultiplayerState netJetSimulatorDirectInputDevice8SyncMultiplayerState) : netJetSimulatorDirectInputDevice8Pointer(netJetSimulatorDirectInputDevice8Pointer), netJetSimulatorDirectInputDevice8SyncMultiplayerState(netJetSimulatorDirectInputDevice8SyncMultiplayerState) {
	destroyNetJetSimulatorDirectInputDevice8Pointer = &NetJetSimulatorDirectInputEffect::destroyNetJetSimulatorDirectInputDevice8Pointer;
	
	if (!originalDirectInputEffectPointer) {
		throw "originalDirectInputEffectPointer must not be NULL";
	}

	this->originalDirectInputEffectPointer = originalDirectInputEffectPointer;
}

NetJetSimulatorDirectInputEffect::~NetJetSimulatorDirectInputEffect() {
	originalDirectInputEffectPointer = NULL;
	destroyNetJetSimulatorDirectInputDevice8Pointer();
}

/*** IUnknown methods ***/
COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE NetJetSimulatorDirectInputEffect::QueryInterface(THIS_ REFIID riid, LPVOID * ppvObj) {
	return originalDirectInputEffectPointer->QueryInterface(riid, ppvObj);
}

COM_DECLSPEC_NOTHROW ULONG STDMETHODCALLTYPE NetJetSimulatorDirectInputEffect::AddRef(THIS) {
	return originalDirectInputEffectPointer->AddRef();
}

COM_DECLSPEC_NOTHROW ULONG STDMETHODCALLTYPE NetJetSimulatorDirectInputEffect::Release(THIS) {
	ULONG refCount = originalDirectInputEffectPointer->Release();

	// in case no further Ref is there, the Original Object has deleted itself
	// so do we here
	if (!refCount) {
		if (netJetSimulatorDirectInputDevice8Pointer) {
			// notify the device that the effect is being destroyed
			NET_JET_SIMULATOR_DIRECT_INPUT_DEVICE_8_POINTER_MAP::iterator netJetSimulatorDirectInputDevice8PointerMapIterator = netJetSimulatorDirectInputDevice8PointerMap.find(netJetSimulatorDirectInputDevice8Pointer);

			if (netJetSimulatorDirectInputDevice8PointerMapIterator != netJetSimulatorDirectInputDevice8PointerMap.end()) {
				netJetSimulatorDirectInputDevice8PointerMapIterator->second.erase(this);
			}
		}

		delete this;
	}
	return refCount;
}

/*** IDirectInputEffect methods ***/
COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE NetJetSimulatorDirectInputEffect::Initialize(THIS_ HINSTANCE hinst, DWORD dwVersion, REFGUID rguid) {
	syncMultiplayerState();
	return originalDirectInputEffectPointer->Initialize(hinst, dwVersion, rguid);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE NetJetSimulatorDirectInputEffect::GetEffectGuid(THIS_ LPGUID pguid) {
	syncMultiplayerState();
	return originalDirectInputEffectPointer->GetEffectGuid(pguid);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE NetJetSimulatorDirectInputEffect::GetParameters(THIS_ LPDIEFFECT peff, DWORD dwFlags) {
	syncMultiplayerState();
	return originalDirectInputEffectPointer->GetParameters(peff, dwFlags);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE NetJetSimulatorDirectInputEffect::SetParameters(THIS_ LPCDIEFFECT peff, DWORD dwFlags) {
	syncMultiplayerState();
	return originalDirectInputEffectPointer->SetParameters(peff, dwFlags);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE NetJetSimulatorDirectInputEffect::Start(THIS_ DWORD dwIterations, DWORD dwFlags) {
	syncMultiplayerState();
	return originalDirectInputEffectPointer->Start(dwIterations, dwFlags);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE NetJetSimulatorDirectInputEffect::Stop(THIS) {
	syncMultiplayerState();
	return originalDirectInputEffectPointer->Stop();
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE NetJetSimulatorDirectInputEffect::GetEffectStatus(THIS_ LPDWORD pdwFlags) {
	syncMultiplayerState();
	return originalDirectInputEffectPointer->GetEffectStatus(pdwFlags);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE NetJetSimulatorDirectInputEffect::Download(THIS) {
	syncMultiplayerState();
	return originalDirectInputEffectPointer->Download();
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE NetJetSimulatorDirectInputEffect::Unload(THIS) {
	syncMultiplayerState();
	return originalDirectInputEffectPointer->Unload();
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE NetJetSimulatorDirectInputEffect::Escape(THIS_ LPDIEFFESCAPE pesc) {
	syncMultiplayerState();
	return originalDirectInputEffectPointer->Escape(pesc);
}

void NetJetSimulatorDirectInputEffect::destroyNetJetSimulatorDirectInputDevice8Pointer() {
	netJetSimulatorDirectInputDevice8Pointer = NULL;
	netJetSimulatorDirectInputDevice8SyncMultiplayerState = 0;
}

void NetJetSimulatorDirectInputEffect::syncMultiplayerState() {
	if (netJetSimulatorDirectInputDevice8Pointer && netJetSimulatorDirectInputDevice8SyncMultiplayerState) {
		(netJetSimulatorDirectInputDevice8Pointer->*netJetSimulatorDirectInputDevice8SyncMultiplayerState)();
	}
}