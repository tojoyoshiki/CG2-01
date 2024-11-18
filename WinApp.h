#pragma once
#include <Windows.h>
#include <cstdint>
#include <winnt.h>

class WinApp
{
public://静的メンバ関数
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg,
		WPARAM wparam, LPARAM lparam);

public://メンバ関数
	void Initialize();
	void Update();
};

