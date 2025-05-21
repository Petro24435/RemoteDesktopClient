#include <winsock2.h>
#include <ws2tcpip.h>
#include <fstream>
#include <sstream>
#include <cstdio>  // ��� std::remove � std::rename
#include <string>
#include "server_tab.h"
#include <iostream>
#include <string>
#include <map>
#include <curl/curl.h>

#include <windows.h>
#include <opencv2/opencv.hpp>
#include <vector>
#include <unordered_map>
#include <thread>

#pragma comment(lib, "Gdi32.lib")
#pragma comment(lib, "User32.lib")
#pragma comment(lib, "ws2_32.lib")

#define WIDTH 1920
#define HEIGHT 1080
#define SCALE 1
#define NEW_WIDTH (WIDTH / SCALE)
#define NEW_HEIGHT (HEIGHT / SCALE)

std::map<int, SOCKET> activeClients; // ���� � ���� �������, �������� � �볺������� �����
#include "serverUserRegistration.h"
int screenWidth = GetSystemMetrics(SM_CXSCREEN);
int screenHeight = GetSystemMetrics(SM_CYSCREEN);
SOCKET serverSocket;
bool serverRunning = false;  // ���� ��� ��������, �� ������ ��� ���������
void logMessage(HWND hwnd, const std::string& message);
std::unordered_map<int, bool> keyStates;

void CaptureScreen(cv::Mat& frame) {
    HDC hScreen = GetDC(NULL);
    HDC hDC = CreateCompatibleDC(hScreen);
    HBITMAP hBitmap = CreateCompatibleBitmap(hScreen, WIDTH, HEIGHT);
    SelectObject(hDC, hBitmap);
    BitBlt(hDC, 0, 0, WIDTH, HEIGHT, hScreen, 0, 0, SRCCOPY);

    BITMAPINFOHEADER bi = { sizeof(BITMAPINFOHEADER), WIDTH, -HEIGHT, 1, 24, BI_RGB };
    cv::Mat fullFrame(HEIGHT, WIDTH, CV_8UC3);
    GetDIBits(hScreen, hBitmap, 0, HEIGHT, fullFrame.data, (BITMAPINFO*)&bi, DIB_RGB_COLORS);

    cv::resize(fullFrame, frame, cv::Size(NEW_WIDTH, NEW_HEIGHT));
    cv::cvtColor(frame, frame, cv::COLOR_BGRA2BGR);

    DeleteObject(hBitmap);
    DeleteDC(hDC);
    ReleaseDC(NULL, hScreen);
}



bool initializeServer(HWND hwnd, const std::string& serverIp, int serverPort) {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        logMessage(hwnd, "WSAStartup �� ������� ������������!");
        setStatusColor(hwnd, 'r');
        return false;
    }

    serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        logMessage(hwnd, "�� ������� �������� �����!");
        setStatusColor(hwnd, 'r');
        WSACleanup();
        return false;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(serverPort);
    inet_pton(AF_INET, serverIp.c_str(), &serverAddr.sin_addr);

    int optVal = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&optVal, sizeof(optVal));

    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        logMessage(hwnd, "�� ������� ����'����� ����� �� IP � �����!");
        closesocket(serverSocket);
        WSACleanup();
        return false;
    }

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        logMessage(hwnd, "�� ������� ������ ������� ����!");
        closesocket(serverSocket);
        WSACleanup();
        return false;
    }

    return true;
}

void handleClient(HWND hwnd, SOCKET clientSocket) {
    std::atomic<bool> running = true;

    std::thread mouseRecvThread([&]() {
        struct MouseEvent {
            int x, y;
            int action; // 0 - ���, 1 - ���� ���, 2 - ������ ���
            bool isDown;
        };

        while (running) {
            MouseEvent event;
            int bytesReceived = recv(clientSocket, (char*)&event, sizeof(event), 0);

            if (bytesReceived <= 0) break; // �'������� ��������
            if(mouseAccess)
            {
                // �������� ����� ������ ������� ��� ������������� ���������
                int screenWidth = GetSystemMetrics(SM_CXSCREEN);
                int screenHeight = GetSystemMetrics(SM_CYSCREEN);

                // ������������ ����������
                int targetX = (event.x * screenWidth) / 65535;
                int targetY = (event.y * screenHeight) / 65535;

                // ������� ����
                switch (event.action) {
                case 0: // ��� ����
                    SetCursorPos(targetX, targetY);
                    break;

                case 1: // ˳��� ���
                    mouse_event(event.isDown ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_LEFTUP,
                        targetX, targetY, 0, 0);
                    break;

                case 2: // ������ ���
                    mouse_event(event.isDown ? MOUSEEVENTF_RIGHTDOWN : MOUSEEVENTF_RIGHTUP,
                        targetX, targetY, 0, 0);
                    break;
                }
            }
        }
        });

    std::thread keyboardRecvThread([&]() {
        struct KeyEvent {
            int vkCode;
            bool isPressed;
        };

        while (true) {
            KeyEvent event;
            int bytesReceived = recv(clientSocket, (char*)&event, sizeof(event), 0);

            if (bytesReceived <= 0) break; // �'������� ��������
            if (keyboardAccess)
            {
                // ������ ����������/���������� ������
                INPUT input = { 0 };
                input.type = INPUT_KEYBOARD;
                input.ki.wVk = event.vkCode;
                input.ki.dwFlags = event.isPressed ? 0 : KEYEVENTF_KEYUP;
                SendInput(1, &input, sizeof(INPUT));
            }
        }});
    // �������� ���� ����
    cv::Mat frame_raw, frame_bgr;
    std::vector<uchar> buffer;

    while (true) {
        CaptureScreen(frame_raw);
        cv::cvtColor(frame_raw, frame_bgr, cv::COLOR_BGRA2BGR);
        buffer.clear();
        cv::imencode(".jpg", frame_bgr, buffer, { cv::IMWRITE_JPEG_QUALITY, 80 });

        int imgSize = static_cast<int>(buffer.size());
        if (send(clientSocket, (char*)&imgSize, sizeof(imgSize), 0) == SOCKET_ERROR) break;
        if (send(clientSocket, reinterpret_cast<char*>(buffer.data()), imgSize, 0) == SOCKET_ERROR) break;

        Sleep(10);  // ~100 FPS
    }

    running = false;
    closesocket(clientSocket);
    if (mouseRecvThread.joinable()) mouseRecvThread.join();
    if (keyboardRecvThread.joinable()) keyboardRecvThread.join();
}


void serverThreadFunction(HWND hwnd, std::string serverLogin, int serverPort, std::string serverKey, std::string serverIp) {
    if (serverRunning) {
        logMessage(hwnd, "������ ��� ���������!");
        return;
    }

    if (!initializeServer(hwnd, serverIp, serverPort)) return;

    logMessage(hwnd, "������ ������ �� IP " + serverIp + " � ����� " + std::to_string(serverPort));
    setStatusColor(hwnd, 'g');
    serverRunning = true;

    SOCKET clientSocket;
    sockaddr_in clientAddr;
    int clientAddrSize = sizeof(clientAddr);

    while (serverRunning) {
        clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientAddrSize);
        if (clientSocket != INVALID_SOCKET) {
            activeClients[serverPort] = clientSocket;
            logMessage(hwnd, "����� �볺�� ���������� �� ����� " + std::to_string(serverPort));
            setStatusColor(hwnd, 'y');

            char loginBuffer[256] = { 0 };
            int bytesReceived = recv(clientSocket, loginBuffer, sizeof(loginBuffer) - 1, 0);
            if (bytesReceived > 0) {
                loginBuffer[bytesReceived] = '\0';
                std::string clientLogin = loginBuffer;
                SetWindowTextA(hClientEdit, clientLogin.c_str());
            }

            std::thread clientThread(handleClient, hwnd, clientSocket);
            clientThread.detach();
        }
    }

    closesocket(serverSocket);
    WSACleanup();
    serverRunning = false;
}


void addConnection(HWND hwnd, const std::string& serverLogin, int serverPort, const std::string& serverKey, const std::string& serverIp) {
    if (serverIp.empty() || serverKey.empty()) {
        logMessage(hwnd, "��������� ��������� ��� �'�������!");
        return;
    }

    std::string url = globalConfig.GetBaseUrl() + "/add_connection/";
    std::string jsonData =
        "{\"serverLogin\":\"" + serverLogin +
        "\",\"port\":" + std::to_string(serverPort) +
        ",\"serverKey\":\"" + serverKey +
        "\",\"serverIp\":\"" + serverIp + "\"}";

    std::string response;
    if (!PostJson(url, jsonData, response)) {
        logMessage(hwnd, "������� ��� ����� �� /add_connection/");
        return;
    }

    if (!serverRunning) {
        std::thread(serverThreadFunction, hwnd, serverLogin, serverPort, serverKey, serverIp).detach();
        setStatusColor(hwnd, 'y');
    }
}


void removeConnection(HWND hwnd, const std::string& serverLogin, int serverPort) {
    std::string url = globalConfig.GetBaseUrl() + "/remove_connection/";
    std::string jsonData =
        "{\"serverLogin\":\"" + serverLogin +
        "\",\"port\":" + std::to_string(serverPort) + "}";

    std::string response;
    if (!PostJson(url, jsonData, response)) {
        logMessage(hwnd, "������� ��� �������� �'�������");
        return;
    }

    if (serverRunning) {
        serverRunning = false;
        if (serverSocket != INVALID_SOCKET) {
            closesocket(serverSocket);
            serverSocket = INVALID_SOCKET;
        }
        WSACleanup();
        logMessage(hwnd, "������ ������� ��� ����� " + std::to_string(serverPort));
        setStatusColor(hwnd, '0');
    }
}




// ��������� �'�������: ������ �볺��� ����� �� ������
void updateConnection(HWND hwnd, int serverPort, const std::string& clientLogin, const std::string& clientIp) {
    std::string url = globalConfig.GetBaseUrl() + "/update_connection/";
    std::string jsonData =
        "{\"port\":" + std::to_string(serverPort) +
        ",\"clientLogin\":\"" + clientLogin +
        "\",\"clientIp\":\"" + clientIp + "\"}";
    setStatusColor(hwnd, 'y');
    std::string response;
    if (!PostJson(url, jsonData, response)) {
        logMessage(hwnd, "������� ��� �������� �'�������");
        return;
    }
}

void disconnectClient(HWND hwnd, int serverPort) {
    std::string url = globalConfig.GetBaseUrl() + "/disconnect_client/";
    std::string jsonData = "{\"port\":" + std::to_string(serverPort) + "}";

    std::string response;
    if (!PostJson(url, jsonData, response)) {
        logMessage(hwnd, "������� ��� ������ �'������� � �볺����");
        return;
    }

    auto it = activeClients.find(serverPort);
    if (it != activeClients.end()) {
        closesocket(it->second);
        activeClients.erase(it);
        logMessage(hwnd, "�볺������� ����� ��� ����� " + std::to_string(serverPort) + " �������.");
        setStatusColor(hwnd, 'g');
    }
    else {
        logMessage(hwnd, "�� �������� ��������� �볺��� ��� ����� " + std::to_string(serverPort));
    }
}





void cleanUnusedPortsAndKeys() {

    // �������� �� ������ �����
    std::set<int> usedPorts = getUsedPorts();

    std::string url = globalConfig.GetBaseUrl() + "/clean_unused_connections/";
    std::ostringstream oss;
    oss << "{\"usedPorts\":[";

    bool first = true;
    for (int port : usedPorts) {
        if (!first) oss << ",";
        oss << port;
        first = false;
    }
    oss << "]}";

    std::string response;
    if (!PostJson(url, oss.str(), response)) {
        std::cerr << "������� ��� ������� �'������" << std::endl;
    }
}







//void ProcessMouseData(char* data) {
//    int x, y, leftClick, rightClick;
//    memcpy(&x, data, sizeof(int));
//    memcpy(&y, data + sizeof(int), sizeof(int));
//    memcpy(&leftClick, data + 2 * sizeof(int), sizeof(int));
//    memcpy(&rightClick, data + 3 * sizeof(int), sizeof(int));
//    // ������ ���������� ����
//    SimulateMouse(x, y, leftClick, rightClick);
//}


//  ������� ���������� ������
void SimulateKeyPress(int key, bool isPressed) {
    INPUT input = { 0 };
    input.type = INPUT_KEYBOARD;
    input.ki.wVk = key;
    input.ki.dwFlags = isPressed ? 0 : KEYEVENTF_KEYUP;
    SendInput(1, &input, sizeof(INPUT));
}


size_t WriteCallbackController(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t totalSize = size * nmemb;
    output->append((char*)contents, totalSize);
    return totalSize;
}

bool PostJson(const std::string& url, const std::string& jsonData, std::string& response) {
    CURL* curl = curl_easy_init();
    if (!curl) return false;

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonData.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallbackController);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(curl);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    return (res == CURLE_OK);
}
void logMessage(HWND hwnd, const std::string& message) {
    //std::ofstream logFile("server_log.txt", std::ios::app);
    //if (logFile.is_open()) {
    //    logFile << message << std::endl;
    //    logFile.close();
    //}
    if (hwnd != NULL) {
        HWND hStatusEdit = hLogEdit;
        if (hStatusEdit) {
            int size_needed = MultiByteToWideChar(CP_ACP, 0, message.c_str(), -1, NULL, 0);
            std::wstring wMessage(size_needed, 0);
            MultiByteToWideChar(CP_ACP, 0, message.c_str(), -1, &wMessage[0], size_needed);
            SetWindowTextW(hStatusEdit, wMessage.c_str());
        }
    }
}