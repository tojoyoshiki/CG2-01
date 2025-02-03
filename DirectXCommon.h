#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxcapi.h>
#include <array>
#include <wrl.h>
#include <string>
#include <format>
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
	struct Vector3 {
		float x;
		float y;
		float z;
	};
	struct Vector4 {
		float x;
		float y;
		float z;
		float w;
	};
	struct Matrix4x4 {
		float m[4][4];
	};
	struct Transform {
		Vector3 scale;
		Vector3 rotate;
		Vector3 translate;
	};
	struct Material {
		Vector4 color;
		int32_t enableLighting;
		float padding[3];
		Matrix4x4 uvTransform;
	};
	struct DirectionalLight {
		Vector4 color;
		Vector3 direction;
		float intensity;
	};
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
	void PreDraw();
	void PostDraw();

private:

	HRESULT hr;
	Transform transform{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };
	Transform cameraTransform{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f} ,{0.0f,0.0f,-10.0f} };
	Transform transformSprite{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };
	Transform uvTransformSprite{
		{1.0f,1.0f,1.0f},
		{0.0f,0.0f,0.0f},
		{0.0f,0.0f,0.0f},
	};
	//SRV切り替え
	bool useMonsterBall = true;
	Material* materialData = nullptr;
	//Light用マテリアルリソース
	DirectionalLight* directionalLightData = nullptr;
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


};

