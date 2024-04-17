#include <Windows.h>
#include <cstdint>

//ウインドウプロシージャ
LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg,
	WPARAM wparam, LPARAM lparam) {

	//メッセージに応じてゲーム画面の処理を行う
	switch (msg) {
	case WM_DESTROY:
		//OSに対してアプリ終了を伝える
		PostQuitMessage(0);

		return 0;
	}

	//メッセージ処理
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

//Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	WNDCLASS wc{};
	//ウインドウプロシージャ
	wc.lpfnWndProc = WindowProc;
	//ウインドウクラス名
	wc.lpszClassName = L"CG2WindowClass";
	//インスタンスハンドル
	wc.hInstance = GetModuleHandle(nullptr);
	//カーソル
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

	//ウインドウクラス登録
	RegisterClass(&wc);

	//クライアント領域のサイズ
	const int32_t kClientWidth = 1280;
	const int32_t kClientHeight = 1280;

	//ウインドウサイズを表す構造体にクライアント領域を入れる
	RECT wrc = { 0,0,kClientWidth,kClientHeight };

	//クライアント領域をもとに実際のサイズにwrcを変更させる
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	//ウインドウの生成
	HWND hwnd = CreateWindow(
		wc.lpszClassName, //利用するクラス名
		L"CG2_LE2C_19_トウジョウ_ヨシキ", //タイトルバーの文字
		WS_OVERLAPPEDWINDOW,              //ウインドウスタイル
		CW_USEDEFAULT,                    //表示ｘ座標
		CW_USEDEFAULT,                    //表示y座標
		wrc.right - wrc.left,             //横幅
		wrc.bottom - wrc.top,             //立幅
		nullptr,                          //親ウインドウハンドル
		nullptr,                          //メニューハンドル
		wc.hInstance,                     //インスタンスハンドル
		nullptr);                         //オプション

	//ウインドウ表示
	ShowWindow(hwnd, SW_SHOW);

	MSG msg{};
	//ウインドウの×ボタンが押されるまでループ
	while (msg.message != WM_QUIT) {
		//Windowにメッセージが来てたら最優先で処理
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			//ゲーム処理ここから
		}
	}

	//出力ウインドウへの文字出力
	OutputDebugStringA("Hello,DirectX!\n");

	return 0;
}