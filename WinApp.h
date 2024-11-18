#pragma once
#include <Windows.h>
#include <cstdint>
#include <winnt.h>

class WinApp
{
public://静的メンバ関数
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg,
		WPARAM wparam, LPARAM lparam);

	static const int32_t kClientWidth = 1280;
	static const int32_t kClientHeight = 720;

public://メンバ関数
	void Initialize();
	void Update();
};

