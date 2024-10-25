#include "client.h"
#include <iostream>

Client::Client() : ConnectSocket(INVALID_SOCKET) {}

Client::~Client() {
    if (ConnectSocket != INVALID_SOCKET) {
        closesocket(ConnectSocket);
        WSACleanup();
    }
}

void Client::setServerIP(const std::string& ip) {
    serverIP = ip;
}

bool Client::connectToServer() {
    WSADATA wsaData;
    struct addrinfo* result = NULL, hints;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) return false;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    if (getaddrinfo(serverIP.c_str(), port.c_str(), &hints, &result) != 0) {
        WSACleanup();
        return false;
    }

    ConnectSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ConnectSocket == INVALID_SOCKET || connect(ConnectSocket, result->ai_addr, (int)result->ai_addrlen) == SOCKET_ERROR) {
        freeaddrinfo(result);
        WSACleanup();
        return false;
    }

    freeaddrinfo(result);
    return true;
}

std::string Client::sendMessage(const std::string& message) {
    send(ConnectSocket, message.c_str(), (int)message.length(), 0);

    char recvbuf[512];
    int recvResult = recv(ConnectSocket, recvbuf, 512, 0);
    if (recvResult > 0) {
        return std::string(recvbuf, recvResult);
    }
    return "No response or connection lost.";
}
