#include <Windows.h>
#include <cstdint>
#include <string>
#include <format>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <cassert>
#include <dxgidebug.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"dxguid.lib")

std::wstring ConvertString(const std::string& str) {
	if (str.empty()) {
		return std::wstring();
	}

	auto sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(&str[0]), static_cast<int>(str.size()), NULL, 0);
	if (sizeNeeded == 0) {
		return std::wstring();
	}
	std::wstring result(sizeNeeded, 0);
	MultiByteToWideChar(CP_UTF8, 0, reinterpret_cast<const char*>(&str[0]), static_cast<int>(str.size()), &result[0], sizeNeeded);
	return result;
}

std::string ConvertString(const std::wstring& str) {
	if (str.empty()) {
		return std::string();
	}

	auto sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), NULL, 0, NULL, NULL);
	if (sizeNeeded == 0) {
		return std::string();
	}
	std::string result(sizeNeeded, 0);
	WideCharToMultiByte(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), result.data(), sizeNeeded, NULL, NULL);
	return result;
}

void Log(const std::string& messege) {
	OutputDebugStringA(messege.c_str());
}

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

#pragma region Windowの生成

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

#pragma endregion

#pragma region DebugLayer
#ifdef _DEBUG
	ID3D12Debug1* debugController = nullptr;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
		debugController->EnableDebugLayer();

		debugController->SetEnableGPUBasedValidation(TRUE);
	}
#endif // DEBUG
#pragma endregion 

#pragma region DXGIFactory
	//DXGIファクトリーの生成
	IDXGIFactory7* dxgiFactory = nullptr;
	HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));
	assert(SUCCEEDED(hr));
#pragma endregion

#pragma region IDXGIAdaptor
	//アダプタ変数
	IDXGIAdapter4* useAdapter = nullptr;
	//性能の良いアダプタの要求
	for (UINT i = 0; dxgiFactory->EnumAdapterByGpuPreference(i,
		DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&useAdapter)) !=
		DXGI_ERROR_NOT_FOUND; i++) {
		//アダプタの情報を取得する
		DXGI_ADAPTER_DESC3 adapterDesc{};
		assert(SUCCEEDED(hr));//取得の確認
		//ソフトウェアアダプタでないことを確認
		if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)) {
			//アダプタの情報をログに出力
			Log(ConvertString(std::format(L"Use Adapter:{}\n", adapterDesc.Description)));
			break;
		}
		useAdapter = nullptr;
	}
	//適切なアダプタが見つからなかった場合起動できない
	assert(useAdapter != nullptr);
#pragma endregion

#pragma region D3D12Deviceの生成
	ID3D12Device* device = nullptr;
	//機械レベルとログ出力用の文字列
	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_12_2,D3D_FEATURE_LEVEL_12_1,D3D_FEATURE_LEVEL_12_0 };
	const char* featureLevelStrings[] = { "12.2","12.1","12.0" };
	//高い順に生成できるか試していく
	for(size_t i=0;i<_countof(featureLevels);++i){
		//採用したアダプタでデバイス生成
		hr = D3D12CreateDevice(useAdapter, featureLevels[i], IID_PPV_ARGS(&device));
		if (SUCCEEDED(hr)) {
			//生成出来たらログ出力を行ってループを抜ける
			Log(std::format("FeatureLevel : {}\n", featureLevelStrings[i]));
			break;
		}
	}
	//デバイスの生成が失敗した場合起動できない
	assert(device != nullptr);
	//初期化完了のログを出力
	Log("Complete create D3D12Device!!!\n");
#pragma endregion

#pragma region Error
#ifdef DEBUG
	ID3D12InfoQueue* infoQueue = nullptr;
	if (SUCCEEDED(device->QueryInterface(IID_PPV_ARGS(&infoQueue)))) {
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTTION, true);
		infoQueue->SetBreakOnServerity(D3D12_MESSAGE_SEVERRITY_ERROR, true);
		//infoQueue->SetBreakOnServerity(D3D12_MESSAGE_SEVERRITY_WARNING, true);

		D3D12_MESSAGE_ID denyIds[] = {
		D3D12_MESSAGE_ID_RESOURECE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE
		};
		D3D12_MESSAGE_SERVERTY severities[] = { D3D12_MESSAGE_SEVERITY_INFO };
		D3D12_INFO_QUEUE_FILTER filter{};
		filter.DentList.NumIDs = _countof(denyIds);
		filter.DentList.pIDList = denyIds;
		filter.DentList.NumSeverrities = _countof(severities);
		filter.DentList.pSeverityList = severities;
		infoQueue->PushStorageFilter(&filter);

		infoQueue->Release();
	}
#endif // DEBUG
#pragma endregion

#pragma region CommandAllocator
	//コマンドキューを生成
	ID3D12CommandQueue* commandQueue = nullptr;
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
	hr = device->CreateCommandQueue(&commandQueueDesc,
		IID_PPV_ARGS(&commandQueue));
	//生成が失敗
	assert(SUCCEEDED(hr));

	//コマンドアロケータを生成
	ID3D12CommandAllocator* commandAllocator = nullptr;
	hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
	//生成が失敗
	assert(SUCCEEDED(hr));
#pragma endregion

#pragma region CommandList
	//コマンドリストを生成
	ID3D12GraphicsCommandList* commandList = nullptr;
	hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator, nullptr,
		IID_PPV_ARGS(&commandList));
	//生成が失敗
	assert(SUCCEEDED(hr));
#pragma endregion

#pragma region SwapChain
	//スワップチェーンを生成
	IDXGISwapChain4* swapChain = nullptr;
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	//画面の幅、ウインドウのクライアント領域をおなしものにしておく
	swapChainDesc.Width = kClientWidth;
	//画面の高さ、ウインドウのクライアント領域をおなしものにしておく
	swapChainDesc.Height = kClientHeight;
	//色の形式
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	//マルチサンプルしない
	swapChainDesc.SampleDesc.Count = 1;
	//描画のターゲットとして使用
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	//ダブルバッファ
	swapChainDesc.BufferCount = 2;
	//モニターに移したら中身を破棄
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	//コマンドキュー、ウインドウハンドル、設定を渡して生成する
	hr = dxgiFactory->CreateSwapChainForHwnd(commandQueue, hwnd, &swapChainDesc, nullptr, nullptr,
		reinterpret_cast<IDXGISwapChain1**>(&swapChain));
	//生成が失敗
	assert(SUCCEEDED(hr));
#pragma endregion

#pragma region DescripterHeap
	//ディスクリプタヒープの生成
	ID3D12DescriptorHeap* rtvDescriptorHeap = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC rtvDescriptorHeapDesc{};
	//レンダーターゲットビュー用
	rtvDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	//ダブルバッファ用に２こ
	rtvDescriptorHeapDesc.NumDescriptors = 2;
	hr = device->CreateDescriptorHeap(&rtvDescriptorHeapDesc, IID_PPV_ARGS(&rtvDescriptorHeap));
	//生成が失敗
	assert(SUCCEEDED(hr));
#pragma endregion

#pragma region Resources
	//swapChainからResourcesを引っ張ってくる
	ID3D12Resource* swapChainResources[2] = { nullptr };
	hr = swapChain->GetBuffer(0, IID_PPV_ARGS(&swapChainResources[0]));
	//生成が失敗
	assert(SUCCEEDED(hr));
	hr = swapChain->GetBuffer(1, IID_PPV_ARGS(&swapChainResources[1]));
	//生成が失敗
	assert(SUCCEEDED(hr));
#pragma endregion

#pragma region RTVの生成
	//RTVの生成
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	//出力結果をSRGBに変換して書き込む
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	//出力結果を２ｄテクスチャとして書き込む
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	//ディスクリプタの先頭を取得する
	D3D12_CPU_DESCRIPTOR_HANDLE rtvStartHandle = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	//RTVを２つ取得するのでディスクリプタ２つ用意する
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2];
	//１つ目。
	rtvHandles[0] = rtvStartHandle;
	device->CreateRenderTargetView(swapChainResources[0], &rtvDesc, rtvHandles[0]);
	//２つ目。
	rtvHandles[1].ptr = rtvHandles[0].ptr + device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	device->CreateRenderTargetView(swapChainResources[1], &rtvDesc, rtvHandles[1]);
#pragma endregion 

#pragma region Fence
	ID3D12Fence* fence = nullptr;
	uint64_t fenceValue = 0;
	hr = device->CreateFence(fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
	assert(SUCCEEDED(hr));

	HANDLE fenceEvent = CreateEvent(NULL, FALSE , FALSE , NULL);
	assert(fenceEvent != nullptr);
#pragma endregion

	MSG msg{};
	//ウインドウの×ボタンが押されるまでループ
	while (msg.message != WM_QUIT) {
		//Windowにメッセージが来てたら最優先で処理
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			// ゲーム処理ここから

			//バックバッファのインデックスを取得
			UINT backBufferIndex = swapChain->GetCurrentBackBufferIndex();
#pragma region TransitionBarrier
			//トランジションバリア
			D3D12_RESOURCE_BARRIER barrier{};
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			barrier.Transition.pResource = swapChainResources[backBufferIndex];
			barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
			barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
			commandList->ResourceBarrier(1, &barrier);
#pragma endregion
			//描画先のRTVを設定
			commandList->OMSetRenderTargets(1, &rtvHandles[backBufferIndex], false, nullptr);
			//指定した色で画面全体をクリアする
			float clearColor[] = { 0.1f,0.25f,0.5f,1.0f };
			commandList->ClearRenderTargetView(rtvHandles[backBufferIndex], clearColor, 0, nullptr);
			//RenderTargetからPresentに遷移
			barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
			barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
			//バリア生成
			commandList->ResourceBarrier(1, &barrier);
			//コマンドリストの内容を確定させる
			hr = commandList->Close();
			assert(SUCCEEDED(hr));
			//GPUにコマンドリストの実行を行わせる
			ID3D12CommandList* commandLists[] = { commandList };
			commandQueue->ExecuteCommandLists(1, commandLists);
			//GPUとOSに画面の効果案を行うように通知する
			swapChain->Present(1, 0);
			//Fenceの値を更新
			fenceValue++;
			commandQueue->Signal(fence, fenceValue);
			if (fence->GetCompletedValue() < fenceValue) {
				fence->SetEventOnCompletion(fenceValue, fenceEvent);
				WaitForSingleObject(fenceEvent, INFINITE);
			}
			//次のフレーム用のコマンドリストを準備
			hr = commandAllocator->Reset();
			assert(SUCCEEDED(hr));
			hr = commandList->Reset(commandAllocator, nullptr);
			assert(SUCCEEDED(hr));

		}
	}

	//出力ウインドウへの文字出力
	OutputDebugStringA("Hello,DirectX!\n");

	CloseHandle(fenceEvent);
	fence->Release();
	rtvDescriptorHeap->Release();
	swapChainResources[0]->Release();
	swapChainResources[1]->Release();
	swapChain->Release();
	commandList->Release();
	commandAllocator->Release();
	commandQueue->Release();
	device->Release();
	useAdapter->Release();
	dxgiFactory->Release();
#ifdef DEBUG
	debugController->Release();
#endif // DEBUG
	CloseWindow(hwnd);

	//リソースリークチェック
	IDXGIDebug1* debug;
	if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debug)))) {
		debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
		debug->ReportLiveObjects(DXGI_DEBUG_APP, DXGI_DEBUG_RLO_ALL);
		debug->ReportLiveObjects(DXGI_DEBUG_D3D12, DXGI_DEBUG_RLO_ALL);
		debug->Release();
	}

	return 0;
}