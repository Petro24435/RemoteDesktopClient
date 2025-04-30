#include <winsock2.h>
#include <ws2tcpip.h>
#include <fstream>
#include <sstream>
#include <cstdio>  // для std::remove і std::rename
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
#define SCALE 1.5
#define NEW_WIDTH (WIDTH / SCALE)
#define NEW_HEIGHT (HEIGHT / SCALE)
#define PORT 12345
std::map<int, SOCKET> activeClients; // ключ — порт сервера, значення — клієнтський сокет
#include "serverUserRegistration.h"

SOCKET serverSocket;
bool serverRunning = false;  // Флаг для перевірки, чи сервер вже запущений

std::unordered_map<int, bool> keyStates;

//  Зняття скріншоту
void CaptureScreen(cv::Mat& frame) {
    HDC hScreen = GetDC(NULL);
    HDC hDC = CreateCompatibleDC(hScreen);
    HBITMAP hBitmap = CreateCompatibleBitmap(hScreen, WIDTH, HEIGHT);
    SelectObject(hDC, hBitmap);
    BitBlt(hDC, 0, 0, WIDTH, HEIGHT, hScreen, 0, 0, SRCCOPY);

    BITMAPINFOHEADER bi = { sizeof(BITMAPINFOHEADER), WIDTH, -HEIGHT, 1, 32, BI_RGB };
    cv::Mat raw(HEIGHT, WIDTH, CV_8UC4);
    GetDIBits(hScreen, hBitmap, 0, HEIGHT, raw.data, (BITMAPINFO*)&bi, DIB_RGB_COLORS);

    cv::resize(raw, frame, cv::Size(NEW_WIDTH, NEW_HEIGHT));

    DeleteObject(hBitmap);
    DeleteDC(hDC);
    ReleaseDC(NULL, hScreen);
}


// Приймаємо координати миші від клієнта та коригуємо їх
void SimulateMouse(int x, int y, int leftClick, int rightClick) {
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    int absX = (x * 65535) / screenWidth;
    int absY = (y * 65535) / screenHeight;

    // Переміщення курсора
    INPUT input = { 0 };
    input.type = INPUT_MOUSE;
    input.mi.dx = absX;
    input.mi.dy = absY;
    input.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;
    SendInput(1, &input, sizeof(INPUT));

    // ЛКМ
    if (leftClick) {
        input.mi.dx = 0;
        input.mi.dy = 0;
        input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
        SendInput(1, &input, sizeof(INPUT));
        input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
        SendInput(1, &input, sizeof(INPUT));
    }

    // ПКМ
    if (rightClick) {
        input.mi.dx = 0;
        input.mi.dy = 0;
        input.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN;
        SendInput(1, &input, sizeof(INPUT));
        input.mi.dwFlags = MOUSEEVENTF_RIGHTUP;
        SendInput(1, &input, sizeof(INPUT));
    }
}


void ProcessMouseData(char* data) {
    int x, y, leftClick, rightClick;
    memcpy(&x, data, sizeof(int));
    memcpy(&y, data + sizeof(int), sizeof(int));
    memcpy(&leftClick, data + 2 * sizeof(int), sizeof(int));
    memcpy(&rightClick, data + 3 * sizeof(int), sizeof(int));
    // Імітуємо натискання миші
    SimulateMouse(x, y, leftClick, rightClick);
}


//  Імітація натискання клавіші
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
        HWND hStatusEdit = GetDlgItem(hwnd, 4009);
        if (hStatusEdit) {
            int size_needed = MultiByteToWideChar(CP_ACP, 0, message.c_str(), -1, NULL, 0);
            std::wstring wMessage(size_needed, 0);
            MultiByteToWideChar(CP_ACP, 0, message.c_str(), -1, &wMessage[0], size_needed);
            SetWindowTextW(hStatusEdit, wMessage.c_str());
        }
    }
}

// Функція для безпечного отримання всіх байтів

bool recvAll(SOCKET socket, char* buffer, int totalBytes) {
    int bytesReceived = 0;
    while (bytesReceived < totalBytes) {
        int result = recv(socket, buffer + bytesReceived, totalBytes - bytesReceived, 0);
        if (result <= 0) {
            return false; // Помилка або закриття з'єднання
        }
        bytesReceived += result;
    }
    return true;
}

// Функція для безпечного надсилання всіх байтів
bool sendAll(SOCKET socket, const char* data, int totalBytes) {
    int bytesSent = 0;
    while (bytesSent < totalBytes) {
        int result = send(socket, data + bytesSent, totalBytes - bytesSent, 0);
        if (result == SOCKET_ERROR) {
            return false;
        }
        bytesSent += result;
    }
    return true;
}

void handleClient(HWND hwnd, SOCKET clientSocket) {
    // Отримуємо тип клієнта (не обов'язково, якщо тільки один тип)
    char clientType[16] = { 0 };
    int received = recv(clientSocket, clientType, sizeof(clientType) - 1, 0);
    if (received <= 0) {
        closesocket(clientSocket);
        return;
    }

    cv::Mat frame_raw, frame_bgr;
    std::vector<uchar> buffer;

    while (true) {
        // Захоплення і кодування зображення
        CaptureScreen(frame_raw);
        cv::cvtColor(frame_raw, frame_bgr, cv::COLOR_BGRA2BGR);  // OpenCV працює з BGR
        buffer.clear();
        cv::imencode(".jpg", frame_bgr, buffer, { cv::IMWRITE_JPEG_QUALITY, 80 });

        // Відправляємо розмір та зображення
        int imgSize = static_cast<int>(buffer.size());
        if (send(clientSocket, (char*)&imgSize, sizeof(imgSize), 0) == SOCKET_ERROR) break;
        if (send(clientSocket, reinterpret_cast<char*>(buffer.data()), imgSize, 0) == SOCKET_ERROR) break;

        // Прийом команд
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(clientSocket, &readfds);
        timeval timeout{ 0, 1000 };  // 1 мс

        if (select(0, &readfds, nullptr, nullptr, &timeout) > 0) {
            int data[4];
            int bytes = recv(clientSocket, (char*)data, sizeof(data), 0);
            if (bytes == sizeof(data)) {
                if (data[1] == 0 || data[1] == 1)
                    SimulateKeyPress(data[0], data[1]);  // клавіатура
                else
                    SimulateMouse(data[0], data[1], data[2], data[3]);  // миша
            }
        }

        Sleep(10);  // обмежуємо FPS ~100
    }

    closesocket(clientSocket);
}







// Функція для ініціалізації сервера
bool initializeServer(HWND hwnd, const std::string& serverIp, int serverPort) {
    WSADATA wsaData;
    int wsaResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (wsaResult != 0) {
        logMessage(hwnd, "WSAStartup не вдалося ініціалізувати!");
        setStatusColor(hwnd, 'r'); // Червоний
        return false;
    }

    serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        logMessage(hwnd, "Не вдалося створити сокет!");
        setStatusColor(hwnd, 'r'); // Червоний
        WSACleanup();
        return false;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(serverPort);
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    int result = inet_pton(AF_INET, serverIp.c_str(), &serverAddr.sin_addr);
    if (result <= 0) {
        logMessage(hwnd, "Невірна IP-адреса!");
        setStatusColor(hwnd, '0');
        closesocket(serverSocket);
        WSACleanup();
        return false;
    }
    int optVal = 1;
    int optLen = sizeof(optVal);
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&optVal, optLen);
    int bindResult = bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    if (bindResult == SOCKET_ERROR) {
        logMessage(hwnd, "Не вдалося прив'язати сокет до IP і порту!");
        closesocket(serverSocket);
        WSACleanup();
        return false;
    }

    int listenResult = listen(serverSocket, SOMAXCONN);
    if (listenResult == SOCKET_ERROR) {
        logMessage(hwnd, "Не вдалося почати слухати порт!");
        closesocket(serverSocket);
        WSACleanup();
        return false;
    }

    return true;
}

// Функція для запуску сервера
void serverThreadFunction(HWND hwnd, std::string serverLogin, int serverPort, std::string serverKey, std::string serverIp) {
    if (serverRunning) {
        logMessage(hwnd, "Сервер вже запущений!");
        return;  // Якщо сервер вже працює, не запускати знову
    }

    // Ініціалізація сервера
    if (!initializeServer(hwnd, serverIp, serverPort)) {
        return; // Якщо ініціалізація не вдалася, припиняємо запуск
    }

    logMessage(hwnd, "Сервер працює на IP " + serverIp + " і порту " + std::to_string(serverPort));
    setStatusColor(hwnd, 'g'); // Зелений
    serverRunning = true;

    SOCKET clientSocket;
    sockaddr_in clientAddr;
    int clientAddrSize = sizeof(clientAddr);

    while (serverRunning) {
        clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrSize);
        if (clientSocket != INVALID_SOCKET) {
            // Додаємо клієнта в активні клієнти
            activeClients[serverPort] = clientSocket;
            logMessage(hwnd, "Новий клієнт підключився до порту " + std::to_string(serverPort));
            setStatusColor(hwnd, 'y');

            // Далі вже обробка логіну, наприклад:
            char loginBuffer[256] = { 0 };
            int bytesReceived = recv(clientSocket, loginBuffer, sizeof(loginBuffer) - 1, 0);
            if (bytesReceived > 0) {
                loginBuffer[bytesReceived] = '\0';
                std::string clientLogin = loginBuffer;
                SetWindowTextA(GetDlgItem(hwnd, 4006), clientLogin.c_str());
            }

            // Створення окремого потоку на обробку
            std::thread clientThread(handleClient, hwnd, clientSocket);
            clientThread.detach();
        }

    }

    // Завершення роботи сервера
    closesocket(serverSocket);
    WSACleanup();
    serverRunning = false;
}

void addConnection(HWND hwnd, const std::string& serverLogin, int serverPort, const std::string& serverKey, const std::string& serverIp) {
    if (serverIp.empty() || serverKey.empty()) {
        logMessage(hwnd, "Некоректні параметри для з'єднання!");
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
        logMessage(hwnd, "Помилка при запиті на /add_connection/");
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
        logMessage(hwnd, "Помилка при видаленні з'єднання");
        return;
    }

    if (serverRunning) {
        serverRunning = false;
        if (serverSocket != INVALID_SOCKET) {
            closesocket(serverSocket);
            serverSocket = INVALID_SOCKET;
        }
        WSACleanup();
        logMessage(hwnd, "Сервер закрито для порту " + std::to_string(serverPort));
        setStatusColor(hwnd, 'g');
    }
}




// Оновлюємо з'єднання: додаємо клієнта тільки за портом
void updateConnection(HWND hwnd, int serverPort, const std::string& clientLogin, const std::string& clientIp) {
    std::string url = globalConfig.GetBaseUrl() + "/update_connection/";
    std::string jsonData =
        "{\"port\":" + std::to_string(serverPort) +
        ",\"clientLogin\":\"" + clientLogin +
        "\",\"clientIp\":\"" + clientIp + "\"}";
    setStatusColor(hwnd, 'y');
    std::string response;
    if (!PostJson(url, jsonData, response)) {
        logMessage(hwnd, "Помилка при оновленні з'єднання");
        return;
    }
}

void disconnectClient(HWND hwnd, int serverPort) {
    std::string url = globalConfig.GetBaseUrl() + "/disconnect_client/";
    std::string jsonData = "{\"port\":" + std::to_string(serverPort) + "}";

    std::string response;
    if (!PostJson(url, jsonData, response)) {
        logMessage(hwnd, "Помилка при розриві з'єднання з клієнтом");
        return;
    }

    auto it = activeClients.find(serverPort);
    if (it != activeClients.end()) {
        closesocket(it->second);
        activeClients.erase(it);
        logMessage(hwnd, "Клієнтський сокет для порту " + std::to_string(serverPort) + " закрито.");
        setStatusColor(hwnd, 'g');
    }
    else {
        logMessage(hwnd, "Не знайдено активного клієнта для порту " + std::to_string(serverPort));
    }
}





void cleanUnusedPortsAndKeys() {

    // Отримати всі зайняті порти
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
        std::cerr << "Помилка при очищенні з'єднань" << std::endl;
    }
}


