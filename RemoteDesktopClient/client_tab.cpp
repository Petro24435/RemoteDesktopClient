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
//#include "server_tab.h"
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "User32.lib")


ClientTabData clientTabData;
extern ClientInfo currentClient;
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
        if (LOWORD(wp) == 3014) { // кнопка "Під'єднатись"
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

            //MessageBox(hwnd, L"Підключення успішне!", L"OK", MB_OK);

            // Оновлення з'єднання — додавання клієнта
            std::thread([=]() {
                std::string updateUrl = globalConfig.GetBaseUrl() + "/update_connection/";
                std::string updateJson =
                    "{\"port\":" + std::to_string(port) +
                    ",\"clientLogin\":\"" + login +
                    "\",\"clientIp\":\"" + currentClient.ip + "\"}";

                std::string updateResponse;
                PostJson(updateUrl, updateJson, updateResponse);
                }).detach();

                // Запуск з’єднання з сервером
                std::thread(connectToServer, ip, port).detach();
        }


        else if (LOWORD(wp) == 3015) // Кнопка "Від'єднатись"
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
        POINT lastPos = { -1, -1 }; // Щоб точно відправився перший рух
        bool lButtonPrev = false;
        bool rButtonPrev = false;

        int screenWidth = GetSystemMetrics(SM_CXSCREEN);
        int screenHeight = GetSystemMetrics(SM_CYSCREEN);

        while (isRunning) {
            POINT pos;
            GetCursorPos(&pos);

            SHORT lState = GetAsyncKeyState(VK_LBUTTON);
            SHORT rState = GetAsyncKeyState(VK_RBUTTON);

            bool lButtonNow = (lState & 0x8000);
            bool rButtonNow = (rState & 0x8000);

            bool moved = (pos.x != lastPos.x || pos.y != lastPos.y);
            bool lChanged = (lButtonNow != lButtonPrev);
            bool rChanged = (rButtonNow != rButtonPrev);

            if (moved || lChanged || rChanged) {
                float normX = (float)pos.x / screenWidth;
                float normY = (float)pos.y / screenHeight;

                // Пакет: тип (1 byte), normX (4 bytes), normY (4 bytes)
                // тип: 0 - move, 1 - L down, 2 - L up, 3 - R down, 4 - R up
                uint8_t action = 0;

                if (lChanged) {
                    action = lButtonNow ? 1 : 2;
                }
                else if (rChanged) {
                    action = rButtonNow ? 3 : 4;
                }
                else if (moved) {
                    action = 0;
                }

                char buffer[9];
                buffer[0] = action;
                memcpy(buffer + 1, &normX, sizeof(float));
                memcpy(buffer + 5, &normY, sizeof(float));

                send(clientSocket, buffer, sizeof(buffer), 0);

                lastPos = pos;
                lButtonPrev = lButtonNow;
                rButtonPrev = rButtonNow;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(15));
        }
        });

}

// Функція для малювання елементів вкладки клієнта
void DrawClientTab(HWND hwnd) {
    CreateWindowEx(0, L"STATIC", L"Ключ:", WS_CHILD | WS_VISIBLE,
        20, 20, 100, 20, hwnd, NULL, NULL, NULL);
    clientTabData.hwndKey = CreateWindowEx(0, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER,
        130, 20, 200, 25, hwnd, (HMENU)3011, NULL, NULL);

    CreateWindowEx(0, L"STATIC", L"Логін:", WS_CHILD | WS_VISIBLE,
        20, 60, 100, 20, hwnd, NULL, NULL, NULL);
    clientTabData.hwndLogin = CreateWindowEx(0, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER,
        130, 60, 200, 25, hwnd, (HMENU)3012, NULL, NULL);

    CreateWindowEx(0, L"STATIC", L"Порт:", WS_CHILD | WS_VISIBLE,
        20, 100, 100, 20, hwnd, NULL, NULL, NULL);
    clientTabData.hwndPort = CreateWindowEx(0, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER,
        130, 100, 100, 25, hwnd, (HMENU)3013, NULL, NULL);

    // Кнопка для під'єднання
    CreateWindowEx(0, L"BUTTON", L"Під'єднатись", WS_CHILD | WS_VISIBLE | WS_BORDER,
        370, 20, 150, 25, hwnd, (HMENU)3014, NULL, NULL);
    CreateWindowEx(0, L"BUTTON", L"Від'єднатись", WS_CHILD | WS_VISIBLE | WS_BORDER,
        370, 60, 150, 25, hwnd, (HMENU)3015, NULL, NULL);  // Ідентифікатор кнопки 3005
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

