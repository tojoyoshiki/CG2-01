#include "Input.h"
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")

void Input::Initialize(HINSTANCE hInstance, HWND hwnd)
{
	//インスタンス生成
	IDirectInput8* directInput = nullptr;
	HRESULT result = DirectInput8Create(
		hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8,
		(void**)&directInput, nullptr);
	assert(SUCCEEDED(result));
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
