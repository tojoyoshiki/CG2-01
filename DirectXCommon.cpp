#include "DirectXCommon.h"
#include <cassert>
#include "externals/DirectXTex/d3dx12.h"

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

using namespace Microsoft::WRL;

//ディスクリプタヒープ生成関数
Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> DirectXCommon::CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible)
{
	//ディスクリプタヒープの生成
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
	descriptorHeapDesc.Type = heapType;	//レンダーターゲットビュー用
	descriptorHeapDesc.NumDescriptors = numDescriptors;
	descriptorHeapDesc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;	//ダブルバッファ用に2つ。多くても構わない
	HRESULT hr = device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&descriptorHeap));
	//ディスクリプタヒープが作れなかったので起動できない
	assert(SUCCEEDED(hr));

	return descriptorHeap;
}
//ディスクリプタハンドル取得関数
D3D12_CPU_DESCRIPTOR_HANDLE DirectXCommon::GetCPUDescriptorHandle(ID3D12DescriptorHeap* descriptorHeap, uint32_t descriptorSize, uint32_t index)
{
	D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
	handleCPU.ptr += (descriptorSize * index);
	return handleCPU;
}
//ディスクリプタハンドル取得関数
D3D12_GPU_DESCRIPTOR_HANDLE DirectXCommon::GetGPUDescriptorHandle(ID3D12DescriptorHeap* descriptorHeap, uint32_t descriptorSize, uint32_t index)
{
	D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = descriptorHeap->GetGPUDescriptorHandleForHeapStart();
	handleGPU.ptr += (descriptorSize * index);
	return handleGPU;
}
//深度ステンシルテクスチャ作成
Microsoft::WRL::ComPtr<ID3D12Resource> DirectXCommon::CreateDepthStencilTextureResource(ID3D12Device* device, int width, int height, DXGI_FORMAT format)
{
	D3D12_RESOURCE_DESC depthStencilDesc = {};
	depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthStencilDesc.Width = width;
	depthStencilDesc.Height = height;
	depthStencilDesc.DepthOrArraySize = 1;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.Format = format;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE clearValue = {};
	clearValue.Format = format;
	clearValue.DepthStencil.Depth = 1.0f;
	clearValue.DepthStencil.Stencil = 0;

	Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilBuffer;
	HRESULT hr = device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&depthStencilDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&clearValue,
		IID_PPV_ARGS(&depthStencilBuffer));

	assert(SUCCEEDED(hr));

	return depthStencilBuffer;
}

Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> DirectXCommon::CreateDescriptorHeap(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible)
{
	//ディスクリプタヒープの生成
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
	descriptorHeapDesc.Type = heapType;	//レンダーターゲットビュー用
	descriptorHeapDesc.NumDescriptors = numDescriptors;
	descriptorHeapDesc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;	//ダブルバッファ用に2つ。多くても構わない
	HRESULT hr = device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&descriptorHeap));
	//ディスクリプタヒープが作れなかったので起動できない
	assert(SUCCEEDED(hr));

}
//コンバート
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

void DirectXCommon::Initialize(WinApp* winApp)
{
	//NULL検出
	assert(winApp);
	//メンバ変数に記憶
	this->winApp = winApp;
}

void DirectXCommon::InitializeDevice()
{
	//HRESULT hr;

#ifdef _DEBUG

	Microsoft::WRL::ComPtr<ID3D12Debug1> debugController = nullptr;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
		//デバッグレイヤーを有効化する
		debugController->EnableDebugLayer();
		//さらにGPU側でもチェックを行うようにする
		debugController->SetEnableGPUBasedValidation(TRUE);
	}

#endif

#pragma region ファクトリーの生成
	//DXGIファクトリーの生成
	Microsoft::WRL::ComPtr<IDXGIFactory7>dxgiFactory = nullptr;

	//"HRESULTはWindows系のエラーコード"であり、
	//関数が成功したかどうかを SUCCEEDEDマクロ で判定できる	
	HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));

	//初期化の根本的な部分でエラーが出た場合はプログラムが間違っているか、
	//どうにも出来ない場合が多いので assert にしておく
	assert(SUCCEEDED(hr));

#pragma endregion

#pragma region アダプタの作成
	//使用するアダプタ用の変数。最初に nullptr を入れておく
	Microsoft::WRL::ComPtr<IDXGIAdapter4> useAdapter = nullptr;
	//良い順にアダプタを頼む
	for (UINT i = 0; dxgiFactory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&useAdapter)) != DXGI_ERROR_NOT_FOUND; ++i) {
		//アダプターの情報を取得する
		DXGI_ADAPTER_DESC3 adapterDesc{};
		hr = useAdapter->GetDesc3(&adapterDesc);
		assert(SUCCEEDED(hr));//取得できないのは一大事

		//ソフトウェアアダプタでなければ採用
		if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)) {
			//採用したアダプタの情報をログに出力。wstring の方なので注意
			Logger::Log(StringUtility::ConvertString(std::format(L"Use Adapter:{}\n", adapterDesc.Description)));
			break;
		}
		useAdapter = nullptr;//ソフトウェアアダプタの場合は見なかったことにする
	}
	//適切なアダプタが見つからなかったので起動できない
	assert(useAdapter != nullptr);

#pragma endregion

#pragma region デバイスの作成
	Microsoft::WRL::ComPtr<ID3D12Device> device = nullptr;
	//機能レベルとログ出力用の文字列
	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_12_2,D3D_FEATURE_LEVEL_12_1,D3D_FEATURE_LEVEL_12_0
	};

	const char* featureLevelStrings[] = { "12.2","12.1","12.0" };
	//高い順に生成できるか試していく
	for (size_t i = 0; _countof(featureLevels); ++i) {
		//採用したアダプターでデバイスを生成
		hr = D3D12CreateDevice(useAdapter.Get(), featureLevels[i], IID_PPV_ARGS(&device));
		//指定した機能レベルでデバイスが生成できたかを確認
		if (SUCCEEDED(hr)) {
			//生成できたのでログ出力を行って
			Logger::Log(std::format("FeatureLevel : {}\n", featureLevelStrings[i]));
			break;
		}
	}
	//デバイスの生成がうまくいかなかったので起動できない
	assert(device != nullptr);
	//初期化完了のログを出す
	Logger::Log("Complet create D3D12Device!!!\n");
#pragma endregion

#ifdef _DEBUG

	Microsoft::WRL::ComPtr<ID3D12InfoQueue> infoQueue = nullptr;
	if (SUCCEEDED(device->QueryInterface(IID_PPV_ARGS(&infoQueue)))) {
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
		D3D12_MESSAGE_ID denyIds[] = { D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE };
		D3D12_MESSAGE_SEVERITY severities[] = { D3D12_MESSAGE_SEVERITY_INFO };
		D3D12_INFO_QUEUE_FILTER filter{};
		filter.DenyList.NumIDs = _countof(denyIds);
		filter.DenyList.pIDList = denyIds;
		filter.DenyList.NumSeverities = _countof(severities);
		filter.DenyList.pSeverityList = severities;
		infoQueue->PushStorageFilter(&filter);
	}
#endif
}

void DirectXCommon::CommandInitialize()
{
#pragma region CommandQueue
	//コマンドキューを生成する
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue = nullptr;
	D3D12_COMMAND_QUEUE_DESC CommandQueueDesc{};
	hr = device->CreateCommandQueue(&CommandQueueDesc, IID_PPV_ARGS(&commandQueue));
	//コマンドキューの生成がうまくいかなかったので起動出来ない
	assert(SUCCEEDED(hr));

#pragma endregion

#pragma region commandAllocator
	//コマンドアロケーターを生成する
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator = nullptr;
	hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
	//コマンドアロケーターの生成がうまくいかなかったので起動出来ない
	assert(SUCCEEDED(hr));

#pragma endregion

#pragma region commandList

	//コマンドリストを生成する
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList = nullptr;
	hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList));
	//コマンドリストの生成がうまくいかなかったので起動出来ない
	assert(SUCCEEDED(hr));

#pragma endregion
}

void DirectXCommon::CreateSwapChain()
{
#pragma region スワップチェーンの生成
	//スワップチェーンを生成する
	Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain = nullptr;
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	swapChainDesc.Width = WinApp::kClientWidth;	//画面の幅。ウィンドウのクライアント領域を同じものにしておく
	swapChainDesc.Height = WinApp::kClientHeight;//画面の高さ。ウィンドウのクライアント領域を同じものにしておく
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;	//色の形式
	swapChainDesc.SampleDesc.Count = 1;	//マルチサンプルしない
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;	//描画のターゲットとして利用する
	swapChainDesc.BufferCount = 2;	//ダブルバッファ
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;	//モニタに写したら、中身を破棄
	//コマンドキュー、ウィンドウハンドル、設定を渡して生成する	
	hr = dxgiFactory->CreateSwapChainForHwnd(
		commandQueue.Get(),
		winApp->GetHwnd(), &swapChainDesc, nullptr, nullptr,
		reinterpret_cast<IDXGISwapChain1**>(swapChain.GetAddressOf()));
	assert(SUCCEEDED(hr));

#pragma endregion
}

void DirectXCommon::CreateDepthBuffer()
{
	//震度バッファリソース設定
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateDepthStencilTextureResource(ID3D12Device * device, int32_t width, int32_t height) {
		//生成するResourceの設定
		D3D12_RESOURCE_DESC resourceDesc{};
		resourceDesc.Width = width;//Textureの幅
		resourceDesc.Height = height;//Textureの高さ
		resourceDesc.MipLevels = 1;//MipMapの数
		resourceDesc.DepthOrArraySize = 1;//奥行き or 配列Textureの配列数
		resourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;//DepthStencilとして利用可能なフォーマット
		resourceDesc.SampleDesc.Count = 1;//サンプリングカウント。1固定
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;//2次元
		resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;//DepthStencilとして使う通知

		//利用するHeapの設定
		D3D12_HEAP_PROPERTIES heapProperties{};
		heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;//VRAM上に作る

		//深度地のクリア設定
		D3D12_CLEAR_VALUE depthClearValue{};
		depthClearValue.DepthStencil.Depth = 1.0f;//1.0f( 最大値 )でクリア
		depthClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;//フォーマット。Resourceと合わせる。

		//Resourceの生成
		Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
		HRESULT hr = device->CreateCommittedResource(
			&heapProperties,//Heapの設定
			D3D12_HEAP_FLAG_NONE,//Heapの特殊な設定。特になし
			&resourceDesc,//Resourceの設定
			D3D12_RESOURCE_STATE_DEPTH_WRITE,//深度地を書き込む状態にしておく
			&depthClearValue,//Clear最適値
			IID_PPV_ARGS(&resource)//作成するResourceポインタへのポインタ
		);
		assert(SUCCEEDED(hr));

		return resource;
	}

#pragma region 利用するヒープの作成
	//利用するヒープの設定

	//DSVの設定
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;//Format。基本的にはResourceに合わせる
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	device->CreateDepthStencilView(depthStencilResource.Get(), &dsvDesc, dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	//出力ウィンドウへの文字出力ループを抜ける
	Logger::Log("Hello,DirectX!\n");
#pragma endregion

#pragma region 深度地のクリア設定
	//指定した色で画面全体をクリアする
	float clearColor[] = { 0.1f,0.25f,0.5f,1.0f };
	ID3D12GraphicsCommandList* cmdList = GetCommandList();
	if (cmdList) {
		cmdList->ClearRenderTargetView(rtvHandles[backBufferIndex], clearColor, 0, nullptr);
	}
	else {
		throw std::runtime_error("Command list is null.");
	}//GetCommandList()->ClearRenderTargetView(rtvHandles[backBufferIndex], clearColor, 0, nullptr);

	//指定した深度で画面全体をクリアする
	commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	//描画用のDescriptorHeapの設定
	ID3D12DescriptorHeap* descriptorHeaps[] = { srvDescriptorHeap.Get() };
	commandList->SetDescriptorHeaps(1, descriptorHeaps);
#pragma endregion

#pragma region 震度バッファ生成

#pragma endregion

#pragma region リソースの生成

#pragma endregion
}


Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible) {
	//ディスクリプタヒープの生成
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
	descriptorHeapDesc.Type = heapType;	//レンダーターゲットビュー用
	descriptorHeapDesc.NumDescriptors = numDescriptors;
	descriptorHeapDesc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;	//ダブルバッファ用に2つ。多くても構わない
	HRESULT hr = device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&descriptorHeap));
	//ディスクリプタヒープが作れなかったので起動できない
	assert(SUCCEEDED(hr));

	return descriptorHeap;
}

void DirectXCommon::CreateDescriptorHeaps()
{
#pragma region RTV用のDescriptorSizeを取得しておく
	const uint32_t descriptorSizeRTV =
		device->GetDescriptorHandleIncrementSize(
			D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
#pragma endregion

#pragma region SRV用のDescriptorSizeを取得しておく
	const uint32_t descriptorSizeSRV =
		device->GetDescriptorHandleIncrementSize(
			D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
#pragma endregion

#pragma region DSV用のDescriptorSizeを取得しておく

	const uint32_t descriptorSizeDSV =
		device->GetDescriptorHandleIncrementSize(
			D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

#pragma endregion

#pragma region ディスクリプタヒープの生成

	//RTV用のヒープでディスクリプタの数は2。RTVはShader内で触るものではないので、ShaderVisibleはfalse
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 2, false);
	//SRV用のヒープでディスクリプタの数は128。SRVはShader内で触るものなので、ShaderVisibleはtrue

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvDescriptorHeap = CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 128, true);

#pragma endregion

#pragma region RTV用のディスクリプタヒープ生成

#pragma endregion
}

void DirectXCommon::RenderTargetViewInitialize()
{
#pragma region SwapChainからResourceを引っ張てくる

	//SwapChainからResourceを引っ張てくる
	Microsoft::WRL::ComPtr<ID3D12Resource> swapChainResource[2] = { nullptr };
	hr = swapChain->GetBuffer(0, IID_PPV_ARGS(&swapChainResource[0]));
	//うまく取得できなければ起動できない
	assert(SUCCEEDED(hr));
	hr = swapChain->GetBuffer(1, IID_PPV_ARGS(&swapChainResource[1]));
	assert(SUCCEEDED(hr));

#pragma endregion

#pragma region RTV用の設定、RTVハンドルの要素数を２個に変更する
	//RTVの設定
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;	//出力結果をSRGBに変換して書き込む
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;	//2dテスクチャとして書き込む
	//ディスクリプタの先頭を取得する
	D3D12_CPU_DESCRIPTOR_HANDLE rtvStartHandle = GetCPUDescriptorHandle(rtvDescriptorHeap.Get(), descriptorSizeRTV, 0);
	//RTVを2つ作るのでディスクリプタを2つ用意
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2];
	//まず1つ目を作る。1つ目は最初のところに作る。作る場所をこちらで指定してあげる必要がある
	rtvHandles[0] = rtvStartHandle;

	//2つ目のディスクリプタハンドルを得る( 自力で )
	rtvHandles[1].ptr = rtvHandles[0].ptr + device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);


#pragma endregion

	for (uint32_t i = 0; i < 2 /*RTVハンドルの要素数*/; ++i) {
		rtvHandles[i];
		device->CreateRenderTargetView(swapChainResource[0].Get(), &rtvDesc, rtvHandles[0]);
		//2つ目を作る
		device->CreateRenderTargetView(swapChainResource[1].Get(), &rtvDesc, rtvHandles[1]);
	}
}

void DirectXCommon::DepthStencilViewInitialize()
{
	//DepthStencilTextureをウィンドウのサイズで作成
	Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResource = CreateDepthStencilTextureResource(device.Get(), WinApp::kClientWidth, WinApp::kClientHeight);


	//描画先のRTVとDSVを設定する
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
}

void DirectXCommon::CreateFence()
{
	Microsoft::WRL::ComPtr<ID3D12Fence> fence = nullptr;
	uint64_t fenceValue = 0;
	hr = device->CreateFence(fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
	assert(SUCCEEDED(hr));

	HANDLE fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	assert(fenceEvent != nullptr);
}

void DirectXCommon::ViewportInitialize()
{
	D3D12_VIEWPORT viewport{};
	viewport.Width = WinApp::kClientWidth;
	viewport.Height = WinApp::kClientHeight;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
}

void DirectXCommon::ScissorRectInitialize()
{
	D3D12_RECT scissorRect{};
	scissorRect.left = 0;
	scissorRect.right = WinApp::kClientWidth;
	scissorRect.top = 0;
	scissorRect.bottom = WinApp::kClientHeight;
}

void DirectXCommon::CreateDXCCompiler()
{
	IDxcUtils* dxcUtils = nullptr;
	hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils));
	assert(SUCCEEDED(hr));

	IDxcCompiler3* dxcCompiler = nullptr;
	hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler));
	assert(SUCCEEDED(hr));

	IDxcIncludeHandler* includeHandler = nullptr;
	hr = dxcUtils->CreateDefaultIncludeHandler(&includeHandler);
	assert(SUCCEEDED(hr));
}

void DirectXCommon::ImGuiInitialize()
{
	//Imguiの初期化
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(winApp->GetHwnd());
	ImGui_ImplDX12_Init(device.Get(),
		swapChainDesc.BufferCount,
		rtvDesc.Format,
		srvDescriptorHeap.Get(),
		srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
		srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart()
	);

	//ゲームの処理
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	//色を変えるImGuiの処理
	ImGui::Begin("Setting");
	// ColorEdit3を使用して色を選択
	ImGui::DragFloat3("CameraTransform", &cameraTransform.translate.x, 0.01f);
	ImGui::DragFloat3("cameraRotate", &cameraTransform.rotate.x, 0.01f);
	ImGui::SliderAngle("RotateX", &transform.rotate.x);
	ImGui::SliderAngle("RotateY", &transform.rotate.y);
	ImGui::SliderAngle("RotateZ", &transform.rotate.z);
	ImGui::Checkbox("useMonsterBall", &useMonsterBall);
	ImGui::SliderInt("Light", &materialData->enableLighting, 0, 1);
	ImGui::SliderFloat3("LightDirector", &directionalLightData->direction.x, -1.0f, 1.0f);
	ImGui::DragFloat2("UVTranslate", &uvTransformSprite.translate.x, 0.01f, -10.0f, 10.0f);
	ImGui::DragFloat2("UVScale", &uvTransformSprite.scale.x, 0.01f, -10.0f, 10.0f);
	ImGui::SliderAngle("UVRotate", &uvTransformSprite.rotate.z);
	ImGui::End();

	//ImGuiの内部コマンドを生成する
	ImGui::Render();
}

void DirectXCommon::LoadTexture(const std::string& filePath)
{
	//テクスチャファイルを読んでプログラムを扱えるようにする
	DirectX::ScratchImage image{};
	std::wstring filePathW = ConvertString(filePath);
	HRESULT hr = DirectX::LoadFromWICFile(filePathW.c_str(), dxCommon->WIC_FLAGS_FORCE_SRGB, nullptr, image);
	assert(SUCCEEDED(hr));

	//MipMapの作成
	DirectX::ScratchImage mipImages{};
	hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);
	assert(SUCCEEDED(hr));

	//MipMap付きのデータを返す
	return mipImages;
}

void DirectXCommon::PreDraw()
{

	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	//色を変えるImGuiの処理
	ImGui::Begin("Setting");
	// ColorEdit3を使用して色を選択
	ImGui::DragFloat3("CameraTransform", &cameraTransform.translate.x, 0.01f);
	ImGui::DragFloat3("cameraRotate", &cameraTransform.rotate.x, 0.01f);
	ImGui::SliderAngle("RotateX", &transform.rotate.x);
	ImGui::SliderAngle("RotateY", &transform.rotate.y);
	ImGui::SliderAngle("RotateZ", &transform.rotate.z);
	ImGui::Checkbox("useMonsterBall", &useMonsterBall);
	ImGui::SliderInt("Light", &materialData->enableLighting, 0, 1);
	ImGui::SliderFloat3("LightDirector", &directionalLightData->direction.x, -1.0f, 1.0f);
	ImGui::DragFloat2("UVTranslate", &uvTransformSprite.translate.x, 0.01f, -10.0f, 10.0f);
	ImGui::DragFloat2("UVScale", &uvTransformSprite.scale.x, 0.01f, -10.0f, 10.0f);
	ImGui::SliderAngle("UVRotate", &uvTransformSprite.rotate.z);
	ImGui::End();

	//バックバッファのインデックスを取得	
	UINT backBufferIndex = swapChain->GetCurrentBackBufferIndex();

	//TransitionBarrierの設定
	D3D12_RESOURCE_BARRIER barrier{};
	
	//リソースバリアで書き込み可能に変更
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	//バリアを張る対象のリソース。現在のバックバッファに対して行う
	barrier.Transition.pResource = swapChainResource[backBufferIndex].Get();
	//遷移前( 現在 )のResourceState
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	//遷移後のResourceState
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	//TransitionBarrierを張る
	GetCommandList()->ResourceBarrier(1, &barrier);

	//描画先のRTVとDSVを設定する
	GetCommandList()->OMSetRenderTargets(1, &rtvHandles[backBufferIndex], false, &dsvHandle);
	GetCommandList()->OMSetRenderTargets(1, &dsvHandle[backBufferIndex], false, &rtvHandles);

	//画面全体の色をクリア
	float clearColor[] = { 0.1f,0.25f,0.5f,1.0f };
	commandList->ClearRenderTargetView(
		rtvHandles[backBufferIndex], clearColor, 0, nullptr);

	//画面全体の深度をクリア
	commandList->ClearDepthStencilView(dsvHandle,
		D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	//描画用のDescriptorHeapの設定
	ID3D12DescriptorHeap* descriptorHeaps[] = { srvDescriptorHeap.Get() };
	commandList->SetDescriptorHeaps(1, descriptorHeaps);

	//ビューポート領域のせってい
	GetCommandList()->RSSetViewports(1, &viewport);
	//シザー矩形の設定
	GetCommandList()->RSSetScissorRects(1, &scissorRect);
	//ImGuiの内部コマンドを生成する
	ImGui::Render();

	//描画
	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissorRect);
	commandList->SetGraphicsRootSignature(rootSignature.Get());
	commandList->SetPipelineState(graphicsPipelineState.Get());
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void DirectXCommon::PostDraw()
{
	//バックバッファのインデックスを取得	
	UINT backBufferIndex = swapChain->GetCurrentBackBufferIndex();

	//TransitionBarrierの設定
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	commandList->ResourceBarrier(1, &barrier);

	//コマンドリストの内容を確定させる。全てのコマンドを積んでからCloseすること
	hr = commandList->Close();
	assert(SUCCEEDED(hr));

	ID3D12CommandList* commandLists[] = { GetCommandList() };
	commandQueue->ExecuteCommandLists(1, commandLists);
	swapChain->Present(1, 0);

	//
	fenceValue++;
	commandQueue->Signal(fence.Get(), fenceValue);

	//
	if (fence->GetCompletedValue() < fenceValue) {
		fence->SetEventOnCompletion(fenceValue, fenceEvent);
		WaitForSingleObject(fenceEvent, INFINITE);
	}

	//
	hr = commandAllocator->Reset();
	assert(SUCCEEDED(hr));
	//
	hr = GetCommandList()->Reset(commandAllocator.Get(), nullptr);
	assert(SUCCEEDED(hr));
}
