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
#include <opencv2/opencv.hpp>
#include "serverUserRegistration.h"
#include "client_tab.h"
#include "server_tab.h"
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "User32.lib")


ClientTabData clientTabData;

SOCKET clientSocket = INVALID_SOCKET; // Для з'єднання з сервером

bool decodeKey(const std::string& key, std::string& ipOut, std::string& loginOut, int port);
void connectToServer(const std::string& serverIp, int serverPort);
// Обробник подій вкладки клієнта
LRESULT CALLBACK ClientTabWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_CREATE:
        // Ініціалізація елементів вкладки
        DrawClientTab(hwnd);
        break;

    case WM_COMMAND:
        if (LOWORD(wp) == 3004) { // кнопка "Під'єднатись"
            wchar_t wKey[256], wLogin[256], wPort[256];
            GetWindowText(clientTabData.hwndKey, wKey, 256);
            GetWindowText(clientTabData.hwndLogin, wLogin, 256);
            GetWindowText(clientTabData.hwndPort, wPort, 256);

            std::wstring wsKey(wKey), wsLogin(wLogin), wsPort(wPort);
            std::string key(wsKey.begin(), wsKey.end());
            std::string login(wsLogin.begin(), wsLogin.end());
            wchar_t buffer[16];
            GetWindowText(GetDlgItem(hwnd, 3003), buffer, 16);
            int port = wcstol(buffer, NULL, 10);

            int filledFields = 0;
            if (!key.empty()) filledFields++;
            if (!login.empty()) filledFields++;
            if (port != 0) filledFields++;

            if (filledFields < 2) {
                MessageBox(hwnd, L"Потрібно заповнити хоча б два поля (ключ, логін або порт)!", L"Помилка", MB_ICONERROR);
                return 0;
            }

            std::string ip, decodedLogin;
            if (!decodeKey(key, ip, decodedLogin, port)) {
                MessageBox(hwnd, L"Невірний ключ!", L"Помилка", MB_ICONERROR);
                return 0;
            }

            if (decodedLogin != login) {
                MessageBox(hwnd, L"Логін не збігається з ключем!", L"Помилка", MB_ICONERROR);
                return 0;
            }

            //// Перевірка наявності з'єднання через FastAPI
            //std::string url = globalConfig.GetBaseUrl() + "/check_connection/";
            //std::string checkJson =
            //    "{\"serverLogin\":\"" + login +
            //    "\",\"port\":" + std::to_string(port) +
            //    ",\"serverKey\":\"" + key + "\"}";

            //std::string checkResponse;
            //if (!PostJson(url, checkJson, checkResponse) || checkResponse != "true") {
            //    MessageBox(hwnd, L"З'єднання не знайдено в базі!", L"Помилка", MB_ICONERROR);
            //    return 0;
            //}

            MessageBox(hwnd, L"Підключення успішне!", L"OK", MB_OK);

            // Оновлення з'єднання — додавання клієнта
            std::thread([=]() {
                std::string updateUrl = globalConfig.GetBaseUrl() + "/update_connection/";
                std::string updateJson =
                    "{\"port\":" + std::to_string(port) +
                    ",\"clientLogin\":\"" + login +
                    "\",\"clientIp\":\"" + currentUser.ip + "\"}";

                std::string updateResponse;
                PostJson(updateUrl, updateJson, updateResponse);
                }).detach();

                // Запуск з’єднання з сервером
                std::thread(connectToServer, ip, port).detach();
        }


        else if (LOWORD(wp) == 3005) // Кнопка "Від'єднатись"
        {
            // Перевіряємо, чи є активне з'єднання
            if (clientSocket != INVALID_SOCKET) {
                // Закриваємо сокет
                closesocket(clientSocket);
                clientSocket = INVALID_SOCKET;

                // Очищаємо ресурси Winsock
                WSACleanup();

                // Очищаємо таблицю активних підключень (якщо необхідно)
                // (Додатково, можна додати код для видалення цього з'єднання з таблиці активних з'єднань)

                // Виводимо повідомлення про успішне відключення
                MessageBox(hwnd, L"Від'єднано від сервера", L"Успіх", MB_OK);

                // Можна оновити інтерфейс (наприклад, відключити кнопки або поля вводу)
            }
            else {
                MessageBox(hwnd, L"Немає активного з'єднання", L"Помилка", MB_ICONERROR);
            }
        }

        break;

        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_NOTIFY:
    {
        LPNMHDR lpnmhdr = (LPNMHDR)lp;
        if (lpnmhdr->idFrom == 3 || lpnmhdr->code == 4)
        {
            SetDlgItemText(hwnd, 3003, bufferPort);
            SetDlgItemText(hwnd, 3001, bufferKey);
        }
    }
    break;

    default:
        return DefWindowProc(hwnd, msg, wp, lp);
    }

    return 0;
}
void connectToServer(const std::string& serverIp, int serverPort) {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == INVALID_SOCKET) {
        MessageBox(NULL, L"Не вдалося створити сокет!", L"Помилка", MB_ICONERROR);
        return;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(serverPort);
    inet_pton(AF_INET, serverIp.c_str(), &serverAddr.sin_addr);

    if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        MessageBox(NULL, L"Не вдалося підключитися до сервера!", L"Помилка", MB_ICONERROR);
        closesocket(clientSocket);
        WSACleanup();
        return;
    }

    std::atomic<bool> isRunning(true);
    std::thread mouseThread([&]() {
        POINT lastPos = { 0, 0 };
        while (isRunning) {
            POINT pos;
            GetCursorPos(&pos);
            std::this_thread::sleep_for(std::chrono::milliseconds(500));

            if (pos.x != lastPos.x || pos.y != lastPos.y) {
                int data[3] = { pos.x, pos.y, 0 }; // 0 — рух
                send(clientSocket, (char*)data, sizeof(data), 0);
                lastPos = pos;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        });

    char* buffer = new char[1920 * 1080];
    cv::namedWindow("Remote Desktop");

    while (true) {
        int imgSize = 0;
        if (recv(clientSocket, (char*)&imgSize, sizeof(imgSize), MSG_WAITALL) <= 0) break;

        std::vector<uchar> imgBuffer(imgSize);
        int totalReceived = 0;
        while (totalReceived < imgSize) {
            int bytes = recv(clientSocket, (char*)imgBuffer.data() + totalReceived, imgSize - totalReceived, 0);
            if (bytes <= 0) break;
            totalReceived += bytes;
        }

        cv::Mat img = cv::imdecode(imgBuffer, cv::IMREAD_COLOR);
        if (!img.empty()) {
            cv::imshow("Remote Desktop", img);
        }

        if (cv::waitKey(30) == 27) { // Escape
            isRunning = false;
            break;
        }
    }

    closesocket(clientSocket);
    WSACleanup();
}






// Функція для малювання елементів вкладки клієнта
void DrawClientTab(HWND hwnd) {
    CreateWindowEx(0, L"STATIC", L"Ключ:", WS_CHILD | WS_VISIBLE,
        20, 20, 100, 20, hwnd, NULL, NULL, NULL);
    clientTabData.hwndKey = CreateWindowEx(0, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER,
        130, 20, 200, 25, hwnd, (HMENU)3001, NULL, NULL);

    CreateWindowEx(0, L"STATIC", L"Логін:", WS_CHILD | WS_VISIBLE,
        20, 60, 100, 20, hwnd, NULL, NULL, NULL);
    clientTabData.hwndLogin = CreateWindowEx(0, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER,
        130, 60, 200, 25, hwnd, (HMENU)3002, NULL, NULL);

    CreateWindowEx(0, L"STATIC", L"Порт:", WS_CHILD | WS_VISIBLE,
        20, 100, 100, 20, hwnd, NULL, NULL, NULL);
    clientTabData.hwndPort = CreateWindowEx(0, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER,
        130, 100, 100, 25, hwnd, (HMENU)3003, NULL, NULL);

    // Кнопка для під'єднання
    CreateWindowEx(0, L"BUTTON", L"Під'єднатись", WS_CHILD | WS_VISIBLE | WS_BORDER,
        370, 20, 150, 25, hwnd, (HMENU)3004, NULL, NULL);
    CreateWindowEx(0, L"BUTTON", L"Від'єднатись", WS_CHILD | WS_VISIBLE | WS_BORDER,
        370, 60, 150, 25, hwnd, (HMENU)3005, NULL, NULL);  // Ідентифікатор кнопки 3005
}

// Ініціалізація вкладки клієнта
void InitClientTab(HWND hwnd) {
    // Регістрація класу вікна для вкладки
    WNDCLASS wc = { 0 };

    wc.lpfnWndProc = ClientTabWndProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = L"ClientTabClass";
    RegisterClass(&wc);

    // Створення вікна вкладки клієнта
    CreateWindowEx(0, L"ClientTabClass", L"Клієнт", WS_CHILD | WS_VISIBLE,
        0, 0, 600, 400, hwnd, NULL, NULL, NULL);
}

bool decodeKey(const std::string& key, std::string& ipOut, std::string& loginOut, int port) {
    if (key.size() < 8) return false; // Мінімум 4 байти IP (8 hex символів)

    // Порт у цифри
    std::string portStr = std::to_string(port);
    std::vector<int> portDigits;
    for (char c : portStr) {
        if (std::isdigit(c)) {
            portDigits.push_back(c - '0');
        }
    }

    // IP частина
    ipOut.clear();
    for (int i = 0; i < 4; ++i) {
        std::string hexByte = key.substr(i * 2, 2);
        int val = std::stoi(hexByte, nullptr, 16);
        int portDigit = portDigits[i % portDigits.size()];
        int original = val ^ portDigit;
        ipOut += std::to_string(original);
        if (i != 3) ipOut += ".";
    }

    // Логін частина
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

