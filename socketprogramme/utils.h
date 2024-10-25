#pragma once
#include <windows.h>
#include <iostream>
#include <vector>
#include <tchar.h>
bool imageCapture();
std::wstring ListVisibleWindows();
std::wstring ListServices();
std::string wstringToString(const std::wstring& wstr);
bool terminateAppByPID(DWORD pid);
bool startApplication(const std::wstring& appPath);