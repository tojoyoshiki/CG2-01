#include "Input.h"
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")

void Input::Initialize(WinApp* winApp)
{
	//インスタンス生成
	//HRESULT result = DirectInput8Create(
	//	hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8,
	//	(void**)&directInput, nullptr);
	//assert(SUCCEEDED(result));

	////キーボードデバイスの生成
	//result = directInput->CreateDevice(GUID_SysKeyboard,
	//	&devkeyboard, NULL);
	//assert(SUCCEEDED(result));

	//DurectInputインスタンス生成
	result = DirectInput8Create(winApp->GetInstance(), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&dinput, nullptr);

	//キーボードデバイス生成
	result = dinput->CreateDevice(GUID_SysKeyboard, &devkeyboard, NULL);
	//入力データ形式のセット
	result = devkeyboard->SetDataFormat(&c_dfDIKeyboard);
	result = devkeyboard->SetCooperativeLevel(winApp->GetHwnd(), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	this->winApp = winApp;
}

void Input::Update()
{
	HRESULT result;
	memcpy(keyPre, key, sizeof(key));

	result=devkeyboard->Acquire();
	//BYTE key[256] = {};
	result=devkeyboard->GetDeviceState(sizeof(key), key);
}

bool Input::PushKey(BYTE keyNumber)
{
	if (key[keyNumber]) {
		return false;
	}

	return false;
}

bool Input::TriggerKey(BYTE keyNumber)
{

	return false;
}
