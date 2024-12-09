#include "Input.h"
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")

void Input::Initialize(HINSTANCE hInstance, HWND hwnd)
{
	//インスタンス生成
	HRESULT result = DirectInput8Create(
		hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8,
		(void**)&directInput, nullptr);
	assert(SUCCEEDED(result));

	//キーボードデバイスの生成
	result = directInput->CreateDevice(GUID_SysKeyboard,
		&devkeyboard, NULL);
	assert(SUCCEEDED(result));

	// 入力データ形式のセット
		result = devkeyboard->SetDataFormat(&c_dfDIKeyboard);//標準形式
	assert(SUCCEEDED(result));
	//排他制御レベルのセット
	result = devkeyboard->SetCooperativeLevel(
		hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	assert(SUCCEEDED(result));
}

void Input::Update()
{
	HRESULT result;
	memcpy(keyPre, key, sizeof(key));

	//キーボード情報の取得開始
	result=devkeyboard->Acquire();
	//全キーの入力情報を取得する
	result=devkeyboard->GetDeviceState(sizeof(key), key);
}

bool Input::TriggerKey(BYTE keyNumber)
{
	if (key[keyNumber] && !keyPre[keyNumber]) {
		return true;
	}
	return false;
}
