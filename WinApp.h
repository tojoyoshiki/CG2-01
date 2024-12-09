#pragma once
#include <Windows.h>
#include <cstdint>
#include <winnt.h>

class WinApp
{
public://静的メンバ関数
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg,
		WPARAM wparam, LPARAM lparam);

	//クライアント領域のサイズ
	static const int32_t kClientWidth = 1280;
	static const int32_t kClientHeight = 720;

public://メンバ関数
	void Initialize();
	void Update();

	//終了
	void Finalize();
	
	//メッセージの終了
	bool ProcessMessage();
	
	//getter
	HWND GetHwnd() const { return hwnd; }
	HINSTANCE GetHInstance()const { return wc.hInstance; }

private:
	//ウインドウハンドル
	HWND hwnd = nullptr;
	//ウインドウクラスの設定
	WNDCLASS wc{};
};

