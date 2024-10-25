#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_PORT "8081"
#define DEFAULT_BUFLEN 512

int main() {
    WSADATA wsaData;
    SOCKET ConnectSocket = INVALID_SOCKET;
    struct addrinfo* result = NULL, * ptr = NULL, hints;
    char recvbuf[DEFAULT_BUFLEN];
    int iResult;
    int recvbuflen = DEFAULT_BUFLEN;
    std::string sendbuf;

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

    // Resolve the server address and port
    iResult = getaddrinfo("192.168.1.102", DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        std::cerr << "getaddrinfo failed: " << iResult << std::endl;
        WSACleanup();
        return 1;
    }

    // Attempt to connect to the first address returned by the call to getaddrinfo
    ptr = result;
    ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
    if (ConnectSocket == INVALID_SOCKET) {
        std::cerr << "Socket failed: " << WSAGetLastError() << std::endl;
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    // Connect to server.
    iResult = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        std::cerr << "Connect failed: " << WSAGetLastError() << std::endl;
        closesocket(ConnectSocket);
        ConnectSocket = INVALID_SOCKET;
        return 1;
    }

    freeaddrinfo(result);

    // Check if connection succeeded
    if (ConnectSocket == INVALID_SOCKET) {
        std::cerr << "Unable to connect to server!" << std::endl;
        WSACleanup();
        return 1;
    }

    // Send messages in a loop until "quit" is sent
    bool running = true;
    while (running) {
        std::cout << "\nChoose an action:\n";
        std::cout << "1. List apps\n";
        std::cout << "2. List services\n";
        std::cout << "3. Take a screenshot\n";
        std::cout << "4. Shutdown\n";
        std::cout << "5. On/Off Webcam\n";
        std::cout << "6. File operations\n";
        std::cout << "Type 'quit' to exit.\n";
        std::cout << "Enter choice: ";
        std::getline(std::cin, sendbuf);

        // Send the message to the server
        iResult = send(ConnectSocket, sendbuf.c_str(), (int)sendbuf.length(), 0);
        sendbuf.clear();
        if (iResult == SOCKET_ERROR) {
            std::cerr << "Send failed: " << WSAGetLastError() << std::endl;
            closesocket(ConnectSocket);
            WSACleanup();
            return 1;
        }

        // Quit if needed
        if (sendbuf == "quit") {
            running = false;
            break;
        }

        // Receive response from the server
        iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
        if (iResult > 0) {
            std::string serverResponse(recvbuf, iResult);
            std::cout << "Server response: " << serverResponse << std::endl;

            // Example additional handling
            if (serverResponse == "Please provide the file path:") {
                std::string filePath;
                std::cout << "Enter the file path: ";
                std::getline(std::cin, filePath);
                send(ConnectSocket, filePath.c_str(), filePath.length(), 0);
            }
        }
        else if (iResult == 0) {
            std::cout << "Connection closed by server." << std::endl;
            running = false;
        }
        else {
            std::cerr << "Recv failed: " << WSAGetLastError() << std::endl;
            closesocket(ConnectSocket);
            WSACleanup();
            return 1;
        }
    }

    // Shutdown the connection
    iResult = shutdown(ConnectSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        std::cerr << "Shutdown failed: " << WSAGetLastError() << std::endl;
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    // Cleanup
    closesocket(ConnectSocket);
    WSACleanup();

    return 0;
}
