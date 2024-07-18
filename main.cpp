#include<Windows.h>
#include <cstdint>
#include <string>
#include<format>
#include<d3d12.h>
#include<dxgi1_6.h>
#include<cassert>
#define _USE_MATH_DEFINES
#include <math.h>
#include <dxgidebug.h>

#include <dxcapi.h>

#include "externals/DirectXTex/DirectXTex.h"

#include"externals/imgui/imgui.h"
#include"externals/imgui/imgui_impl_dx12.h"
#include"externals/imgui/imgui_impl_win32.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")

#pragma comment(lib,"dxguid.lib")

#pragma comment(lib,"dxcompiler.lib")

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

void log(const std::string& message) {
	OutputDebugStringA(message.c_str());
}

IDxcBlob* CompileShader(
	const std::wstring& filePath,
	const wchar_t* profile,
	IDxcUtils* dxcUtils,
	IDxcCompiler3* dxcCompiler,
	IDxcIncludeHandler* includeHandler)
{
	log(ConvertString(std::format(L"Begin CompileShader,path:{},profile:{}\n", filePath, profile)));

	IDxcBlobEncoding* shaderSource = nullptr;
	HRESULT hr = dxcUtils->LoadFile(filePath.c_str(), nullptr, &shaderSource);

	assert(SUCCEEDED(hr));

	DxcBuffer shaderSourceBuffer;
	shaderSourceBuffer.Ptr = shaderSource->GetBufferPointer();
	shaderSourceBuffer.Size = shaderSource->GetBufferSize();
	shaderSourceBuffer.Encoding = DXC_CP_UTF8;

	LPCWSTR arguments[] = {
		filePath.c_str(),
		L"-E",L"main",
		L"-T",profile,
		L"-Zi",L"-Qembed_debug",
		L"-Od",
		L"-Zpr",
	};

	IDxcResult* shaderResult = nullptr;
	hr = dxcCompiler->Compile(
		&shaderSourceBuffer,
		arguments,
		_countof(arguments),
		includeHandler,
		IID_PPV_ARGS(&shaderResult));

	assert(SUCCEEDED(hr));


	IDxcBlobUtf8* shaderError = nullptr;
	shaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&shaderError), nullptr);
	if (shaderError != nullptr && shaderError->GetStringLength() != 0)
	{
		log(shaderError->GetStringPointer());
		//警告エラーダメ絶対
		assert(false);
	}
	//4.Complie結果
	IDxcBlob* shaderBlob = nullptr;
	hr = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);
	assert(SUCCEEDED(hr));

	log(ConvertString(std::format(L"Compile Succeeded,path:{},profile:{}\n", filePath, profile)));

	shaderSource->Release();
	shaderResult->Release();

	return shaderBlob;
}

struct Vector2 {
	float x;
	float y;
};

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

Matrix4x4 MakeIdentity4x4() {
	Matrix4x4 result{};

	result.m[0][0] = 1.0f;
	result.m[0][1] = 0.0f;
	result.m[0][2] = 0.0f;
	result.m[0][3] = 0.0f;
	result.m[1][0] = 0.0f;
	result.m[1][1] = 1.0f;
	result.m[1][2] = 0.0f;
	result.m[1][3] = 0.0f;
	result.m[2][0] = 0.0f;
	result.m[2][1] = 0.0f;
	result.m[2][2] = 1.0f;
	result.m[2][3] = 0.0f;
	result.m[3][0] = 0.0f;
	result.m[3][1] = 0.0f;
	result.m[3][2] = 0.0f;
	result.m[3][3] = 1.0f;

	return result;
}

struct Transform {
	Vector3 scale;
	Vector3 rotate;
	Vector3 translate;
};

#pragma region Affine

Matrix4x4 Multiply(Matrix4x4 m1, Matrix4x4 m2) {
	Matrix4x4 result{};
	result.m[0][0] = m1.m[0][0] * m2.m[0][0] + m1.m[0][1] * m2.m[1][0] + m1.m[0][2] * m2.m[2][0] + m1.m[0][3] * m2.m[3][0];
	result.m[0][1] = m1.m[0][0] * m2.m[0][1] + m1.m[0][1] * m2.m[1][1] + m1.m[0][2] * m2.m[2][1] + m1.m[0][3] * m2.m[3][1];
	result.m[0][2] = m1.m[0][0] * m2.m[0][2] + m1.m[0][1] * m2.m[1][2] + m1.m[0][2] * m2.m[2][2] + m1.m[0][3] * m2.m[3][2];
	result.m[0][3] = m1.m[0][0] * m2.m[0][3] + m1.m[0][1] * m2.m[1][3] + m1.m[0][2] * m2.m[2][3] + m1.m[0][3] * m2.m[3][3];

	result.m[1][0] = m1.m[1][0] * m2.m[0][0] + m1.m[1][1] * m2.m[1][0] + m1.m[1][2] * m2.m[2][0] + m1.m[1][3] * m2.m[3][0];
	result.m[1][1] = m1.m[1][0] * m2.m[0][1] + m1.m[1][1] * m2.m[1][1] + m1.m[1][2] * m2.m[2][1] + m1.m[1][3] * m2.m[3][1];
	result.m[1][2] = m1.m[1][0] * m2.m[0][2] + m1.m[1][1] * m2.m[1][2] + m1.m[1][2] * m2.m[2][2] + m1.m[1][3] * m2.m[3][2];
	result.m[1][3] = m1.m[1][0] * m2.m[0][3] + m1.m[1][1] * m2.m[1][3] + m1.m[1][2] * m2.m[2][3] + m1.m[1][3] * m2.m[3][3];

	result.m[2][0] = m1.m[2][0] * m2.m[0][0] + m1.m[2][1] * m2.m[1][0] + m1.m[2][2] * m2.m[2][0] + m1.m[2][3] * m2.m[3][0];
	result.m[2][1] = m1.m[2][0] * m2.m[0][1] + m1.m[2][1] * m2.m[1][1] + m1.m[2][2] * m2.m[2][1] + m1.m[2][3] * m2.m[3][1];
	result.m[2][2] = m1.m[2][0] * m2.m[0][2] + m1.m[2][1] * m2.m[1][2] + m1.m[2][2] * m2.m[2][2] + m1.m[2][3] * m2.m[3][2];
	result.m[2][3] = m1.m[2][0] * m2.m[0][3] + m1.m[2][1] * m2.m[1][3] + m1.m[2][2] * m2.m[2][3] + m1.m[2][3] * m2.m[3][3];

	result.m[3][0] = m1.m[3][0] * m2.m[0][0] + m1.m[3][1] * m2.m[1][0] + m1.m[3][2] * m2.m[2][0] + m1.m[3][3] * m2.m[3][0];
	result.m[3][1] = m1.m[3][0] * m2.m[0][1] + m1.m[3][1] * m2.m[1][1] + m1.m[3][2] * m2.m[2][1] + m1.m[3][3] * m2.m[3][1];
	result.m[3][2] = m1.m[3][0] * m2.m[0][2] + m1.m[3][1] * m2.m[1][2] + m1.m[3][2] * m2.m[2][2] + m1.m[3][3] * m2.m[3][2];
	result.m[3][3] = m1.m[3][0] * m2.m[0][3] + m1.m[3][1] * m2.m[1][3] + m1.m[3][2] * m2.m[2][3] + m1.m[3][3] * m2.m[3][3];
	return result;
}

Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate) {

	Matrix4x4 resultX{};

	resultX.m[0][0] = 1.0f;
	resultX.m[0][1] = 0.0f;
	resultX.m[0][2] = 0.0f;
	resultX.m[0][3] = 0.0f;
	resultX.m[1][0] = 0.0f;
	resultX.m[1][1] = std::cos(rotate.x);
	resultX.m[1][2] = std::sin(rotate.x);
	resultX.m[1][3] = 0.0f;
	resultX.m[2][0] = 0.0f;
	resultX.m[2][1] = -(std::sin(rotate.x));
	resultX.m[2][2] = std::cos(rotate.x);
	resultX.m[2][3] = 0.0f;
	resultX.m[3][0] = 0.0f;
	resultX.m[3][1] = 0.0f;
	resultX.m[3][2] = 0.0f;
	resultX.m[3][3] = 1.0f;


	Matrix4x4 resultY{};

	resultY.m[0][0] = std::cos(rotate.y);
	resultY.m[0][1] = 0.0f;
	resultY.m[0][2] = -(std::sin(rotate.y));
	resultY.m[0][3] = 0.0f;
	resultY.m[1][0] = 0.0f;
	resultY.m[1][1] = 1.0f;
	resultY.m[1][2] = 0.0f;
	resultY.m[1][3] = 0.0f;
	resultY.m[2][0] = std::sin(rotate.y);
	resultY.m[2][1] = 0.0f;
	resultY.m[2][2] = std::cos(rotate.y);
	resultY.m[2][3] = 0.0f;
	resultY.m[3][0] = 0.0f;
	resultY.m[3][1] = 0.0f;
	resultY.m[3][2] = 0.0f;
	resultY.m[3][3] = 1.0f;

	Matrix4x4 resultZ{};

	resultZ.m[0][0] = std::cos(rotate.z);
	resultZ.m[0][1] = std::sin(rotate.z);
	resultZ.m[0][2] = 0.0f;
	resultZ.m[0][3] = 0.0f;
	resultZ.m[1][0] = -(std::sin(rotate.z));
	resultZ.m[1][1] = std::cos(rotate.z);
	resultZ.m[1][2] = 0.0f;
	resultZ.m[1][3] = 0.0f;
	resultZ.m[2][0] = 0.0f;
	resultZ.m[2][1] = 0.0f;
	resultZ.m[2][2] = 1.0f;
	resultZ.m[2][3] = 0.0f;
	resultZ.m[3][0] = 0.0f;
	resultZ.m[3][1] = 0.0f;
	resultZ.m[3][2] = 0.0f;
	resultZ.m[3][3] = 1.0f;

	Matrix4x4 rotateXYZ = Multiply(resultX, Multiply(resultY, resultZ));

	Matrix4x4 result;

	result.m[0][0] = scale.x * rotateXYZ.m[0][0];
	result.m[0][1] = scale.x * rotateXYZ.m[0][1];
	result.m[0][2] = scale.x * rotateXYZ.m[0][2];
	result.m[0][3] = 0.0f;
	result.m[1][0] = scale.y * rotateXYZ.m[1][0];
	result.m[1][1] = scale.y * rotateXYZ.m[1][1];
	result.m[1][2] = scale.y * rotateXYZ.m[1][2];
	result.m[1][3] = 0.0f;
	result.m[2][0] = scale.z * rotateXYZ.m[2][0];
	result.m[2][1] = scale.z * rotateXYZ.m[2][1];
	result.m[2][2] = scale.z * rotateXYZ.m[2][2];
	result.m[2][3] = 0.0f;
	result.m[3][0] = translate.x;
	result.m[3][1] = translate.y;
	result.m[3][2] = translate.z;
	result.m[3][3] = 1.0f;

	return result;
}
#pragma endregion

#pragma region 逆数
Matrix4x4 Inverse(const Matrix4x4& m) {
	float A = m.m[0][0] * m.m[1][1] * m.m[2][2] * m.m[3][3]
		+ m.m[0][0] * m.m[1][2] * m.m[2][3] * m.m[3][1]
		+ m.m[0][0] * m.m[1][3] * m.m[2][1] * m.m[3][2]
		- m.m[0][0] * m.m[1][3] * m.m[2][2] * m.m[3][1]
		- m.m[0][0] * m.m[1][2] * m.m[2][1] * m.m[3][3]
		- m.m[0][0] * m.m[1][1] * m.m[2][3] * m.m[3][2]
		- m.m[0][1] * m.m[1][0] * m.m[2][2] * m.m[3][3]
		- m.m[0][2] * m.m[1][0] * m.m[2][3] * m.m[3][1]
		- m.m[0][3] * m.m[1][0] * m.m[2][1] * m.m[3][2]
		+ m.m[0][3] * m.m[1][0] * m.m[2][2] * m.m[3][1]
		+ m.m[0][2] * m.m[1][0] * m.m[2][1] * m.m[3][3]
		+ m.m[0][1] * m.m[1][0] * m.m[2][3] * m.m[3][2]
		+ m.m[0][1] * m.m[1][2] * m.m[2][0] * m.m[3][3]
		+ m.m[0][2] * m.m[1][3] * m.m[2][0] * m.m[3][1]
		+ m.m[0][3] * m.m[1][1] * m.m[2][0] * m.m[3][2]
		- m.m[0][3] * m.m[1][2] * m.m[2][0] * m.m[3][1]
		- m.m[0][2] * m.m[1][1] * m.m[2][0] * m.m[3][3]
		- m.m[0][1] * m.m[1][3] * m.m[2][0] * m.m[3][2]
		- m.m[0][1] * m.m[1][2] * m.m[2][3] * m.m[3][0]
		- m.m[0][2] * m.m[1][3] * m.m[2][1] * m.m[3][0]
		- m.m[0][3] * m.m[1][1] * m.m[2][2] * m.m[3][0]
		+ m.m[0][3] * m.m[1][2] * m.m[2][1] * m.m[3][0]
		+ m.m[0][2] * m.m[1][1] * m.m[2][3] * m.m[3][0]
		+ m.m[0][1] * m.m[1][3] * m.m[2][2] * m.m[3][0];

	Matrix4x4 result{};
	result.m[0][0] = (m.m[1][1] * m.m[2][2] * m.m[3][3]
		+ m.m[1][2] * m.m[2][3] * m.m[3][1]
		+ m.m[1][3] * m.m[2][1] * m.m[3][2]
		- m.m[1][3] * m.m[2][2] * m.m[3][1]
		- m.m[1][2] * m.m[2][1] * m.m[3][3]
		- m.m[1][1] * m.m[2][3] * m.m[3][2]) / A;

	result.m[0][1] = (-m.m[0][1] * m.m[2][2] * m.m[3][3]
		- m.m[0][2] * m.m[2][3] * m.m[3][1]
		- m.m[0][3] * m.m[2][1] * m.m[3][2]
		+ m.m[0][3] * m.m[2][2] * m.m[3][1]
		+ m.m[0][2] * m.m[2][1] * m.m[3][3]
		+ m.m[0][1] * m.m[2][3] * m.m[3][2]) / A;

	result.m[0][2] = (m.m[0][1] * m.m[1][2] * m.m[3][3]
		+ m.m[0][2] * m.m[1][3] * m.m[3][1]
		+ m.m[0][3] * m.m[1][1] * m.m[3][2]
		- m.m[0][3] * m.m[1][2] * m.m[3][1]
		- m.m[0][2] * m.m[1][1] * m.m[3][3]
		- m.m[0][1] * m.m[1][3] * m.m[3][2]) / A;

	result.m[0][3] = (-m.m[0][1] * m.m[1][2] * m.m[2][3]
		- m.m[0][2] * m.m[1][3] * m.m[2][1]
		- m.m[0][3] * m.m[1][1] * m.m[2][2]
		+ m.m[0][3] * m.m[1][2] * m.m[2][1]
		+ m.m[0][2] * m.m[1][1] * m.m[2][3]
		+ m.m[0][1] * m.m[1][3] * m.m[2][2]) / A;


	result.m[1][0] = (-m.m[1][0] * m.m[2][2] * m.m[3][3]
		- m.m[1][2] * m.m[2][3] * m.m[3][0]
		- m.m[1][3] * m.m[2][0] * m.m[3][2]
		+ m.m[1][3] * m.m[2][2] * m.m[3][0]
		+ m.m[1][2] * m.m[2][0] * m.m[3][3]
		+ m.m[1][0] * m.m[2][3] * m.m[3][2]) / A;

	result.m[1][1] = (m.m[0][0] * m.m[2][2] * m.m[3][3]
		+ m.m[0][2] * m.m[2][3] * m.m[3][0]
		+ m.m[0][3] * m.m[2][0] * m.m[3][2]
		- m.m[0][3] * m.m[2][2] * m.m[3][0]
		- m.m[0][2] * m.m[2][0] * m.m[3][3]
		- m.m[0][0] * m.m[2][3] * m.m[3][2]) / A;

	result.m[1][2] = (-m.m[0][0] * m.m[1][2] * m.m[3][3]
		- m.m[0][2] * m.m[1][3] * m.m[3][0]
		- m.m[0][3] * m.m[1][0] * m.m[3][2]
		+ m.m[0][3] * m.m[1][2] * m.m[3][0]
		+ m.m[0][2] * m.m[1][0] * m.m[3][3]
		+ m.m[0][0] * m.m[1][3] * m.m[3][2]) / A;

	result.m[1][3] = (m.m[0][0] * m.m[1][2] * m.m[2][3]
		+ m.m[0][2] * m.m[1][3] * m.m[2][0]
		+ m.m[0][3] * m.m[1][0] * m.m[2][2]
		- m.m[0][3] * m.m[1][2] * m.m[2][0]
		- m.m[0][2] * m.m[1][0] * m.m[2][3]
		- m.m[0][0] * m.m[1][3] * m.m[2][2]) / A;


	result.m[2][0] = (m.m[1][0] * m.m[2][1] * m.m[3][3]
		+ m.m[1][1] * m.m[2][3] * m.m[3][0]
		+ m.m[1][3] * m.m[2][0] * m.m[3][1]
		- m.m[1][3] * m.m[2][1] * m.m[3][0]
		- m.m[1][1] * m.m[2][0] * m.m[3][3]
		- m.m[1][0] * m.m[2][3] * m.m[3][1]) / A;

	result.m[2][1] = (-m.m[0][0] * m.m[2][1] * m.m[3][3]
		- m.m[0][1] * m.m[2][3] * m.m[3][0]
		- m.m[0][3] * m.m[2][0] * m.m[3][1]
		+ m.m[0][3] * m.m[2][1] * m.m[3][0]
		+ m.m[0][1] * m.m[2][0] * m.m[3][3]
		+ m.m[0][0] * m.m[2][3] * m.m[3][1]) / A;

	result.m[2][2] = (m.m[0][0] * m.m[1][1] * m.m[3][3]
		+ m.m[0][1] * m.m[1][3] * m.m[3][0]
		+ m.m[0][3] * m.m[1][0] * m.m[3][1]
		- m.m[0][3] * m.m[1][1] * m.m[3][0]
		- m.m[0][1] * m.m[1][0] * m.m[3][3]
		- m.m[0][0] * m.m[1][3] * m.m[3][1]) / A;

	result.m[2][3] = (-m.m[0][0] * m.m[1][1] * m.m[2][3]
		- m.m[0][1] * m.m[1][3] * m.m[2][0]
		- m.m[0][3] * m.m[1][0] * m.m[2][1]
		+ m.m[0][3] * m.m[1][1] * m.m[2][0]
		+ m.m[0][1] * m.m[1][0] * m.m[2][3]
		+ m.m[0][0] * m.m[1][3] * m.m[2][1]) / A;

	result.m[3][0] = (-m.m[1][0] * m.m[2][1] * m.m[3][2]
		- m.m[1][1] * m.m[2][2] * m.m[3][0]
		- m.m[1][2] * m.m[2][0] * m.m[3][1]
		+ m.m[1][2] * m.m[2][1] * m.m[3][0]
		+ m.m[1][1] * m.m[2][0] * m.m[3][2]
		+ m.m[1][0] * m.m[2][2] * m.m[3][1]) / A;

	result.m[3][1] = (m.m[0][0] * m.m[2][1] * m.m[3][2]
		+ m.m[0][1] * m.m[2][2] * m.m[3][0]
		+ m.m[0][2] * m.m[2][0] * m.m[3][1]
		- m.m[0][2] * m.m[2][1] * m.m[3][0]
		- m.m[0][1] * m.m[2][0] * m.m[3][2]
		- m.m[0][0] * m.m[2][2] * m.m[3][1]) / A;

	result.m[3][2] = (-m.m[0][0] * m.m[1][1] * m.m[3][2]
		- m.m[0][1] * m.m[1][2] * m.m[3][0]
		- m.m[0][2] * m.m[1][0] * m.m[3][1]
		+ m.m[0][2] * m.m[1][1] * m.m[3][0]
		+ m.m[0][1] * m.m[1][0] * m.m[3][2]
		+ m.m[0][0] * m.m[1][2] * m.m[3][1]) / A;

	result.m[3][3] = (m.m[0][0] * m.m[1][1] * m.m[2][2]
		+ m.m[0][1] * m.m[1][2] * m.m[2][0]
		+ m.m[0][2] * m.m[1][0] * m.m[2][1]
		- m.m[0][2] * m.m[1][1] * m.m[2][0]
		- m.m[0][1] * m.m[1][0] * m.m[2][2]
		- m.m[0][0] * m.m[1][2] * m.m[2][1]) / A;

	return result;
}
#pragma endregion

Matrix4x4 MakePerspectiveFovMatrix(float forY, float aspectRatio, float nearClip, float farClip) {
	Matrix4x4 result;
	float cot = 1 / std::tan(forY / 2);

	result.m[0][0] = (1 / aspectRatio) * cot;
	result.m[1][1] = cot;
	result.m[2][2] = farClip / (farClip - nearClip);
	result.m[2][3] = 1.0f;
	result.m[3][2] = (-nearClip * farClip) / (farClip - nearClip);


	result.m[0][1] = 0.0f;
	result.m[0][2] = 0.0f;
	result.m[0][3] = 0.0f;
	result.m[1][0] = 0.0f;
	result.m[1][2] = 0.0f;
	result.m[1][3] = 0.0f;
	result.m[2][0] = 0.0f;
	result.m[2][1] = 0.0f;
	result.m[3][0] = 0.0f;
	result.m[3][1] = 0.0f;
	result.m[3][3] = 0.0f;


	return result;
}


Matrix4x4 MakeOrthographicMatrix(float left, float top, float right, float bottom, float nearClip, float farClip) {
	Matrix4x4 result;
	result.m[0][0] = 2 / (right - left);
	result.m[1][1] = 2 / (top - bottom);
	result.m[2][2] = 1 / (farClip - nearClip);

	result.m[3][0] = (left + right) / (left - right);
	result.m[3][1] = (top + bottom) / (bottom - top);
	result.m[3][2] = nearClip / (nearClip - farClip);
	result.m[3][3] = 1.0f;

	result.m[0][1] = 0.0f;
	result.m[0][2] = 0.0f;
	result.m[0][3] = 0.0f;
	result.m[1][0] = 0.0f;
	result.m[1][2] = 0.0f;
	result.m[1][3] = 0.0f;
	result.m[2][0] = 0.0f;
	result.m[2][1] = 0.0f;
	result.m[2][3] = 0.0f;

	return result;
}

struct VertexData {
	Vector4 position;
	Vector2 texcoord;
};

ID3D12Resource* CreateBufferResource(ID3D12Device* device, size_t sizeInBytes) {
	//VertexResource
	//頂点シェーダを作る
	D3D12_HEAP_PROPERTIES uploadHeapProperties{};
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;

	D3D12_RESOURCE_DESC vertexResourceDesc{};

	vertexResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	vertexResourceDesc.Width = sizeInBytes;

	vertexResourceDesc.Height = 1;
	vertexResourceDesc.DepthOrArraySize = 1;
	vertexResourceDesc.MipLevels = 1;
	vertexResourceDesc.SampleDesc.Count = 1;

	vertexResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	//実際に頂点リソースを作る
	ID3D12Resource* vertexResource = nullptr;
	HRESULT hr = device->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &vertexResourceDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vertexResource));
	assert(SUCCEEDED(hr));

	return vertexResource;
}

ID3D12DescriptorHeap* CreateDescriptorHeap(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDesciptors, bool shaderVisible)
{
	//ディスクリプターヒープの生成
	ID3D12DescriptorHeap* descriptorHeap = nullptr;
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
	descriptorHeapDesc.Type = heapType;
	descriptorHeapDesc.NumDescriptors = numDesciptors;

	descriptorHeapDesc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	HRESULT hr = device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&descriptorHeap));
	assert(SUCCEEDED(hr));
	return descriptorHeap;
}

DirectX::ScratchImage LoadTexture(const std::string& filePath) {
	//テクスチャファイル // byte関連
	DirectX::ScratchImage image{};
	std::wstring filePathW = ConvertString(filePath);
	HRESULT hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	assert(SUCCEEDED(hr));

	//ミップマップ　//拡大縮小で使う
	DirectX::ScratchImage mipImages{};
	hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);
	assert(SUCCEEDED(hr));

	//ミップマップ付きのデータを返す
	return mipImages;
}

ID3D12Resource* CrateTextureResource(ID3D12Device* device, const DirectX::TexMetadata& metadata) {

	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = UINT(metadata.width);//幅
	resourceDesc.Height = UINT(metadata.height);//高さ
	resourceDesc.MipLevels = UINT16(metadata.miscFlags);//数
	resourceDesc.DepthOrArraySize = UINT(metadata.arraySize);//奥行き　Textureの配置数
	resourceDesc.Format = metadata.format;//format
	resourceDesc.SampleDesc.Count = 1;//サンプリングカウント(1固定)
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION(metadata.dimension);// textureの次元数


	//利用するHeapの設定
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_CUSTOM;
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;


	//Resouceの生成
	ID3D12Resource* resource = nullptr;
	HRESULT hr = device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&resource)
	);
	assert(SUCCEEDED(hr));
	return resource;

}

void UploadTextureData(ID3D12Resource* texture, const DirectX::ScratchImage& mipImages) {

	const DirectX::TexMetadata& metadata = mipImages.GetMetadata();

	for (size_t mipLevel = 0; mipLevel < metadata.mipLevels; ++mipLevel) {
		const DirectX::Image* img = mipImages.GetImage(mipLevel, 0, 0);
		HRESULT hr = texture->WriteToSubresource(
			UINT(mipLevel),
			nullptr,			 //全領域へコピー
			img->pixels,		 //元データアドレス
			UINT(img->rowPitch), //1ラインサイズ
			UINT(img->slicePitch)//1枚サイズ
		);
		assert(SUCCEEDED(hr));
	}
}
ID3D12Resource* CreateDepthStencilTextureResource(ID3D12Device* device, int32_t width, int32_t height) {

	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = width;
	resourceDesc.Height = height;
	resourceDesc.MipLevels = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;//二次元
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;//DepthStrencil

	//利用するHeap
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;//VRAN上で作る

	//深度値のクリア設定
	D3D12_CLEAR_VALUE depthClearValue{};
	depthClearValue.DepthStencil.Depth = 1.0f;
	depthClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

	//resourceの生成
	ID3D12Resource* resource = nullptr;
	HRESULT hr = device->CreateCommittedResource(
		&heapProperties,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&depthClearValue,
		IID_PPV_ARGS(&resource));
	assert(SUCCEEDED(hr));
	return resource;
}

//ウィンドウプロシージャ
LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg,

	WPARAM wparam, LPARAM lparam) {
	//メッセージに応じてゲーム固有の処理を行う
	switch (msg) {
		//ウィンドウが破壊された
	case WM_DESTROY:
		//OSに対して、アプリの終了を伝える
		PostQuitMessage(0);
		return 0;
	}

	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam)) {
		return true;
	}

	//標準のメッセージ処理を行う
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

//Windowsアプリのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	CoInitializeEx(0, COINIT_MULTITHREADED);


#pragma region Windouの生成
	WNDCLASS wc{};

	//ウィンドウプロシージャ
	wc.lpfnWndProc = WindowProc;
	//ウィンドウクラス名
	wc.lpszClassName = L"C62WindowClass";
	//インスタンスハンドル
	wc.hInstance = GetModuleHandle(nullptr);
	//カーソル
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

	//ウィンドウクラスの登録
	RegisterClass(&wc);

	//クライアント領域のサイズ　横　縦
	const int32_t kClientWidth = 1280;
	const int32_t kClientHeight = 720;
	//　ウィンドウサイズを表す構造体にクライアント領域を入れる
	RECT wrc = { 0, 0,kClientWidth,kClientHeight };


	//クライアント領域をもとに実際のサイズにwrcを変更してもらう
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);


	HWND hwnd = CreateWindow(
		wc.lpszClassName,
		L"CG2",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		wrc.right - wrc.left,
		wrc.bottom - wrc.top,
		nullptr,
		nullptr,
		wc.hInstance,
		nullptr);

	ShowWindow(hwnd, SW_SHOW);

#pragma endregion

#ifdef _DEBUG
	ID3D12Debug1* debugController = nullptr;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
		//デバッグレイヤーを有効にする
		debugController->EnableDebugLayer();
		//さらにGPU側でもチェックを行うようにする
		debugController->SetEnableGPUBasedValidation(TRUE);
	}

#endif

#pragma region Factoryの生成
	//DZGIファクトリーの生成
	IDXGIFactory7* dxgiFactory = nullptr;
	//HREUSLTはWindouws系のエラーコード
	//関数が成功したかどうかをSUCCEEDEDマクロで判定できる
	HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory));
	//初期化の標本的な部分でエラーが出た場合はぷろぐらむがまちがっているか、
	// どうにもできない場合が多いのでassertにしておく
	assert(SUCCEEDED(hr));//甲であることを保証　そうでないと止まる

	log("Hello,DirectX\n");

#pragma endregion

#pragma region アダプタの作成

	//使用するアダプタ用の変数。最初にnullptrを入れておく
	IDXGIAdapter4* useAdapter = nullptr;
	//良い順にアダプタを読む
	for (UINT i = 0; dxgiFactory->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&useAdapter)) != DXGI_ERROR_NOT_FOUND; i++) {
		//アダプターの情報を取得する
		DXGI_ADAPTER_DESC3 adapterDesc{};
		hr = useAdapter->GetDesc3(&adapterDesc);
		assert(SUCCEEDED(hr));
		//ソフトウェアダプタでなければ採用
		if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)) {
			//採用したアダプタの情報をログに出力。wstringのほうなので注意
			log(ConvertString(std::format(L"Use Adapter:{}\n", adapterDesc.Description)));
			break;
		}
		useAdapter = nullptr;
	}
	//適切なアダプタが見つからないので起動しない
	//適切なアダプタが見つからなかったら起動できなくする
	assert(useAdapter != nullptr);

#pragma endregion

#pragma region Deviceの生成
	ID3D12Device* device = nullptr;

	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_12_2,D3D_FEATURE_LEVEL_12_1,D3D_FEATURE_LEVEL_12_0
	};
	const char* featureLevelStrings[] = { "12.2","12.1","12,0" };

	for (size_t i = 0; i < _countof(featureLevels); ++i) {
		hr = D3D12CreateDevice(useAdapter, featureLevels[i], IID_PPV_ARGS(&device));
		if (SUCCEEDED(hr)) {
			log(std::format("FEatureLevel : {}\n", featureLevelStrings[i]));
			break;
		}
	}

	assert(device != nullptr);
	log("Complete create D3D12Device!!!\n");

#pragma endregion

#ifdef _DEBUG
	ID3D12InfoQueue* infoQueue = nullptr;
	if (SUCCEEDED(device->QueryInterface(IID_PPV_ARGS(&infoQueue)))) {
		//やばいエラー時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
		//エラー時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
		//緊急時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);

		D3D12_MESSAGE_ID denyIds[] = {
			D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE
		};

		D3D12_MESSAGE_SEVERITY severities[] = { D3D12_MESSAGE_SEVERITY_INFO };
		D3D12_INFO_QUEUE_FILTER filter{};
		filter.DenyList.NumIDs = _countof(denyIds);
		filter.DenyList.pIDList = denyIds;
		filter.DenyList.NumCategories = _countof(severities);
		filter.DenyList.pSeverityList = severities;

		infoQueue->PushStorageFilter(&filter);

		infoQueue->Release();
	}

#endif

#pragma region コマンドキュー

	//コマンドキュー生成
	ID3D12CommandQueue* commandQueue = nullptr;
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
	hr = device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(&commandQueue));
	//生成できない場合
	assert(SUCCEEDED(hr));

#pragma endregion

#pragma region コマンドアロケータ
	//コマンドアロケータ生成
	ID3D12CommandAllocator* commandAllocator = nullptr;
	hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
	//生成できない場合
	assert(SUCCEEDED(hr));

#pragma endregion

#pragma region コマンドリスト
	//コマンドリスト生成
	ID3D12GraphicsCommandList* commandList = nullptr;
	hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator, nullptr, IID_PPV_ARGS(&commandList));
	//生成できない場合
	assert(SUCCEEDED(hr));
#pragma endregion

#pragma region ディスクリプターヒープの生成

	ID3D12DescriptorHeap* rtvDescriptorHeap = CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 2, false);

	ID3D12DescriptorHeap* srvDescriptorHeap = CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 128, true);

#pragma endregion

#pragma region Swap Chainの生成
	//スワップチェイン生成
	IDXGISwapChain4* swapChain = nullptr;
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	swapChainDesc.Width = kClientWidth;
	swapChainDesc.Height = kClientHeight;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 2;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	hr = dxgiFactory->CreateSwapChainForHwnd(commandQueue, hwnd, &swapChainDesc, nullptr, nullptr, reinterpret_cast<IDXGISwapChain1**>(&swapChain));
	assert(SUCCEEDED(hr));

	//SwapchainからResourceを引っ張ってくる
	ID3D12Resource* swapChainResource[2] = { nullptr };
	hr = swapChain->GetBuffer(0, IID_PPV_ARGS(&swapChainResource[0]));
	assert(SUCCEEDED(hr));

	hr = swapChain->GetBuffer(1, IID_PPV_ARGS(&swapChainResource[1]));
	assert(SUCCEEDED(hr));
#pragma endregion

	DirectX::ScratchImage mipImages = LoadTexture("resource/uvChecker.png");
	const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
	ID3D12Resource* textureResource = CrateTextureResource(device, metadata);
	UploadTextureData(textureResource, mipImages);

	ID3D12Resource* depthStencilResource = CreateDepthStencilTextureResource(device, kClientWidth, kClientHeight);

	ID3D12DescriptorHeap* dsvDescriptorHeap = CreateDescriptorHeap(device, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1, false);

	//DSV生成
	D3D12_DEPTH_STENCIL_VIEW_DESC dscDesc{};
	dscDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dscDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	//DSVHeapの先頭
	device->CreateDepthStencilView(depthStencilResource, &dscDesc, dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);

	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU = srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU = srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	//先頭ImGui
	textureSrvHandleCPU.ptr += device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	textureSrvHandleGPU.ptr += device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	//SRVの生成
	device->CreateShaderResourceView(textureResource, &srvDesc, textureSrvHandleCPU);

	//RTV
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	D3D12_CPU_DESCRIPTOR_HANDLE rtvStartHandle = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[2];

	rtvHandles[0] = rtvStartHandle;
	device->CreateRenderTargetView(swapChainResource[0], &rtvDesc, rtvHandles[0]);

	rtvHandles[1].ptr = rtvHandles[0].ptr + device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	device->CreateRenderTargetView(swapChainResource[1], &rtvDesc, rtvHandles[1]);
	//DXC
	IDxcUtils* dxcUtils = nullptr;
	IDxcCompiler3* dxcCompiler = nullptr;
	hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils));
	assert(SUCCEEDED(hr));
	hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler));
	assert(SUCCEEDED(hr));

	IDxcIncludeHandler* includeHandler = nullptr;
	hr = dxcUtils->CreateDefaultIncludeHandler(&includeHandler);
	assert(SUCCEEDED(hr));
#pragma region PSO

	//RootSignature
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	descriptorRange[0].BaseShaderRegister = 0;
	descriptorRange[0].NumDescriptors = 1;
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	//RootParameter作成
	D3D12_ROOT_PARAMETER rootParameters[3] = {};
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[0].Descriptor.ShaderRegister = 0;

	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameters[1].Descriptor.ShaderRegister = 0;//Object3d.VS.hlsl の b0

	descriptionRootSignature.pParameters = rootParameters;
	descriptionRootSignature.NumParameters = _countof(rootParameters);

	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	rootParameters[2].DescriptorTable.pDescriptorRanges = descriptorRange;
	rootParameters[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);

	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;
	staticSamplers[0].ShaderRegister = 0;
	staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	descriptionRootSignature.pStaticSamplers = staticSamplers;
	descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);
	ID3DBlob* signatureBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;
	hr = D3D12SerializeRootSignature(&descriptionRootSignature,
		D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr)) {
		log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}
	//バイナリを元に生成
	ID3D12RootSignature* rootSignature = nullptr;
	hr = device->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
	assert(SUCCEEDED(hr));
	//InputLayout
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[2] = {};
	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	inputElementDescs[1].SemanticName = "TEXCOORD";
	inputElementDescs[1].SemanticIndex = 0;
	inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = _countof(inputElementDescs);

	D3D12_BLEND_DESC blendDesc{};
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	
	D3D12_RASTERIZER_DESC rasterizerDesc{};

	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;

	//shaderのコンパイラ
	IDxcBlob* vertexShaderBlob = CompileShader(L"Object3d.VS.hlsl", L"vs_6_0", dxcUtils, dxcCompiler, includeHandler);
	assert(vertexShaderBlob != nullptr);

	IDxcBlob* pixelShaderBlob = CompileShader(L"Object3d.PS.hlsl", L"ps_6_0", dxcUtils, dxcCompiler, includeHandler);
	assert(pixelShaderBlob != nullptr);


	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
	graphicsPipelineStateDesc.pRootSignature = rootSignature;
	graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;
	graphicsPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(),vertexShaderBlob->GetBufferSize() };
	graphicsPipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(),pixelShaderBlob->GetBufferSize() };
	graphicsPipelineStateDesc.BlendState = blendDesc;
	graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;

	graphicsPipelineStateDesc.NumRenderTargets = 1;
	graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

	graphicsPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	graphicsPipelineStateDesc.SampleDesc.Count = 1;
	graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc;
	graphicsPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

	ID3D12PipelineState* graphicsPipelineState = nullptr;
	hr = device->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&graphicsPipelineState));
	assert(SUCCEEDED(hr));

#pragma endregion

	ID3D12Resource* vertexResource = CreateBufferResource(device, sizeof(VertexData) * 6);

	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};

	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();

	vertexBufferView.SizeInBytes = sizeof(VertexData) * 6;

	vertexBufferView.StrideInBytes = sizeof(VertexData);

	//頂点リソースにデータを書き込む
	VertexData* vertexData = nullptr;
	//書き込むためのアドレスを取得
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	const uint32_t kSubdivisiont = 16;  // 分割数

	const float kLonEvery = 2.0f * float(M_PI) / kSubdivisiont;  // 経度分割1つ分の角度
	const float kLatEvery = float(M_PI) / kSubdivisiont;  // 緯度分割1つ分の角度

	// 緯度の方向に分割　-π/2 ~ π/2
	for (uint32_t latIndex = 0; latIndex < kSubdivisiont; ++latIndex) {
		float lat = -float(M_PI) / 2.0f + kLatEvery * latIndex; // 現在の緯度

		// 経度の方向に分割0 ~ 2π
		for (uint32_t lonIndex = 0; lonIndex < kSubdivisiont; ++lonIndex)
		{
			float v;
			float u;

			u = float(lonIndex) / float(kSubdivisiont);
			v = 1.0f - float(latIndex) / float(kSubdivisiont);

			uint32_t start = (latIndex * kSubdivisiont + lonIndex) * 6;
			float lon = lonIndex * kLonEvery;

			//頂点にデータを入力する。基準点a
			//a
			vertexData[start].position.x = cos(lat) * cos(lon);
			vertexData[start].position.y = sin(lat);
			vertexData[start].position.z = cos(lat) * sin(lon);
			vertexData[start].position.w = 1.0f;
			vertexData[start].texcoord = { u,v };
			//b
			vertexData[start + 2].position.x = cos(lat) * cos(lon + kLonEvery);
			vertexData[start + 2].position.y = sin(lat);
			vertexData[start + 2].position.z = cos(lat) * sin(lon + kLonEvery);
			vertexData[start + 2].position.w = 1.0f;
			vertexData[start + 2].texcoord = { u + 1.0f / kSubdivisiont,v };
			//c
			vertexData[start + 1].position.x = cos(lat + kLatEvery) * cos(lon);
			vertexData[start + 1].position.y = sin(lat + kLatEvery);
			vertexData[start + 1].position.z = cos(lat + kLatEvery) * sin(lon);
			vertexData[start + 1].position.w = 1.0f;
			vertexData[start + 1].texcoord = { u ,v - 1.0f / kSubdivisiont };
			//d
			vertexData[start + 3].position.x = cos(lat + kLatEvery) * cos(lon + kLonEvery);
			vertexData[start + 3].position.y = sin(lat + kLatEvery);
			vertexData[start + 3].position.z = cos(lat + kLatEvery) * sin(lon + kLonEvery);
			vertexData[start + 3].position.w = 1.0f;
			vertexData[start + 3].texcoord = { u + 1.0f / kSubdivisiont ,v - 1.0f / kSubdivisiont };
			//b2
			vertexData[start + 4].position.x = cos(lat) * cos(lon + kLonEvery);
			vertexData[start + 4].position.y = sin(lat);
			vertexData[start + 4].position.z = cos(lat) * sin(lon + kLonEvery);
			vertexData[start + 4].position.w = 1.0f;
			vertexData[start + 4].texcoord = { u + 1.0f / kSubdivisiont,v };
			//c2
			vertexData[start + 5].position.x = cos(lat + kLatEvery) * cos(lon);
			vertexData[start + 5].position.y = sin(lat + kLatEvery);
			vertexData[start + 5].position.z = cos(lat + kLatEvery) * sin(lon);
			vertexData[start + 5].position.w = 1.0f;
			vertexData[start + 5].texcoord = { u,v - 1.0f / kSubdivisiont };
		}
	}


	ID3D12Resource* wvpResource = CreateBufferResource(device, sizeof(Matrix4x4));

	Matrix4x4* wvpDate = nullptr;

	wvpResource->Map(0, nullptr, reinterpret_cast<void**>(&wvpDate));

	*wvpDate = MakeIdentity4x4();

	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	//leftTop
	//vertexData[0].position = { -0.5f, -0.5f,0.0f,1.0f };
	//vertexData[0].texcoord = { 0.0f,1.0f };

	////Top
	//vertexData[1].position = { 0.0f, 0.5f,0.0f,1.0f };
	//vertexData[1].texcoord = { 0.5f,0.0f };

	////rightBottom
	//vertexData[2].position = { 0.5f, -0.5f,0.0f,1.0f };
	//vertexData[2].texcoord = { 1.0f,1.0f };

	////leftTop
	//vertexData[3].position = { -0.5f, -0.5f,0.5f,1.0f };
	//vertexData[3].texcoord = { 0.0f,1.0f };

	////Top
	//vertexData[4].position = { 0.0f, 0.0f,0.0f,1.0f };
	//vertexData[4].texcoord = { 0.5f,0.0f };

	////rightBottom
	//vertexData[5].position = { 0.5f, -0.5f,-0.5f,1.0f };
	//vertexData[5].texcoord = { 1.0f,1.0f };

	//Spricte
	ID3D12Resource* vertexResourceSprite = CreateBufferResource(device, sizeof(VertexData) * 6);

	//頂点バッファービュー
	D3D12_VERTEX_BUFFER_VIEW vertexBufferViewSprite{};
	//リソースの先頭アドレス
	vertexBufferViewSprite.BufferLocation = vertexResourceSprite->GetGPUVirtualAddress();
	//使用するリソースサイズ
	vertexBufferViewSprite.SizeInBytes = sizeof(VertexData) * 6;
	//頂点サイズ
	vertexBufferViewSprite.StrideInBytes = sizeof(VertexData);

	ID3D12Resource* transformationMatrixResourceSprite = CreateBufferResource(device, sizeof(Matrix4x4));

	Matrix4x4* transformationMatrixDataSprite = nullptr;

	transformationMatrixResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixDataSprite));

	*transformationMatrixDataSprite = MakeIdentity4x4();

	VertexData* vertexDateSprite = nullptr;
	vertexResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&vertexDateSprite));

	vertexDateSprite[0].position = { 0.0f,360.0f,0.0f,1.0f };
	vertexDateSprite[0].texcoord = { 0.0f,1.0f };
	vertexDateSprite[1].position = { 0.0f,0.0f,0.0f,1.0f };
	vertexDateSprite[1].texcoord = { 0.0f,0.0f };
	vertexDateSprite[2].position = { 640.0f,360.0f,0.0f,1.0f };
	vertexDateSprite[2].texcoord = { 1.0f,1.0f };
	vertexDateSprite[3].position = { 0.0f,0.0f,0.0f,1.0f };
	vertexDateSprite[3].texcoord = { 0.0f,0.0f };
	vertexDateSprite[4].position = { 640.0f,0.0f,0.0f,1.0f };
	vertexDateSprite[4].texcoord = { 1.0f,0.0f };
	vertexDateSprite[5].position = { 640.0f,360.0f,0.0f,1.0f };
	vertexDateSprite[5].texcoord = { 1.0f,1.0f };
	ID3D12Resource* materialResource = CreateBufferResource(device, sizeof(Vector4));
	Vector4* materialDate = nullptr;
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialDate));
	*materialDate = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	//ビューポート
	D3D12_VIEWPORT viewport;

	viewport.Width = kClientWidth;
	viewport.Height = kClientHeight;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	D3D12_RECT scissorRect{};

	scissorRect.left = 0;
	scissorRect.right = kClientWidth;
	scissorRect.top = 0;
	scissorRect.bottom = kClientHeight;
	Transform transform{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f} ,{0.0f,0.0f,0.0f} };
	Transform cameraTransform = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f}, {0.0f,0.0f,-4.5f} };

	Transform transformSprite{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };

	float* inputMaterial[3] = { &materialDate->x,&materialDate->y,&materialDate->z };
	float* inputTransform[3] = { &transform.translate.x,&transform.translate.y,&transform.translate.z };

	float* inputRotate[3] = { &transform.rotate.x,&transform.rotate.y,&transform.rotate.z };

	float* inputScale[3] = { &transform.scale.x,&transform.scale.y,&transform.scale.z };

	//ImGui初期化
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX12_Init(device, swapChainDesc.BufferCount,
		rtvDesc.Format, srvDescriptorHeap,
		srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(),
		srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());

	//初期化で0でFenceを作る
	ID3D12Fence* fence = nullptr;
	uint64_t fenceValue = 0;
	hr = device->CreateFence(fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence));
	assert(SUCCEEDED(hr));

	HANDLE fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	assert(fenceEvent != nullptr);
	        
	MSG msg{};
	//ウィンドウの×ボタンが押されるまでループ
	while (msg.message != WM_QUIT) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			//ゲームの処理

			ImGui_ImplDX12_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();

			transform.rotate.y += 0.03f;

			Matrix4x4 worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
			Matrix4x4 cameraMatrix = MakeAffineMatrix(cameraTransform.scale, cameraTransform.rotate, cameraTransform.translate);
			Matrix4x4 viewMatrix = Inverse(cameraMatrix);
			Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(0.45f, float(1280.0f) / float(720.0f), 0.1f, 100.0f);
			//Matrix4x4 projectionMatrix = MakeOrthographicMatrix((float)scissorRect.left, (float)scissorRect.top, (float)scissorRect.right, (float)scissorRect.bottom, 0.1f, 100.0f);
			Matrix4x4 WorldViewProjectionMatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));

			*wvpDate = WorldViewProjectionMatrix;
			Matrix4x4 worldMatrixSprite = MakeAffineMatrix(transformSprite.scale, transformSprite.rotate, transformSprite.translate);
			//Matrix4x4 cameraMatrixSprite = MakeAffineMatrix(cameraTransform.scale, cameraTransform.rotate, cameraTransform.translate);
			Matrix4x4 viewMatrixSprite = MakeIdentity4x4();
			//Matrix4x4 projectionMatrixSprite = MakePerspectiveFovMatrix(0.45f, float(1280.0f) / float(720.0f), 0.1f, 100.0f);
			Matrix4x4 projectionMatrixSprite = MakeOrthographicMatrix(0.0f, 0.0f, (float)kClientWidth, (float)kClientHeight, 0.0f, 100.0f);
			Matrix4x4 worldViewProjectionMatrixSprite = Multiply(worldMatrixSprite, Multiply(viewMatrixSprite, projectionMatrixSprite));

			*transformationMatrixDataSprite = worldViewProjectionMatrixSprite;

			ImGui::ShowDemoWindow();

			//ImGuiの内部コマンド
			ImGui::Render();
			//　これから書き込みバックバッファのインデックスを取得
			UINT backBufferIndex = swapChain->GetCurrentBackBufferIndex();

			//TransitionBarrierの設定
			D3D12_RESOURCE_BARRIER barrier{};
			//今回のバリアはTransition
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			//Noneにしておく
			barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			//バリアを貼る対象のリソース。現在のバッファに対して行う
			barrier.Transition.pResource = swapChainResource[backBufferIndex];
			//前の(現在の)ResourceState
			barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
			//後のResourceState
			barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
			//TransitionBarrierを張る
			commandList->ResourceBarrier(1, &barrier);
			// 描画先のRTVの設定をする
			commandList->OMSetRenderTargets(1, &rtvHandles[backBufferIndex], false, nullptr);
			//指定した色で画面をクリアする　
			float clearColor[] = { 0.1f,0.25f,0.5f,1.0f };
			//コマンド蓄積
			commandList->ClearRenderTargetView(rtvHandles[backBufferIndex], clearColor, 0, nullptr);

			//描画用のDescriptorHeap
			ID3D12DescriptorHeap* descriptorHeaps[] = { srvDescriptorHeap };
			commandList->SetDescriptorHeaps(1, descriptorHeaps);

			//DSV
			D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
			commandList->OMSetRenderTargets(1, &rtvHandles[backBufferIndex], false, &dsvHandle);

			commandList->RSSetViewports(1, &viewport);
			commandList->RSSetScissorRects(1, &scissorRect);

			commandList->SetGraphicsRootSignature(rootSignature);
			commandList->SetPipelineState(graphicsPipelineState);
			commandList->IASetVertexBuffers(0, 1, &vertexBufferView);

			commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			commandList->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());

			commandList->SetGraphicsRootConstantBufferView(1, wvpResource->GetGPUVirtualAddress());

			commandList->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU);

			commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

			commandList->DrawInstanced(kSubdivisiont, 1, 0, 0);

			commandList->IASetVertexBuffers(0, 1, &vertexBufferViewSprite);
			commandList->SetGraphicsRootConstantBufferView(1, transformationMatrixResourceSprite->GetGPUVirtualAddress());
			commandList->DrawInstanced(6, 1, 0, 0);

			//実際のcommandListのImGui描画コマンドを挟む
			ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList);


			barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
			barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
			//TransitionBarrierを張る
			commandList->ResourceBarrier(1, &barrier);
			hr = commandList->Close();
			assert(SUCCEEDED(hr));


			//GPUにコマンドリストの実行を行わせる
			ID3D12CommandList* commandLists[] = { commandList };
			commandQueue->ExecuteCommandLists(1, commandLists);
			//GPUとOSに画面の交換を行うように通知する
			swapChain->Present(1, 0);

			//// 出力ウィンドウへの文字出力
			//OutputDebugStringA("Hello DirectX!\n");
			//FENCEを更新する
			fenceValue++;

			commandQueue->Signal(fence, fenceValue);

			if (fence->GetCompletedValue() < fenceValue) {

				fence->SetEventOnCompletion(fenceValue, fenceEvent);

				WaitForSingleObject(fenceEvent, INFINITE);

			}


			//次のフレームのコマンドリストを準備
			hr = commandAllocator->Reset();
			assert(SUCCEEDED(hr));
			hr = commandList->Reset(commandAllocator, nullptr);
			assert(SUCCEEDED(hr));

		}
	}
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	CloseHandle(fenceEvent);
	fence->Release();
	rtvDescriptorHeap->Release();
	srvDescriptorHeap->Release();
	swapChainResource[0]->Release();
	swapChainResource[1]->Release();
	swapChain->Release();
	commandList->Release();
	commandAllocator->Release();
	commandQueue->Release();
	device->Release();
	useAdapter->Release();
	dxgiFactory->Release();

	wvpResource->Release();
	vertexResource->Release();

	graphicsPipelineState->Release();

	signatureBlob->Release();
	if (errorBlob) {
		errorBlob->Release();
	}
	rootSignature->Release();
	pixelShaderBlob->Release();
	vertexShaderBlob->Release();

	materialResource->Release();

#ifdef _DEBUG
	debugController->Release();
#endif

	mipImages.Release();
	textureResource->Release();

	depthStencilResource->Release();
	dsvDescriptorHeap->Release();


	vertexResourceSprite->Release();
	transformationMatrixResourceSprite->Release();

	CloseWindow(hwnd);

	//リソースリークチェック
	IDXGIDebug1* debug;
	if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debug)))) {
		debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
		debug->ReportLiveObjects(DXGI_DEBUG_APP, DXGI_DEBUG_RLO_ALL);
		debug->ReportLiveObjects(DXGI_DEBUG_D3D12, DXGI_DEBUG_RLO_ALL);
		debug->Release();
	}

	CoUninitialize();

	return 0;
}