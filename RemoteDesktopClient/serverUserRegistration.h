#ifndef SERVERUSERREGISTRATION_H
#define SERVERUSERREGISTRATION_H

#include <string>

// ��������� ������������
struct ServerConfig {
    std::string server_ip = "192.168.0.200";   // IP �������
    int port = 8000;                        // ���� �������

    std::string GetBaseUrl() const {
        return "http://" + server_ip + ":" + std::to_string(port);
    }
};
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output);

// ��������� ��������� �������� �����, ��� ���� ����������������� ��� �񳺿 ��������
extern ServerConfig globalConfig;

#endif // SERVERUSERREGISTRATION_H

