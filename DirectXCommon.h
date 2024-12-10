#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include "WinApp.h"

//DirectX基盤
class DirectXCommon
{
public:
	void Initialize(WinApp* winApp);
	//コマンド関連の初期化
	void CommandInitialize();
	//スワップチェーンの生成
	void CreateSwapChain();
	//震度バッファの生成
	void CreateDepthBuffer();
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
	//各種ディスクリプタヒープの生成
	void CreateDescriptorHeap();
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

private:
	//DirectXデバイス
	Microsoft::WRL::ComPtr<ID3D12Device> device;
	//DXGIファクトリー
	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory;

	//WindowsAPI
	WinApp* winApp = nullptr;
};

