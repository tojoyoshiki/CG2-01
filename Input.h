#pragma once
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <cassert>
#include <Windows.h>
#include <wrl.h>
#include "WinApp.h"
 
#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")

class Input
{
public:
	template <class T>using ComPtr = Microsoft::WRL::ComPtr<T>;
	
	void Initialize(WinApp* winApp);
	void Update();
	
	bool PushKey(BYTE keyNumber);
	bool TriggerKey(BYTE keyNumber);

private:
	//インスタンス生成
	IDirectInput8* directInput = nullptr;
	//キーボードデバイスの生成
	ComPtr<IDirectInputDevice8> devkeyboard;

	WinApp* winApp = nullptr;

	BYTE key[256] = {};
	BYTE keyPre[256] = {};
};

