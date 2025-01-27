#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxcapi.h>
#include <array>
#include <wrl.h>
#include <string>
#include <format>
#include <assert.h>
#include <cassert>
#include "WinApp.h"
#include "Logger.h"
#include "StringUtility.h"
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"

//DirectX基盤
class DirectXCommon
{
public:
	//ディスクリプタヒープ生成関数
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap>
		CreateDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE heapType,
			UINT numDescriptors, bool shaderVisible);
	//ディスクリプタハンドル取得関数の移植
	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorHandle(
		ID3D12DescriptorHeap* descriptorHeap,
		uint32_t descriptorSize, uint32_t index);
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorHandle(
		ID3D12DescriptorHeap* descriptorHeap,
		uint32_t descriptorSize, uint32_t index);
	//深度ステンシルテクスチャ作成
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateDepthStencilTextureResource(
		ID3D12Device* device,
		int width,
		int height,
		DXGI_FORMAT format = DXGI_FORMAT_D32_FLOAT);
	//コンバート
	std::wstring ConvertString(const std::string& str);

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(
		ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible);

	ID3D12GraphicsCommandList* GetCommandList() { return commandList.Get(); }

	ID3D12Device* GetDevice() { return device.Get(); }

	IDXGISwapChain4* GetSwapChain() { return swapChain.Get(); }

	//初期化
	void Initialize(WinApp* winApp);
	//デバイスの初期化
	void InitializeDevice();
	//コマンド関連の初期化
	void CommandInitialize();
	//スワップチェーンの生成
	void CreateSwapChain();
	//震度バッファの生成
	void CreateDepthBuffer();
	//各種ディスクリプタヒープの生成
	void CreateDescriptorHeaps();
	//レンダーターゲットビューの初期化
	void RenderTargetViewInitialize();
	//深度ステンシルビューの初期化
	void DepthStencilViewInitialize();
	//フェンスの生成
	void CreateFence();
	//ビューポート矩形の初期化
	void ViewportInitialize();
	//シザリング矩形の初期化
	void ScissorRectInitialize();
	//DXCコンパイラの生成
	void CreateDXCCompiler();
	//ImGuiの初期化
	void ImGuiInitialize();

	void LoadTexture(const std::string& filePath);

	//描画前処理
	//void PreDraw();
	//void PostDraw();

private:

	HRESULT hr;
	//DirectXデバイス
	Microsoft::WRL::ComPtr<ID3D12Device> device;
	//DXGIファクトリー
	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory;
	//デバッグコントローラー
	Microsoft::WRL::ComPtr<ID3D12Debug>debugController = nullptr;
	//WindowsAPI
	WinApp* winApp = nullptr;
	//スワップチェーンリソース
	std::array<Microsoft::WRL::ComPtr<ID3D12Resource>,2> swapChainResources;
	// スワップチェーン
	Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain;
	//
	// RTV用ディスクリプタヒープ
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap;
	// DSV用ディスクリプタヒープ
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap;
	//

	// コマンド関連
	//
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator;
	//
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList;

	// フェンス
	//
	Microsoft::WRL::ComPtr<ID3D12Fence> fence;
	uint64_t fenceValue = 0;
	HANDLE fenceEvent = nullptr;
	//

	//0 ビューポート
	D3D12_VIEWPORT viewport{};
	// シザー矩形
	D3D12_RECT scissorRect{};

	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob = CompileShader(L"resources/shaders/Object3D.VS.hlsl", L"vs_6_0", dxcUtils, dxcCompiler, includeHnadler);
	assert(vertexShaderBlob != nullptr);

	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob = CompileShader(L"resources/shaders/Object3D.PS.hlsl", L"ps_6_0", dxcUtils, dxcCompiler, includeHnadler);
	assert(pixelShaderBlob != nullptr);

	// SRVを作成するDescriptorHeapの場所を決める(2枚目)
	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU2 = GetCPUDescriptorHandle(srvDescriptorHeap.Get(), descriptorSizeSRV, 2);
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU2 = GetGPUDescriptorHandle(srvDescriptorHeap.Get(), descriptorSizeSRV, 2);
};

