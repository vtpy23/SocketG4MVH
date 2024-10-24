#include <windows.h>
#include <iostream>
#include <sstream>
#include <tchar.h>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <map>

using namespace cv;
using namespace std;

std::wstringstream activeAppTitlesStream;
std::map<std::wstring, DWORD> windowTitlesAndPIDs;

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
    if (IsWindowVisible(hwnd)) {
        TCHAR windowTitle[256];
        GetWindowText(hwnd, windowTitle, sizeof(windowTitle) / sizeof(TCHAR));

        if (_tcslen(windowTitle) > 0) {
            DWORD pid;
            GetWindowThreadProcessId(hwnd, &pid);  // Lấy PID của cửa sổ
            activeAppTitlesStream << windowTitle << L" (PID: " << pid << L")\n";
            windowTitlesAndPIDs[windowTitle] = pid; // Lưu tiêu đề và PID vào map
        }
    }
    return TRUE;
}


std::wstring ListVisibleWindows() {
    activeAppTitlesStream.str(L"");  // Xóa nội dung cũ
    windowTitlesAndPIDs.clear();     // Xóa map cũ
    EnumWindows(EnumWindowsProc, 0);
    return activeAppTitlesStream.str();
}

std::wstring ListServices() {
    SC_HANDLE scm = OpenSCManager(NULL, NULL, SC_MANAGER_ENUMERATE_SERVICE);
    if (!scm) {
        return L"Failed to open Service Control Manager";  // Chuỗi kiểu wide string cần thêm tiền tố 'L'
    }

    DWORD bytesNeeded = 0, servicesCount = 0;
    EnumServicesStatus(scm, SERVICE_WIN32, SERVICE_STATE_ALL, NULL, 0, &bytesNeeded, &servicesCount, NULL);

    std::vector<BYTE> buffer(bytesNeeded);
    LPENUM_SERVICE_STATUS services = (LPENUM_SERVICE_STATUS)buffer.data();

    if (!EnumServicesStatus(scm, SERVICE_WIN32, SERVICE_STATE_ALL, services, bytesNeeded, &bytesNeeded, &servicesCount, NULL)) {
        CloseServiceHandle(scm);
        return L"Failed to enumerate services";  // Sửa thành wide string
    }

    std::wstring serviceList;  // Sử dụng std::wstring thay cho std::string
    for (DWORD i = 0; i < servicesCount; i++) {
        serviceList += services[i].lpServiceName;  // Đây là LPWSTR, tương thích với std::wstring
        serviceList += L" - ";  // Wide string
        serviceList += services[i].lpDisplayName;  // LPWSTR
        serviceList += L"\n";  // Wide string
    }

    CloseServiceHandle(scm);
    return serviceList;  // Trả về std::wstring
}

std::string wstringToString(const std::wstring& wstr) {
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string str(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), (int)wstr.size(), &str[0], size_needed, NULL, NULL);
    return str;
}

bool imageCapture() {
    VideoCapture cap(0); // Mở camera mặc định
    if (!cap.isOpened()) {
        cout << "Cannot open camera!" << endl;
        return false;
    }

    Mat frame;
    cap >> frame; // Chụp ảnh
    if (frame.empty()) {
        cout << "Captured frame is empty!" << endl;
        return false;
    }

    // Đặt tên tệp và định dạng
    std::string filePath = "C:\\Users\\Admin\\Downloads\\screenshot.jpg";

    // Ghi ảnh vào tệp
    if (!imwrite(filePath, frame)) {
        cout << "Failed to save the image!" << endl;
        return false;
    }

    cap.release(); // Giải phóng camera
    cout << "Image captured and saved successfully at " << filePath << endl;
    return true;
}

bool terminateAppByPID(DWORD pid) {
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
    if (hProcess == NULL) {
        std::cerr << "Failed to open process with PID: " << pid << std::endl;
        return false;
    }

    if (!TerminateProcess(hProcess, 0)) {
        std::cerr << "Failed to terminate process with PID: " << pid << std::endl;
        CloseHandle(hProcess);
        return false;
    }

    CloseHandle(hProcess);
    return true;
}

bool startApplication(const std::wstring& appPath) {
    STARTUPINFO si = { sizeof(si) };
    PROCESS_INFORMATION pi;

    if (!CreateProcess(appPath.c_str(), NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        std::cerr << "Failed to start application: " << wstringToString(appPath) << std::endl;
        return false;
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return true;
}