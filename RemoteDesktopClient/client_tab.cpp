#define _CRT_SECURE_NO_WARNINGS

#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <fstream>
#include <sstream>
#include <string>
#include <cctype>
#include <thread>
#include <atomic>
#include <chrono>
#include <opencv2/opencv.hpp>
#include "serverUserRegistration.h"
#include "client_tab.h"
#include "user.h"
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "User32.lib")


ClientTabData clientTabData;
extern ClientInfo currentClient;
SOCKET clientSocket = INVALID_SOCKET; // ��� �'������� � ��������

bool decodeKey(const std::string& key, std::string& ipOut, std::string& loginOut, int port);
void connectToServer(const std::string& serverIp, int serverPort);
// �������� ���� ������� �볺���
LRESULT CALLBACK ClientTabWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_CREATE:
        // ����������� �������� �������
        DrawClientTab(hwnd);
        break;

    case WM_COMMAND:
        if (LOWORD(wp) == 3014) { // ������ "ϳ�'��������"
            wchar_t wKey[256], wLogin[256], wPort[256];
            GetWindowText(clientTabData.hwndKey, wKey, 256);
            GetWindowText(clientTabData.hwndLogin, wLogin, 256);
            GetWindowText(clientTabData.hwndPort, wPort, 256);

            std::wstring wsKey(wKey), wsLogin(wLogin), wsPort(wPort);
            std::string key(wsKey.begin(), wsKey.end());
            std::string login(wsLogin.begin(), wsLogin.end());
            wchar_t buffer[16];
            GetWindowText(GetDlgItem(hwnd, 3013), buffer, 16);
            int port = wcstol(buffer, NULL, 10);

            int filledFields = 0;
            if (!key.empty()) filledFields++;
            if (!login.empty()) filledFields++;
            if (port != 0) filledFields++;

            if (filledFields < 2) {
                MessageBox(hwnd, L"������� ��������� ���� � ��� ���� (����, ���� ��� ����)!", L"�������", MB_ICONERROR);
                return 0;
            }

            std::string ip, decodedLogin;
            if (!decodeKey(key, ip, decodedLogin, port)) {
                MessageBox(hwnd, L"������� ����!", L"�������", MB_ICONERROR);
                return 0;
            }

            if (decodedLogin != login) {
                MessageBox(hwnd, L"���� �� �������� � ������!", L"�������", MB_ICONERROR);
                return 0;
            }

            //MessageBox(hwnd, L"ϳ��������� ������!", L"OK", MB_OK);

            // ��������� �'������� � ��������� �볺���
            std::thread([=]() {
                std::string updateUrl = globalConfig.GetBaseUrl() + "/update_connection/";
                std::string updateJson =
                    "{\"port\":" + std::to_string(port) +
                    ",\"clientLogin\":\"" + login +
                    "\",\"clientIp\":\"" + currentClient.ip + "\"}";

                std::string updateResponse;
                PostJson(updateUrl, updateJson, updateResponse);
                }).detach();

                // ������ 璺������ � ��������
                std::thread(connectToServer, ip, port).detach();
        }


        else if (LOWORD(wp) == 3015) // ������ "³�'��������"
        {
            // ����������, �� � ������� �'�������
            if (clientSocket != INVALID_SOCKET) {
                // ��������� �����
                closesocket(clientSocket);
                clientSocket = INVALID_SOCKET;

                // ������� ������� Winsock
                WSACleanup();

                // ������� ������� �������� ��������� (���� ���������)
                // (���������, ����� ������ ��� ��� ��������� ����� �'������� � ������� �������� �'������)

                // �������� ����������� ��� ������ ����������
                MessageBox(hwnd, L"³�'������ �� �������", L"����", MB_OK);

                // ����� ������� ��������� (���������, ��������� ������ ��� ���� �����)
            }
            else {
                MessageBox(hwnd, L"���� ��������� �'�������", L"�������", MB_ICONERROR);
            }
        }

        break;

        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;
        break;

    default:
        return DefWindowProc(hwnd, msg, wp, lp);
    }

    return 0;
}
void connectToServer(const std::string& serverIp, int serverPort) {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        MessageBox(NULL, L"������� ����������� Winsock!", L"�������", MB_ICONERROR);
        return;
    }

    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == INVALID_SOCKET) {
        MessageBox(NULL, L"�� ������� �������� �����!", L"�������", MB_ICONERROR);
        WSACleanup();
        return;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(serverPort);
    inet_pton(AF_INET, serverIp.c_str(), &serverAddr.sin_addr);

    if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        MessageBox(NULL, L"�� ������� ����������� �� �������!", L"�������", MB_ICONERROR);
        closesocket(clientSocket);
        WSACleanup();
        return;
    }

    std::atomic<bool> isRunning(true);

    // ��������� ����
    send(clientSocket, currentUser.login.c_str(), currentUser.login.size(), 0);

    const std::string windowName = "Remote Screen";

    cv::namedWindow(windowName, cv::WINDOW_NORMAL);
    cv::resizeWindow(windowName, 1280, 720);

    // ���� �������� ����
    std::thread mouseThread([&]() {
        POINT lastPos = { -1, -1 };
        bool lButtonPrev = false;
        bool rButtonPrev = false;

        while (isRunning) {
            POINT cursorPos;
            GetCursorPos(&cursorPos);

            HWND hwnd = FindWindowA(NULL, windowName.c_str());
            if (!hwnd) continue;

            RECT windowRect;
            GetWindowRect(hwnd, &windowRect);

            RECT clientRect;
            GetClientRect(hwnd, &clientRect);
            int winW = clientRect.right;
            int winH = clientRect.bottom;

            // ����������� ������� � ���������� ����
            int x = cursorPos.x - windowRect.left;
            int y = cursorPos.y - windowRect.top - (GetSystemMetrics(SM_CYCAPTION)); // ���������� ���������

            if (x < 0 || y < 0 || x >= winW || y >= winH) {
                std::this_thread::sleep_for(std::chrono::milliseconds(15));
                continue;
            }

            SHORT lState = GetAsyncKeyState(VK_LBUTTON);
            SHORT rState = GetAsyncKeyState(VK_RBUTTON);

            bool lButtonNow = (lState & 0x8000);
            bool rButtonNow = (rState & 0x8000);

            bool moved = (x != lastPos.x || y != lastPos.y);
            bool lChanged = (lButtonNow != lButtonPrev);
            bool rChanged = (rButtonNow != rButtonPrev);

            if (moved || lChanged || rChanged) {
                float normX = (float)x / winW;
                float normY = (float)y / winH;

                uint8_t action = 0;
                if (lChanged) action = lButtonNow ? 1 : 2;
                else if (rChanged) action = rButtonNow ? 3 : 4;
                else if (moved) action = 0;

                char buffer[9];
                buffer[0] = action;
                memcpy(buffer + 1, &normX, sizeof(float));
                memcpy(buffer + 5, &normY, sizeof(float));

                send(clientSocket, buffer, sizeof(buffer), 0);

                lastPos = { x, y };
                lButtonPrev = lButtonNow;
                rButtonPrev = rButtonNow;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(15));
        }
        });

    // ���� ������� ����������
    std::thread imageThread([&]() {
        using namespace std::chrono;
        auto lastTime = high_resolution_clock::now();
        int frameCount = 0;
        float fps = 0.0f;

        const std::string windowName = "Remote Screen";
        cv::namedWindow(windowName, cv::WINDOW_NORMAL);

        HWND hwnd = FindWindowA(NULL, windowName.c_str());
        if (hwnd != NULL) {
            SetWindowPos(hwnd, HWND_TOPMOST, 100, 100, 1280, 720, SWP_SHOWWINDOW);
        }

        while (isRunning) {
            int imgSize = 0;
            int received = recv(clientSocket, (char*)&imgSize, sizeof(imgSize), MSG_WAITALL);
            imgSize = ntohl(imgSize);
            if (received != sizeof(imgSize) || imgSize <= 0) {
                isRunning = false;
                break;
            }

            std::vector<uchar> imgData(imgSize);
            int total = 0;
            while (total < imgSize) {
                int bytes = recv(clientSocket, (char*)imgData.data() + total, imgSize - total, 0);
                if (bytes <= 0) {
                    isRunning = false;
                    break;
                }
                total += bytes;
            }

            if (!isRunning) break;
            cv::Mat img = cv::imdecode(imgData, cv::IMREAD_COLOR);
            if (img.empty()) continue;

            // FPS
            frameCount++;
            auto now = high_resolution_clock::now();
            auto duration = duration_cast<milliseconds>(now - lastTime);
            if (duration.count() >= 1000) {
                fps = frameCount * 1000.0f / duration.count();
                frameCount = 0;
                lastTime = now;
            }

            // ������� FPS
            std::string fpsText = "FPS: " + std::to_string(static_cast<int>(fps));
            cv::putText(img, fpsText, cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 0.9, cv::Scalar(0, 255, 0), 2);

            // ³��������� ���������� ��� �������������
            cv::imshow(windowName, img);
            int key = cv::waitKey(1);

            if (key == 27) {
                isRunning = false;
                break;
            }
        }

        cv::destroyWindow(windowName);
        });

    // ������� ��������� join ��� ��� ������
    imageThread.detach();
    mouseThread.detach();

    // �������� ��� ���������� ���������� ������
    while (isRunning) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    closesocket(clientSocket);
    WSACleanup();
}








// ������� ��� ��������� �������� ������� �볺���
void DrawClientTab(HWND hwnd) {
    CreateWindowEx(0, L"STATIC", L"����:", WS_CHILD | WS_VISIBLE,
        20, 20, 100, 20, hwnd, NULL, NULL, NULL);
    clientTabData.hwndKey = CreateWindowEx(0, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER,
        130, 20, 200, 25, hwnd, (HMENU)3011, NULL, NULL);

    CreateWindowEx(0, L"STATIC", L"����:", WS_CHILD | WS_VISIBLE,
        20, 60, 100, 20, hwnd, NULL, NULL, NULL);
    clientTabData.hwndLogin = CreateWindowEx(0, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER,
        130, 60, 200, 25, hwnd, (HMENU)3012, NULL, NULL);

    CreateWindowEx(0, L"STATIC", L"����:", WS_CHILD | WS_VISIBLE,
        20, 100, 100, 20, hwnd, NULL, NULL, NULL);
    clientTabData.hwndPort = CreateWindowEx(0, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER,
        130, 100, 100, 25, hwnd, (HMENU)3013, NULL, NULL);

    // ������ ��� ��'�������
    CreateWindowEx(0, L"BUTTON", L"ϳ�'��������", WS_CHILD | WS_VISIBLE | WS_BORDER,
        370, 20, 150, 25, hwnd, (HMENU)3014, NULL, NULL);
    CreateWindowEx(0, L"BUTTON", L"³�'��������", WS_CHILD | WS_VISIBLE | WS_BORDER,
        370, 60, 150, 25, hwnd, (HMENU)3015, NULL, NULL);  // ������������� ������ 3005
}

// ����������� ������� �볺���
void InitClientTab(HWND hwnd) {
    // ���������� ����� ���� ��� �������
    WNDCLASS wc = { 0 };

    wc.lpfnWndProc = ClientTabWndProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = L"ClientTabClass";
    RegisterClass(&wc);

    // ��������� ���� ������� �볺���
    CreateWindowEx(0, L"ClientTabClass", L"�볺��", WS_CHILD | WS_VISIBLE,
        0, 0, 600, 400, hwnd, NULL, NULL, NULL);
}

bool decodeKey(const std::string& key, std::string& ipOut, std::string& loginOut, int port) {
    if (key.size() < 8) return false; // ̳���� 4 ����� IP (8 hex �������)

    // ���� � �����
    std::string portStr = std::to_string(port);
    std::vector<int> portDigits;
    for (char c : portStr) {
        if (std::isdigit(c)) {
            portDigits.push_back(c - '0');
        }
    }

    // IP �������
    ipOut.clear();
    for (int i = 0; i < 4; ++i) {
        std::string hexByte = key.substr(i * 2, 2);
        int val = std::stoi(hexByte, nullptr, 16);
        int portDigit = portDigits[i % portDigits.size()];
        int original = val ^ portDigit;
        ipOut += std::to_string(original);
        if (i != 3) ipOut += ".";
    }

    // ���� �������
    loginOut.clear();
    for (size_t i = 8; i + 1 < key.size(); i += 2) {
        std::string hexByte = key.substr(i, 2);
        int val = std::stoi(hexByte, nullptr, 16);
        int portDigit = portDigits[((i - 8) / 2) % portDigits.size()];
        char originalChar = static_cast<char>(val ^ portDigit);
        loginOut += originalChar;
    }

    return true;
}

