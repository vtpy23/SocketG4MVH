#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include "utils.h"
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Psapi.lib")
#define DEFAULT_PORT "8081"
#define DEFAULT_BUFLEN 512 
using namespace std;
int main() {
    WSADATA wsaData;
    SOCKET ListenSocket = INVALID_SOCKET, ClientSocket = INVALID_SOCKET;
    struct addrinfo* result = NULL, hints;
    int iResult;
    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;
    string sendbuf;
    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        std::cerr << "WSAStartup failed: " << iResult << std::endl;
        return 1;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // Resolve the server address and port
    iResult = getaddrinfo("0.0.0.0", DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        std::cerr << "getaddrinfo failed: " << iResult << std::endl;
        WSACleanup();
        return 1;
    }

    // Create a socket for connecting to server
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        std::cerr << "Socket failed: " << WSAGetLastError() << std::endl;
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    // Bind the socket
    iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        std::cerr << "Bind failed: " << WSAGetLastError() << std::endl;
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    freeaddrinfo(result);

    // Listen on the socket
    iResult = listen(ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        std::cerr << "Listen failed: " << WSAGetLastError() << std::endl;
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }
    else {
        cout << "Listening to client";
    }
    // Accept a client socket
    ClientSocket = accept(ListenSocket, NULL, NULL);
    if (ClientSocket == INVALID_SOCKET) {
        std::cerr << "Accept failed: " << WSAGetLastError() << std::endl;
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }
    else {
        cout << "Connected to client: " <<ClientSocket << endl;
    }
    bool running = true;
    while (running) {
        iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
        if (iResult > 0) {
            std::string receivedMessage(recvbuf, iResult);
            std::cout << "Received: " << receivedMessage << std::endl;

            if (receivedMessage == "quit") {
                std::cout << "Quit message received. Shutting down server..." << std::endl;
                running = false;
            }
            else if (receivedMessage == "1") {
                // Liệt kê các cửa sổ hiển thị
                std::wstring result = ListVisibleWindows();
                std::string tmp1(result.begin(), result.end());
                sendbuf = tmp1;

                // Gửi danh sách các ứng dụng về client
                iResult = send(ClientSocket, sendbuf.c_str(), (int)sendbuf.length(), 0);
                if (iResult == SOCKET_ERROR) {
                    std::cerr << "Send failed: " << WSAGetLastError() << std::endl;
                    closesocket(ClientSocket);
                    WSACleanup();
                    return 1;
                }

                // Nhận PID từ client để tắt ứng dụng
                iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
                if (iResult > 0) {
                    std::string receivedPID(recvbuf, iResult);
                    DWORD pidToTerminate = std::stoi(receivedPID);  // Chuyển đổi PID từ chuỗi sang số

                    if (terminateAppByPID(pidToTerminate)) {
                        sendbuf = "Application terminated successfully";
                    }
                    else {
                        sendbuf = "Failed to terminate application";
                    }

                    // Gửi phản hồi về client
                    iResult = send(ClientSocket, sendbuf.c_str(), (int)sendbuf.length(), 0);
                    if (iResult == SOCKET_ERROR) {
                        std::cerr << "Send failed: " << WSAGetLastError() << std::endl;
                        closesocket(ClientSocket);
                        WSACleanup();
                        return 1;
                    }
                }
            }
            else if (receivedMessage == "2") {
                /*EnumWindows(EnumWindowsProc, 0);*/
                wstring result2 = ListServices();  // Gọi ListServices
                string tmp2 = wstringToString(result2);  // Chuyển đổi từ std::wstring sang std::string
                sendbuf = tmp2;  // Gán cho sendbuf
            }
            else if (receivedMessage == "3") {
                /*EnumWindows(EnumWindowsProc, 0);*/
                //Chup anh tu camera sau do luu vao mot file nao do
                if (imageCapture()) {
                    sendbuf = "Chup anh thanh cong";
                }
            }
            else if (receivedMessage == "4") {
                system("C:\\WINDOWS\\System32\\shutdown/s");
            }
            else if (receivedMessage == "on_app") {
                // Gửi yêu cầu đến client để cung cấp đường dẫn tới ứng dụng muốn bật
                sendbuf = "Please provide the full path of the application you want to start:";
                iResult = send(ClientSocket, sendbuf.c_str(), (int)sendbuf.length(), 0);
                if (iResult == SOCKET_ERROR) {
                    std::cerr << "Send failed: " << WSAGetLastError() << std::endl;
                    closesocket(ClientSocket);
                    WSACleanup();
                    return 1;
                }

                // Nhận đường dẫn từ client
                iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
                if (iResult > 0) {
                    std::string receivedAppPath(recvbuf, iResult);
                    std::wstring appPath = std::wstring(receivedAppPath.begin(), receivedAppPath.end()); // Chuyển đổi từ string sang wstring

                    // Khởi động ứng dụng với đường dẫn nhận được
                    if (startApplication(appPath)) {
                        sendbuf = "Application started successfully";
                    }
                    else {
                        sendbuf = "Failed to start application";
                    }

                    // Gửi phản hồi về client
                    iResult = send(ClientSocket, sendbuf.c_str(), (int)sendbuf.length(), 0);
                    if (iResult == SOCKET_ERROR) {
                        std::cerr << "Send failed: " << WSAGetLastError() << std::endl;
                        closesocket(ClientSocket);
                        WSACleanup();
                        return 1;
                    }
                }
            }
        }
        else if (iResult == SOCKET_ERROR) {
            std::cerr << "Receive failed: " << WSAGetLastError() << std::endl;
            closesocket(ListenSocket);
            WSACleanup();
            return 1;
        }
        /*cout << "Enter message: ";
        getline(cin, sendbuf);*/
        
        iResult = send(ClientSocket, sendbuf.c_str(), (int)sendbuf.length(), 0);
        
        if (iResult == SOCKET_ERROR) {
            std::cerr << "Send failed: " << WSAGetLastError() << std::endl;
            closesocket(ClientSocket);
            WSACleanup();
            return 1;
        }
    }

    // Shutdown the connection
    iResult = shutdown(ClientSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        std::cerr << "Shutdown failed: " << WSAGetLastError() << std::endl;
        closesocket(ClientSocket);
        WSACleanup();
        return 1;
    }

    // Cleanup
    closesocket(ClientSocket);
    WSACleanup();
    return 0;
}
//https://www.youtube.com/watch?v=YUjamcyuKT4