#include <windows.h>
#include <iostream>
#include <vector>
#include <tchar.h>


std::vector<std::wstring> activeAppTitles;


BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
    if (IsWindowVisible(hwnd)) {
        TCHAR windowTitle[256];
     
        GetWindowText(hwnd, windowTitle, sizeof(windowTitle) / sizeof(TCHAR));

       
        if (_tcslen(windowTitle) > 0) {
            activeAppTitles.push_back(windowTitle);
        }
    }
    return TRUE; 
}


std::vector<std::wstring> ListVisibleWindows() {
    EnumWindows(EnumWindowsProc, 0);
    return activeAppTitles;
}

