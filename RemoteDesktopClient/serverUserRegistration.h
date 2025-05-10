#ifndef SERVERUSERREGISTRATION_H
#define SERVERUSERREGISTRATION_H

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <string>

// ��������� ������������
struct ServerConfig {
    std::string server_ip = "192.168.0.200";//"172.20.10.5";   // IP �������
    int port = 8000;                        // ���� �������

    std::string GetBaseUrl() const {
        return "http://" + server_ip + ":" + std::to_string(port);
    }
};
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output);

// ��������� ��������� �������� �����, ��� ���� ����������������� ��� �񳺿 ��������
extern ServerConfig globalConfig;
bool PostJson(const std::string& url, const std::string& jsonData, std::string& response);
extern HINSTANCE hInst;
#endif // SERVERUSERREGISTRATION_H

