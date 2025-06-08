#pragma once

#include <format>

#define TRACE(fmt, ...) \
	OutputDebugStringA(std::format(fmt, ##__VA_ARGS__).c_str());