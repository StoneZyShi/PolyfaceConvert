#pragma once
#include <string>
#include <Windows.h>

std::string getTime();

std::string deleteLine(std::string FileName);

std::string WChar2Ansi(LPCWSTR pwszSrc);

std::string ws2s(const std::wstring& ws);

