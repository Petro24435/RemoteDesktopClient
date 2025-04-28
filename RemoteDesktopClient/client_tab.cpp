#define _CRT_SECURE_NO_WARNINGS
#include "client_tab.h"
#include "server_tab.h"
#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <fstream>
#include <sstream>
#include <string>
#include <cctype>
#include <thread>
#pragma comment(lib, "Ws2_32.lib")

std::unordered_set<int> usedPorts;
std::unordered_map<std::string, std::string> usersTable;
std::unordered_set<std::string> keysTable;

ClientTabData clientTabData;

SOCKET clientSocket = INVALID_SOCKET; // Для з'єднання з сервером

bool LoadPortsCSV(const std::string& filename);
bool LoadUsersCSV(const std::string& filename);
bool LoadKeysCSV(const std::string& filename);
bool decodeKey(const std::string& key, std::string& ipOut, std::string& loginOut, int port);
void connectToServer(const std::string& serverIp, int serverPort);
// Обробник подій вкладки клієнта
LRESULT CALLBACK ClientTabWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_CREATE:
        // Ініціалізація елементів вкладки
        DrawClientTab(hwnd);
        LoadPortsCSV("used_ports.csv");
        LoadUsersCSV("users.csv");
        LoadKeysCSV("keys.csv");
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
            wchar_t buffer[16]; // до 5 цифр + запас
            GetWindowText(GetDlgItem(hwnd, 3003), buffer, 16);
            port = wcstol(buffer, NULL, 10); // або wcstol(buffer, NULL, 10)

            // Перевірка на заповненість хоча б двох полів
            int filledFields = 0;
            if (!key.empty()) filledFields++;
            if (!login.empty()) filledFields++;
            if (port != 0) filledFields++;

            if (filledFields < 2) {
                MessageBox(hwnd, L"Потрібно заповнити хоча б два поля (ключ, логін або порт)!", L"Помилка", MB_ICONERROR);
                break;
            }

            // Декодування ключа
            std::string ip, decodedLogin;
            if (!decodeKey(key, ip, decodedLogin, port)) {
                MessageBox(hwnd, L"Невірний ключ!", L"Помилка", MB_ICONERROR);
                break;
            }

            // Перевірка, що логін і порт відповідають розкодуваним
            if (decodedLogin != login) {
                MessageBox(hwnd, L"Логін не збігається з ключем!", L"Помилка", MB_ICONERROR);
                break;
            }

            // Тепер перевіряємо тільки в таблиці active_connections.csv
            std::ifstream infile("active_connections.csv");
            std::string line;
            bool connectionFound = false;

            while (std::getline(infile, line)) {
                std::istringstream iss(line);
                std::string fileLogin, filePortStr, fileKey, fileIp, client, clientIp;
                std::getline(iss, fileLogin, ',');
                std::getline(iss, filePortStr, ',');
                std::getline(iss, fileKey, ',');
                std::getline(iss, fileIp, ',');
                std::getline(iss, client, ',');
                std::getline(iss, clientIp, ',');

                int filePort = std::stoi(filePortStr);

                // Перевірка, чи логін, порт і ключ збігаються
                if (fileLogin == login && filePort == port && fileKey == key) {
                    connectionFound = true;
                    break;  // Якщо знайшли відповідне з'єднання, виходимо з циклу
                }
            }

            infile.close();

            // Якщо знайдено з'єднання
            if (connectionFound) {
                // Успішне підключення
                MessageBox(hwnd, L"Підключення успішне!", L"OK", MB_OK);
                

                // Оновлюємо з'єднання для активного клієнта, використовуючи тільки порт
                updateConnection(hwnd, port, login, currentUser.ip); // У цьому випадку login може бути отримано через ключ або інші методи

                // Ініціація з'єднання з сервером
                std::thread connectThread(connectToServer, ip, port);
                connectThread.detach();
            }


            else {
                // Якщо з'єднання не знайдено
                MessageBox(hwnd, L"З'єднання не знайдено в таблиці активних з'єднань!", L"Помилка", MB_ICONERROR);
            }
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
void OpenConsole() {
    if (AttachConsole(ATTACH_PARENT_PROCESS) == 0) {
        AllocConsole();
    }

    FILE* fp_out;
    if (freopen_s(&fp_out, "CONOUT$", "w", stdout) != 0) {
        MessageBox(NULL, L"Не вдалося перенаправити stdout!", L"Помилка", MB_ICONERROR);
    }

    if (freopen_s(&fp_out, "CONOUT$", "w", stderr) != 0) {
        MessageBox(NULL, L"Не вдалося перенаправити stderr!", L"Помилка", MB_ICONERROR);
    }

    FILE* fp_in;
    if (freopen_s(&fp_in, "CONIN$", "r", stdin) != 0) {
        MessageBox(NULL, L"Не вдалося перенаправити stdin!", L"Помилка", MB_ICONERROR);
    }

    SetConsoleOutputCP(1251);
    SetConsoleCP(1251);

    std::ios_base::sync_with_stdio(false);
}



void connectToServer(const std::string& serverIp, int serverPort) {
    OpenConsole(); // Консоль одразу

    WSADATA wsaData;
    int wsaResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (wsaResult != 0) {
        MessageBox(NULL, L"Не вдалося ініціалізувати Winsock!", L"Помилка", MB_ICONERROR);
        return;
    }

    clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == INVALID_SOCKET) {
        MessageBox(NULL, L"Не вдалося створити сокет!", L"Помилка", MB_ICONERROR);
        WSACleanup();
        return;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(serverPort);
    int result = inet_pton(AF_INET, serverIp.c_str(), &serverAddr.sin_addr);
    if (result <= 0) {
        MessageBox(NULL, L"Невірна IP-адреса сервера!", L"Помилка", MB_ICONERROR);
        closesocket(clientSocket);
        WSACleanup();
        return;
    }

    result = connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
    if (result == SOCKET_ERROR) {
        MessageBox(NULL, L"Не вдалося підключитися до сервера!", L"Помилка", MB_ICONERROR);
        closesocket(clientSocket);
        WSACleanup();
        return;
    }

    MessageBox(NULL, L"Підключення до сервера успішне!", L"Успіх", MB_OK);

    while (true) {
        int n;
        std::cout << "Введіть число (або 0 для виходу): ";
        std::cin >> n;

        if (n == 0) {
            break;
        }

        int netN = htonl(n);
        int sendResult = send(clientSocket, (char*)&netN, sizeof(netN), 0);
        if (sendResult == SOCKET_ERROR) {
            std::cout << "Помилка відправки даних: " << WSAGetLastError() << "\n";
            break;
        }

        int netResult;
        int recvResult = recv(clientSocket, (char*)&netResult, sizeof(netResult), 0);

        if (recvResult == SOCKET_ERROR) {
            std::cout << "Помилка отримання даних: " << WSAGetLastError() << "\n";
            break;
        }
        else if (recvResult == 0) {
            std::cout << "З'єднання з сервером було закрите.\n";
            break;
        }

        int result = ntohl(netResult);
        std::cout << "Відповідь від сервера: " << result << "\n";
    }

    FreeConsole();
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

// --- Зчитування портів ---
// (Тимчасово збережено для контексту вашого питання, в разі потреби можна це включити в додаткову функціональність)
bool LoadPortsCSV(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) return false;

    usedPorts.clear();
    std::string line;
    std::getline(file, line); // Пропускаємо заголовок

    while (std::getline(file, line)) {
        if (!line.empty()) {
            try {
                int port = std::stoi(line);
                usedPorts.insert(port);
            }
            catch (...) {}
        }
    }

    return true;
}

// --- Зчитування користувачів ---
// (Тимчасово збережено для контексту вашого питання)
bool LoadUsersCSV(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) return false;

    usersTable.clear();
    std::string line;
    std::getline(file, line); // Пропускаємо заголовок

    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string login, password;

        std::getline(ss, login, ',');
        std::getline(ss, password, ',');

        // Пропускаємо порожні або числові логіни
        if (!login.empty() && std::isalpha(login[0])) {
            usersTable[login] = password;
        }
    }

    return true;
}

// --- Зчитування ключів ---
// (Тимчасово збережено для контексту вашого питання)
bool LoadKeysCSV(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) return false;

    keysTable.clear();
    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty())
            keysTable.insert(line);
    }

    return true;
}
