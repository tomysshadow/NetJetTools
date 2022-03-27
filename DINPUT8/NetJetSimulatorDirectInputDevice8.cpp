#include "shared.h"
#include "NetJetSimulatorDirectInputDevice8.h"
#include "NetJetSimulatorDirectInputEffect.h"
#include <windows.h>
#include <dinput.h>

GetMultiplayerStateProc getMultiplayerState = 0;

NetJetSimulatorDirectInputDevice8::NetJetSimulatorDirectInputDevice8(IDirectInputDevice8* originalDirectInputDevice8Pointer, INPUT_DEVICE inputDevice) : inputDevice(inputDevice) {
	if (!originalDirectInputDevice8Pointer) {
		throw "originalDirectInputDevice8Pointer must not be NULL";
	}

	this->originalDirectInputDevice8Pointer = originalDirectInputDevice8Pointer;
}

NetJetSimulatorDirectInputDevice8::~NetJetSimulatorDirectInputDevice8() {
	originalDirectInputDevice8Pointer = NULL;
}

/*** IUnknown methods ***/
COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE NetJetSimulatorDirectInputDevice8::QueryInterface(THIS_ REFIID riid, LPVOID * ppvObj) {
	return originalDirectInputDevice8Pointer->QueryInterface(riid, ppvObj);
}

COM_DECLSPEC_NOTHROW ULONG STDMETHODCALLTYPE NetJetSimulatorDirectInputDevice8::AddRef(THIS) {
	return originalDirectInputDevice8Pointer->AddRef();
}

COM_DECLSPEC_NOTHROW ULONG STDMETHODCALLTYPE NetJetSimulatorDirectInputDevice8::Release(THIS) {
	// release own objects
	// ...

	ULONG refCount = originalDirectInputDevice8Pointer->Release();

	// in case no further Ref is there, the Original Object has deleted itself
	// so do we here
	if (!refCount) {
		// notify our effects that the device is being destroyed
		NET_JET_SIMULATOR_DIRECT_INPUT_DEVICE_8_POINTER_MAP::iterator netJetSimulatorDirectInputDevice8PointerMapIterator = netJetSimulatorDirectInputDevice8PointerMap.find(this);

		if (netJetSimulatorDirectInputDevice8PointerMapIterator != netJetSimulatorDirectInputDevice8PointerMap.end()) {
			NET_JET_SIMULATOR_DIRECT_INPUT_EFFECT_POINTER_MAP &netJetSimulatorDirectInputEffectPointerMap = netJetSimulatorDirectInputDevice8PointerMapIterator->second;

			for (NET_JET_SIMULATOR_DIRECT_INPUT_EFFECT_POINTER_MAP::iterator netJetSimulatorDirectInputEffectPointerMapIterator = netJetSimulatorDirectInputEffectPointerMap.begin(); netJetSimulatorDirectInputEffectPointerMapIterator != netJetSimulatorDirectInputEffectPointerMap.end(); netJetSimulatorDirectInputEffectPointerMapIterator++) {
				if (netJetSimulatorDirectInputEffectPointerMapIterator->first && netJetSimulatorDirectInputEffectPointerMapIterator->second) {
					((netJetSimulatorDirectInputEffectPointerMapIterator->first)->*(netJetSimulatorDirectInputEffectPointerMapIterator->second))();
				}
			}

			netJetSimulatorDirectInputDevice8PointerMap.erase(netJetSimulatorDirectInputDevice8PointerMapIterator);
		}

		delete this;
	}
	return refCount;
}

/*** IDirectInputDevice8 methods ***/
COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE NetJetSimulatorDirectInputDevice8::GetCapabilities(THIS_ LPDIDEVCAPS lpDIDevCaps) {
	syncMultiplayerState();
	return originalDirectInputDevice8Pointer->GetCapabilities(lpDIDevCaps);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE NetJetSimulatorDirectInputDevice8::EnumObjects(THIS_ LPDIENUMDEVICEOBJECTSCALLBACK lpCallback, LPVOID pvRef, DWORD dwFlags) {
	syncMultiplayerState();
	return originalDirectInputDevice8Pointer->EnumObjects(lpCallback, pvRef, dwFlags);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE NetJetSimulatorDirectInputDevice8::GetProperty(THIS_ REFGUID rguidProp, LPDIPROPHEADER pdiph) {
	syncMultiplayerState();
	return originalDirectInputDevice8Pointer->GetProperty(rguidProp, pdiph);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE NetJetSimulatorDirectInputDevice8::SetProperty(THIS_ REFGUID rguidProp, LPCDIPROPHEADER pdiph) {
	syncMultiplayerState();
	return originalDirectInputDevice8Pointer->SetProperty(rguidProp, pdiph);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE NetJetSimulatorDirectInputDevice8::Acquire(THIS) {
	if (inputDevice == INPUT_DEVICE_MOUSE) {
		return originalDirectInputDevice8Pointer->Acquire();
	}

	HRESULT err = DI_OK;

	// in a multiplayer state, the keyboard is handled by DirectInput
	// otherwise it is handled by Window Messages
	if (inputDevice == INPUT_DEVICE_KEYBOARD) {
		if (getMultiplayerState) {
			if (getMultiplayerState()) {
				err = originalDirectInputDevice8Pointer->Acquire();

				if (err == DI_OK || err == S_FALSE) {
					multiplayerState = true;
				}
				return err;
			}
		}
	}

	// otherwise this is a joystick, which is handled by XInput, not DirectInput
	// unacquire the device, in case we were allowing acquisition previously
	err = originalDirectInputDevice8Pointer->Unacquire();

	if (err != DI_OK && err != DI_NOEFFECT) {
		return err;
	}

	// the device was unacquired successfully
	// the application should think the device was acquired by this application
	multiplayerState = false;
	return DI_OK;
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE NetJetSimulatorDirectInputDevice8::Unacquire(THIS) {
	if (inputDevice == INPUT_DEVICE_MOUSE) {
		return originalDirectInputDevice8Pointer->Unacquire();
	}

	// unacquire the device, in case we were allowing acquisition previously
	HRESULT err = originalDirectInputDevice8Pointer->Unacquire();

	if (err != DI_OK && err != DI_NOEFFECT) {
		return err;
	}

	// the device was unacquired successfully
	// the application should think the device was acquired by another application
	multiplayerState = false;
	return DI_NOEFFECT;
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE NetJetSimulatorDirectInputDevice8::GetDeviceState(THIS_ DWORD cbData, LPVOID lpvData) {
	syncMultiplayerState();
	return originalDirectInputDevice8Pointer->GetDeviceState(cbData, lpvData);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE NetJetSimulatorDirectInputDevice8::GetDeviceData(THIS_ DWORD cbObjectData, LPDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD dwFlags) {
	syncMultiplayerState();
	return originalDirectInputDevice8Pointer->GetDeviceData(cbObjectData, rgdod, pdwInOut, dwFlags);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE NetJetSimulatorDirectInputDevice8::SetDataFormat(THIS_ LPCDIDATAFORMAT lpdf) {
	syncMultiplayerState();
	return originalDirectInputDevice8Pointer->SetDataFormat(lpdf);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE NetJetSimulatorDirectInputDevice8::SetEventNotification(THIS_ HANDLE hEvent) {
	syncMultiplayerState();
	return originalDirectInputDevice8Pointer->SetEventNotification(hEvent);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE NetJetSimulatorDirectInputDevice8::SetCooperativeLevel(THIS_ HWND hwnd, DWORD dwFlags) {
	syncMultiplayerState();
	return originalDirectInputDevice8Pointer->SetCooperativeLevel(hwnd, dwFlags);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE NetJetSimulatorDirectInputDevice8::GetObjectInfo(THIS_ LPDIDEVICEOBJECTINSTANCE pdidoi, DWORD dwObj, DWORD dwHow) {
	syncMultiplayerState();
	return originalDirectInputDevice8Pointer->GetObjectInfo(pdidoi, dwObj, dwHow);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE NetJetSimulatorDirectInputDevice8::GetDeviceInfo(THIS_ LPDIDEVICEINSTANCE pdidi) {
	syncMultiplayerState();
	return originalDirectInputDevice8Pointer->GetDeviceInfo(pdidi);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE NetJetSimulatorDirectInputDevice8::RunControlPanel(THIS_ HWND hwndOwner, DWORD dwFlags) {
	syncMultiplayerState();
	return originalDirectInputDevice8Pointer->RunControlPanel(hwndOwner, dwFlags);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE NetJetSimulatorDirectInputDevice8::Initialize(THIS_ HINSTANCE hinst, DWORD dwVersion, REFGUID rguid) {
	syncMultiplayerState();
	return originalDirectInputDevice8Pointer->Initialize(hinst, dwVersion, rguid);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE NetJetSimulatorDirectInputDevice8::EnumEffects(THIS_ LPDIENUMEFFECTSCALLBACK lpCallback, LPVOID pvRef, DWORD dwEffType) {
	syncMultiplayerState();
	return originalDirectInputDevice8Pointer->EnumEffects(lpCallback, pvRef, dwEffType);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE NetJetSimulatorDirectInputDevice8::GetEffectInfo(THIS_ LPDIEFFECTINFO pdei, REFGUID rguid) {
	syncMultiplayerState();
	return originalDirectInputDevice8Pointer->GetEffectInfo(pdei, rguid);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE NetJetSimulatorDirectInputDevice8::GetForceFeedbackState(THIS_ LPDWORD pdwOut) {
	syncMultiplayerState();
	return originalDirectInputDevice8Pointer->GetForceFeedbackState(pdwOut);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE NetJetSimulatorDirectInputDevice8::SendForceFeedbackCommand(THIS_ DWORD dwFlags) {
	syncMultiplayerState();
	return originalDirectInputDevice8Pointer->SendForceFeedbackCommand(dwFlags);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE NetJetSimulatorDirectInputDevice8::Escape(THIS_ LPDIEFFESCAPE pesc) {
	syncMultiplayerState();
	return originalDirectInputDevice8Pointer->Escape(pesc);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE NetJetSimulatorDirectInputDevice8::Poll(THIS) {
	syncMultiplayerState();
	return originalDirectInputDevice8Pointer->Poll();
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE NetJetSimulatorDirectInputDevice8::SendDeviceData(THIS_ DWORD cbObjectData, LPCDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD fl) {
	syncMultiplayerState();
	return originalDirectInputDevice8Pointer->SendDeviceData(cbObjectData, rgdod, pdwInOut, fl);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE NetJetSimulatorDirectInputDevice8::EnumEffectsInFile(THIS_ LPCSTR lpszFileName, LPDIENUMEFFECTSINFILECALLBACK pec, LPVOID pvRef, DWORD dwFlags) {
	syncMultiplayerState();
	return originalDirectInputDevice8Pointer->EnumEffectsInFile(lpszFileName, pec, pvRef, dwFlags);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE NetJetSimulatorDirectInputDevice8::WriteEffectToFile(THIS_ LPCSTR lpszFileName, DWORD dwEntries, LPDIFILEEFFECT rgDiFileEft, DWORD dwFlags) {
	syncMultiplayerState();
	return originalDirectInputDevice8Pointer->WriteEffectToFile(lpszFileName, dwEntries, rgDiFileEft, dwFlags);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE NetJetSimulatorDirectInputDevice8::BuildActionMap(THIS_ LPDIACTIONFORMAT lpdiaf, LPCSTR lpszUserName, DWORD dwFlags) {
	syncMultiplayerState();
	return originalDirectInputDevice8Pointer->BuildActionMap(lpdiaf, lpszUserName, dwFlags);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE NetJetSimulatorDirectInputDevice8::SetActionMap(THIS_ LPDIACTIONFORMAT lpdiActionFormat, LPCSTR lptszUserName, DWORD dwFlags) {
	syncMultiplayerState();
	return originalDirectInputDevice8Pointer->SetActionMap(lpdiActionFormat, lptszUserName, dwFlags);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE NetJetSimulatorDirectInputDevice8::GetImageInfo(THIS_ LPDIDEVICEIMAGEINFOHEADER lpdiDevImageInfoHeader) {
	syncMultiplayerState();
	return originalDirectInputDevice8Pointer->GetImageInfo(lpdiDevImageInfoHeader);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE NetJetSimulatorDirectInputDevice8::CreateEffect(THIS_ REFGUID rguid, LPCDIEFFECT lpeff, LPDIRECTINPUTEFFECT * ppdeff, LPUNKNOWN punkOuter) {
	syncMultiplayerState();

	// even though we don't really care about effects
	// we need to sync the multiplayer state for them as well
	HRESULT err = originalDirectInputDevice8Pointer->CreateEffect(rguid, lpeff, ppdeff, punkOuter);

	if (err != DI_OK) {
		return err;
	}

	if (!ppdeff || !*ppdeff) {
		return DIERR_INVALIDPARAM;
	}

	NetJetSimulatorDirectInputEffect::DestroyNetJetSimulatorDirectInputDevice8Pointer destroyNetJetSimulatorDirectInputDevice8Pointer = 0;
	NetJetSimulatorDirectInputEffect* netJetSimulatorDirectInputEffectPointer = new NetJetSimulatorDirectInputEffect(*ppdeff, destroyNetJetSimulatorDirectInputDevice8Pointer, this, &NetJetSimulatorDirectInputDevice8::syncMultiplayerState);

	// need to be mindful of which pointers need to be destroyed at this stage
	if (!netJetSimulatorDirectInputEffectPointer) {
		(*ppdeff)->Release();
		*ppdeff = NULL;
		destroyNetJetSimulatorDirectInputDevice8Pointer = 0;
		return DIERR_OUTOFMEMORY;
	}

	if (!destroyNetJetSimulatorDirectInputDevice8Pointer) {
		netJetSimulatorDirectInputEffectPointer->Release();
		netJetSimulatorDirectInputEffectPointer = NULL;
		*ppdeff = NULL;
		return DIERR_OUTOFMEMORY;
	}
	
	*ppdeff = netJetSimulatorDirectInputEffectPointer;

	NET_JET_SIMULATOR_DIRECT_INPUT_DEVICE_8_POINTER_MAP::iterator netJetSimulatorDirectInputDevice8PointerMapIterator = netJetSimulatorDirectInputDevice8PointerMap.find(this);

	// insert this device into the map if it isn't already there
	if (netJetSimulatorDirectInputDevice8PointerMapIterator == netJetSimulatorDirectInputDevice8PointerMap.end()) {
		netJetSimulatorDirectInputDevice8PointerMapIterator = netJetSimulatorDirectInputDevice8PointerMap.insert({ this, {} }).first;
	}

	netJetSimulatorDirectInputDevice8PointerMapIterator->second.insert({ netJetSimulatorDirectInputEffectPointer, destroyNetJetSimulatorDirectInputDevice8Pointer });
	return DI_OK;
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE NetJetSimulatorDirectInputDevice8::EnumCreatedEffectObjects(THIS_ LPDIENUMCREATEDEFFECTOBJECTSCALLBACK lpCallback, LPVOID pvRef, DWORD fl) {
	syncMultiplayerState();
	//return originalDirectInputDevice8Pointer->EnumCreatedEffectObjects(lpCallback, pvRef, fl);

	// we can't use the original call because now we created the effects
	// pvRef may be null, because it is application defined
	if (!lpCallback) {
		return DIERR_INVALIDPARAM;
	}
	
	NET_JET_SIMULATOR_DIRECT_INPUT_DEVICE_8_POINTER_MAP::iterator netJetSimulatorDirectInputDevice8PointerMapIterator = netJetSimulatorDirectInputDevice8PointerMap.find(this);

	if (netJetSimulatorDirectInputDevice8PointerMapIterator == netJetSimulatorDirectInputDevice8PointerMap.end()) {
		return DI_OK;
	}

	NET_JET_SIMULATOR_DIRECT_INPUT_EFFECT_POINTER_MAP &netJetSimulatorDirectInputEffectPointerMap = netJetSimulatorDirectInputDevice8PointerMapIterator->second;

	for (NET_JET_SIMULATOR_DIRECT_INPUT_EFFECT_POINTER_MAP::iterator netJetSimulatorDirectInputEffectPointerMapIterator = netJetSimulatorDirectInputEffectPointerMap.begin(); netJetSimulatorDirectInputEffectPointerMapIterator != netJetSimulatorDirectInputEffectPointerMap.end(); netJetSimulatorDirectInputEffectPointerMapIterator++) {
		if (netJetSimulatorDirectInputEffectPointerMapIterator->first) {
			if (lpCallback(netJetSimulatorDirectInputEffectPointerMapIterator->first, pvRef) == DIENUM_STOP) {
				break;
			}
		}
	}
	return DI_OK;
}

void NetJetSimulatorDirectInputDevice8::syncMultiplayerState() {
	// we don't need to test input device because
	// this will only be true for relevant devices
	if (!multiplayerState) {
		return;
	}

	if (getMultiplayerState) {
		if (getMultiplayerState()) {
			return;
		}
	}

	// it is necessary to sync the multiplayer state before other actions
	// so that if the error returned is DIERR_NOTACQUIRED or DIERR_NOTEXCLUSIVEACQUIRED
	// then the expected result occurs
	HRESULT err = originalDirectInputDevice8Pointer->Unacquire();

	if (err != DI_OK && err != DI_NOEFFECT) {
		return;
	}

	multiplayerState = false;
}