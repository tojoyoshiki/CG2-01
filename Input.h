#pragma once
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <cassert>
#include <Windows.h>
#include <wrl.h>
 
#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")

class Input
{
public:
	template <class T>using ComPtr = Microsoft::WRL::ComPtr<T>;
	
	void Initialize(HINSTANCE hInstance, HWND hwnd);
	void Update();
	
	bool PushKey(BYTE keyNumber)
	{
		if (key[keyNumber]) {
			return true;
		}

		return false;
	}
	bool TrigerKey(BYTE keyNumber);

private:
	//インスタンス生成
	IDirectInput8* directInput = nullptr;
	//キーボードデバイスの生成
	ComPtr<IDirectInputDevice8> devkeyboard;

	BYTE key[256] = {};
	BYTE keyPre[256] = {};
};

