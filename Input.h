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
	ComPtr<IDirectInputDevice8> keyboard;
	void Initialize(HINSTANCE hInstance, HWND hwnd);
	void Update();

private:
	Comptr<IDirectInputDevice8> keyboard;
};

