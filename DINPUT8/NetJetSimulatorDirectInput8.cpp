#include <initguid.h>

#include "shared.h"
#include "NetJetSimulatorDirectInput8.h"
#include "NetJetSimulatorDirectInputDevice8.h"
#include <windows.h>
#include <dinput.h>

NetJetSimulatorDirectInput8::NetJetSimulatorDirectInput8(IDirectInput8* originalDirectInput8Pointer) {
	if (!originalDirectInput8Pointer) {
		throw "originalDirectInput8Pointer must not be NULL";
	}

	this->originalDirectInput8Pointer = originalDirectInput8Pointer;
}

NetJetSimulatorDirectInput8::~NetJetSimulatorDirectInput8() {
	originalDirectInput8Pointer = NULL;
}

/*** IUnknown methods ***/
COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE NetJetSimulatorDirectInput8::QueryInterface(THIS_ REFIID riid, LPVOID * ppvObj) {
	return originalDirectInput8Pointer->QueryInterface(riid, ppvObj);
}

COM_DECLSPEC_NOTHROW ULONG STDMETHODCALLTYPE NetJetSimulatorDirectInput8::AddRef(THIS) {
	return originalDirectInput8Pointer->AddRef();
}

COM_DECLSPEC_NOTHROW ULONG STDMETHODCALLTYPE NetJetSimulatorDirectInput8::Release(THIS) {
	ULONG refCount = originalDirectInput8Pointer->Release();

	// in case no further Ref is there, the Original Object has deleted itself
	// so do we here
	if (!refCount) {
		delete this;
	}
	return refCount;
}

/*** IDirectInput8 methods ***/
COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE NetJetSimulatorDirectInput8::EnumDevices(THIS_ DWORD dwDevType, LPDIENUMDEVICESCALLBACK lpCallback, LPVOID pvRef, DWORD dwFlags) {
	return originalDirectInput8Pointer->EnumDevices(dwDevType, lpCallback, pvRef, dwFlags);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE NetJetSimulatorDirectInput8::GetDeviceStatus(THIS_ REFGUID rguidInstance) {
	return originalDirectInput8Pointer->GetDeviceStatus(rguidInstance);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE NetJetSimulatorDirectInput8::RunControlPanel(THIS_ HWND hwndOwner, DWORD dwFlags) {
	return originalDirectInput8Pointer->RunControlPanel(hwndOwner, dwFlags);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE NetJetSimulatorDirectInput8::Initialize(THIS_ HINSTANCE hinst, DWORD dwVersion) {
	return originalDirectInput8Pointer->Initialize(hinst, dwVersion);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE NetJetSimulatorDirectInput8::FindDevice(THIS_ REFGUID rguidClass, LPCSTR ptszName, LPGUID pguidInstance) {
	return originalDirectInput8Pointer->FindDevice(rguidClass, ptszName, pguidInstance);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE NetJetSimulatorDirectInput8::EnumDevicesBySemantics(THIS_ LPCSTR ptszUserName, LPDIACTIONFORMAT lpdiActionFormat, LPDIENUMDEVICESBYSEMANTICSCB lpCallback, LPVOID pvRef, DWORD dwFlags) {
	return originalDirectInput8Pointer->EnumDevicesBySemantics(ptszUserName, lpdiActionFormat, lpCallback, pvRef, dwFlags);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE NetJetSimulatorDirectInput8::ConfigureDevices(THIS_ LPDICONFIGUREDEVICESCALLBACK lpdiCallback, LPDICONFIGUREDEVICESPARAMS lpdiCDParams, DWORD dwFlags, LPVOID pvRefData) {
	return originalDirectInput8Pointer->ConfigureDevices(lpdiCallback, lpdiCDParams, dwFlags, pvRefData);
}

COM_DECLSPEC_NOTHROW HRESULT STDMETHODCALLTYPE NetJetSimulatorDirectInput8::CreateDevice(THIS_ REFGUID rguid, LPDIRECTINPUTDEVICE8 * lplpDirectInputDevice, LPUNKNOWN pUnkOuter) {
	HRESULT err = originalDirectInput8Pointer->CreateDevice(rguid, lplpDirectInputDevice, pUnkOuter);

	if (err != DI_OK) {
		return err;
	}

	if (!lplpDirectInputDevice || !*lplpDirectInputDevice) {
		return DIERR_INVALIDPARAM;
	}

	const size_t GUID_SIZE = sizeof(GUID);

	INPUT_DEVICE inputDevice = INPUT_DEVICE_JOYSTICK;

	if (memoryEqual(&rguid, &GUID_SysMouse, GUID_SIZE)
		|| memoryEqual(&rguid, &GUID_SysMouseEm, GUID_SIZE)
		|| memoryEqual(&rguid, &GUID_SysMouseEm2, GUID_SIZE)) {
		inputDevice = INPUT_DEVICE_MOUSE;
	} else if (memoryEqual(&rguid, &GUID_SysKeyboard, GUID_SIZE)
		|| memoryEqual(&rguid, &GUID_SysKeyboardEm, GUID_SIZE)
		|| memoryEqual(&rguid, &GUID_SysKeyboardEm2, GUID_SIZE)) {
		inputDevice = INPUT_DEVICE_KEYBOARD;
	}

	// the object will delete itself once Ref count is zero (similar to COM objects)
	NetJetSimulatorDirectInputDevice8* netJetSimulatorDirectInputDevice8Pointer = new NetJetSimulatorDirectInputDevice8(*lplpDirectInputDevice, inputDevice);

	if (!netJetSimulatorDirectInputDevice8Pointer) {
		(*lplpDirectInputDevice)->Release();
		*lplpDirectInputDevice = NULL;
		return DIERR_OUTOFMEMORY;
	}

	*lplpDirectInputDevice = netJetSimulatorDirectInputDevice8Pointer;
	return DI_OK;
}