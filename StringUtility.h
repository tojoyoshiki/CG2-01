#pragma once
#include <string>
#include <wrl.h>

namespace StringUtility {
	std::wstring ConvertString(const std::string& str);
	std::string ConvertString(const std::wstring& str);
}