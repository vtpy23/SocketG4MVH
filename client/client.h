#pragma once
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

class Client {
public:
    Client();
    ~Client();
    void setServerIP(const std::string& ip);
    bool connectToServer();
    std::string sendMessage(const std::string& message);

private:
    SOCKET ConnectSocket;
    std::string serverIP;
    const std::string port = "8081";
};
