3D Groove NetJet Emulator 1.1.0
By Anthony Kleine
--------
Long Description
	An emulator for the Hasbro NetJet Controller device meant specifically for use with Transformers Battle Universe. Allows the use of your keyboard/360 Controller in place of the NetJet Controller. If a legitimate NetJet Controller is connected, it will still use that controller. If a 360 Controller is connected, it will still use the NetJet Controller, but prefer input from the 360 Controller. Keyboard input will be preferred over either controller. Button presses are not cumulative (pressing left on both the NetJet Controller and keyboard will not make you run twice as fast in-game.) A game key is still required, this is not a keygen. While one could theoretically remove the Armadillo 4.66 DRM used by NetJet games and they would work without any legitimate NetJet Controller, that is outside the scope of this project.

	This is a proxy DLL to reside in the game directory. It relies on the original NetJetController.DLL being present but renamed to NetJetController_.DLL. SetWindowsHookEx is used to prevent the game from receiving keystrokes. This is to prevent key/button presses being received twice.

	In the future this code may be recycled into a fully fledged application to pair with a NetJet Emulator driver that would give a universal emulation. The driver could reasonably be based off of FileDisk, as it would be required to mount an ISO image or similar. This would require returning a buffer in the NetJet Controller's device language from DeviceIoControl. The NetJet Controller exposes a CDROM interface. It mounts an image containing an AutoRun.exe and autorun.inf file which downloads an installer for the keyhole application for NetJet games via the internet. The Hasbro server for this is no longer available, and an error is given if attempting to connect to it. It does not expose any HID (Human Interface Device) interface. Instead, DeviceIoControl is used to Get Controller State as ReadFile/WriteFile would imply reading from/writing to the disk. Games identify the NetJet Controller by its Vendor ID (VID) and Product ID (PID.)
--------
Maps
	Controller Inserted: Always
	Cartridge Inserted: Always (if legitimate NetJet Controller does not have a key in, a null key is returned)

	Keyboard Map:
	1, Enter, Space     - Button 1
	2, Q    , Backspace - Button 2
	3, E                - Button 3
	4, F                - Button 4
	L, Left Shift       - Left Shoulder
	R, Right Shift      - Right Shoulder
	0, Escape           - Start
	W                   - DPad Up
	A                   - DPad Left
	S                   - DPad Down
	D                   - DPad Up
	Up Arrow Key        - Right Thumbstick Up
	Down Arrow Key      - Right Thumbstick Down
	Left Arrow Key      - Right Thumbstick Left
	Right Arrow Key     - Right Thumbstick Right

	360 Controller Map:
	A                                   - Button 1
	B                                   - Button 2
	X                                   - Button 3
	Y                                   - Button 4
	Left Button , Left Trigger          - Left Shoulder
	Right Button, Right Trigger         - Right Shoulder
	Start                               - Start
	DPad Up     , Left Thumbstick Up    - DPad Up
	DPad Down   , Left Thumbstick Down  - DPad Down
	DPad Left   , Left Thumbstick Left  - DPad Left
	DPad Right  , Left Thumbstick Right - DPad Right
	Right Thumbstick Up                 - Right Thumbstick Up
	Right Thumbstick Down               - Right Thumbstick Down
	Right Thumbstick Left               - Right Thumbstick Left
	Right Thumbstick Right              - Right Thumbstick Right
--------
Device Details
	Friendly Name:
	Net Jet Controller USB Device

	Parent:
	USB\VID_3078&PID_B061\6&85ca75f&0&6

	IoControlCodes:
	4D014h - Get Controller State

	OutBuffer:
		DWORD wButtons
		Cartridge_Inserted Controller_Inserted "Right Shoulder" "Left Shoulder" "Button One" "Button Three" "Button Two" "Button Four"
		Mouse_Map??        Unknown             Unknown          "DPad Right"    "DPad Left"  "DPad Down"    "DPad Up"    "Start"

		BYTE bThumbRX
		Right Analog Stick Left to Right

		BYTE bThumbRY
		Right Analog Stick Up to Down

		BYTE controllerKey[20]
		Controller Key

		BYTE cartridgeKey[20]
		Cartridge Key

	Messages:
	401h - WM_CONTROLLER_INSERTED
	402h - WM_CONTROLLER_REMOVED
	403h - WM_CARTRIDGE_INSERTED
	404h - WM_CARTRIDGE_REMOVED
	405h - WM_CONTROLLER_INVALID