#pragma once
#include<d3d12.h>
#include<dxgi1_6.h>
#include<wrl.h>
#include"WinApp.h"
#include<string>
#include<array>
#include <format>
#include<dxcapi.h>
#include <cassert>
#include <vector>
#include "Logger.h"
#include "StringUtility.h"
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"
#include"externals/DirectXTex/DirectXTex.h"
#include"externals/DirectXTex/d3dx12.h"

using namespace Microsoft::WRL;

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
	//コンバート
	std::wstring ConvertString(const std::string& str);
	//コンパイルシェーダー
	Microsoft::WRL::ComPtr<IDxcBlob> CompileShader(const std::wstring& filePath,
		const wchar_t* profile);
	//ディスクリプタヒープ作成
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(
		ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool shaderVisible);
	
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(size_t size);
	//テクスチャデータ転送
	//void UploadTextureData(Microsoft::WRL::ComPtr<ID3D12Resource> texture, 
	//	const DirectX::ScratchImage& mipImages);
	
	Microsoft::WRL::ComPtr<ID3D12Resource> UploadTextureData(ID3D12Resource* texture, const DirectX::ScratchImage& mipImages, ID3D12Device* device,
		ID3D12GraphicsCommandList* commandList);

	//テクスチャーロード
	static DirectX::ScratchImage LoadTexture(const std::string& filePath);
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateDepthStencilTextureResource();

	Microsoft::WRL::ComPtr<ID3D12Resource>CreateTextureResource(
		ID3D12Device* device, const DirectX::TexMetadata& metadata);

	//Getter
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> GetCommandList() { return commandList.Get(); }
	Microsoft::WRL::ComPtr<ID3D12Device> GetDevice() { return device.Get(); }
	Microsoft::WRL::ComPtr<IDXGISwapChain4> GetSwapChain() { return swapChain.Get(); }
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> GetCommandList() const { return commandList.Get(); }

	//初期化
	void Initialize(WinApp* winApp);
	//終了
	void Finalize();
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
	//GraphicsPipelineの作成
	void CreateGraphicsPipeline(const D3D12_GRAPHICS_PIPELINE_STATE_DESC* graphicsPipelineStateDesc);

	//描画前処理
	void PreDraw();
	void PostDraw();

	// RTV用ディスクリプタヒープ
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap;
	// DSV用ディスクリプタヒープ
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap;
	//SRV用ディスクリプタヒープ
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvDescriptorHeap;
	//ディスクリプタヒープの生成
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap;
	//DescriptorSizeを取得しておく
	uint32_t descriptorSizeSRV;
	uint32_t descriptorSizeRTV;
	uint32_t descriptorSizeDSV;

	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU2;
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU2;

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
//	Material* materialData = nullptr;
	//Light用マテリアルリソース
	//DirectionalLight* directionalLightData = nullptr;
	//DirectXデバイス
	Microsoft::WRL::ComPtr<ID3D12Device> device = nullptr;
	//DXGIファクトリー
	Microsoft::WRL::ComPtr<IDXGIFactory7>dxgiFactory = nullptr;
	//読み込みと管理など
	IDxcUtils* dxcUtils;
	//コンパイラ
	IDxcCompiler3* dxcCompiler;
	//include処理ハンドル
	IDxcIncludeHandler* includeHandler;
	//デバッグコントローラー
	Microsoft::WRL::ComPtr<ID3D12Debug1> debugController = nullptr;;

	Microsoft::WRL::ComPtr<ID3D12Resource>createTextureResource=nullptr;

	//使用するアダプタ用の変数。最初に nullptr を入れておく
	Microsoft::WRL::ComPtr<IDXGIAdapter4> useAdapter;

	Microsoft::WRL::ComPtr<ID3D12InfoQueue> infoQueue = nullptr;
	//WindowsAPI
	WinApp* winApp = nullptr;
	//スワップチェーンリソース
	std::array<Microsoft::WRL::ComPtr<ID3D12Resource>,2> swapChainResources;
	// スワップチェーン
	Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain;

	//RTVの設定
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	//RTVを2つ作るのでディスクリプタを2つ用意
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2];
	// DSVの設定
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
	//dsvを作る
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle;

	//ルートシグネチャ
	//Microsoft::WRL::ComPtr<ID3D12RootSignature>rootSignature = nullptr;

	//
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicsPipelineState = nullptr;
	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicPipelineStateDesc{};

	Microsoft::WRL::ComPtr<ID3D12Resource> swapChainResource[2] = { nullptr };

	//Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResource;
	
	// コマンド関連
	//
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator;
	//
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList;

	D3D12_RESOURCE_DESC vertexResourceDesc{};

	// フェンス
	//
	Microsoft::WRL::ComPtr<ID3D12Fence> fence;
	uint64_t fenceValue = 0;
	HANDLE fenceEvent = nullptr;
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};

	D3D12_RESOURCE_BARRIER barrier{};
	//

	//0 ビューポート
	D3D12_VIEWPORT viewport{};
	// シザー矩形
	D3D12_RECT scissorRect{};

	// Resourceの生成
	Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResource = nullptr;
};

