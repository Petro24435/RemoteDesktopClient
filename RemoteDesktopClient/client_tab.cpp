#define _CRT_SECURE_NO_WARNINGS


#include "libraries.h"
#include "serverUserRegistration.h"
#include "client_tab.h"
#include "user.h"
#include "server_tab.h"
#include "friends_tab.h"
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "User32.lib")


ClientTabData clientTabData;
extern ClientInfo currentClient;
SOCKET clientSocket = INVALID_SOCKET; // Для з'єднання з сервером
std::atomic<bool> isRunning(true);
std::atomic<bool> blockHotkeys(false); // Чи блокувати гарячі клавіші
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
                WSACleanup();
                wchar_t buffer[16];
                GetWindowText(GetDlgItem(hwnd, 3013), buffer, 16);
                int port = wcstol(buffer, NULL, 10);
                disconnectClient(hLogEdit, port);
                MessageBox(hwnd, L"Від'єднано від сервера", L"Успіх", MB_OK);

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
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        MessageBox(NULL, L"Помилка ініціалізації Winsock!", L"Помилка", MB_ICONERROR);
        return;
    }

    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == INVALID_SOCKET) {
        MessageBox(NULL, L"Не вдалося створити сокет!", L"Помилка", MB_ICONERROR);
        WSACleanup();
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
    updateConnection(hStatusIcon, serverPort, currentUser.login.c_str(), currentUser.ip.c_str());
    
    std::atomic<bool> isRunning(true);

    // Надсилаємо логін
    send(clientSocket, currentUser.login.c_str(), currentUser.login.size(), 0);
    char loginBuffer[256] = { 0 };
    int bytesReceived = recv(clientSocket, loginBuffer, sizeof(loginBuffer) - 1, 0);
    if (bytesReceived > 0) {
        loginBuffer[bytesReceived] = '\0';
        std::string clientLogin = loginBuffer;
        AddFriendToCSV(std::wstring(clientLogin.begin(), clientLogin.end()), std::wstring(currentUser.login.begin(), currentUser.login.end()));
    }
    
    std::thread inputThread([&]() {
    // Стани миші
    POINT lastPos = { -1, -1 };
    bool lButtonDown = false;
    bool rButtonDown = false;
    bool mButtonDown = false;

    // Стани клавіатури
    bool keyStates[256] = { false };
    bool blockHotkeys = false;

    // Отримуємо розміри екрана для нормалізації
    RECT windowRect;
    GetWindowRect(GetDesktopWindow(), &windowRect);

    while (isRunning) {
        // Обробка миші
        POINT currentPos;
        GetCursorPos(&currentPos);

        bool lButtonNow = (GetAsyncKeyState(VK_LBUTTON) & 0x8000);
        bool rButtonNow = (GetAsyncKeyState(VK_RBUTTON) & 0x8000);
        bool mButtonNow = (GetAsyncKeyState(VK_MBUTTON) & 0x8000);

        // Структура для подій введення
        struct InputEvent {
            int type;       // 0 - миша (рух), 1 - миша (ліва кнопка), 2 - миша (права кнопка)
            // 3 - миша (середня кнопка), 4 - миша (колесо)
            // 10-265 - клавіатура (vkCode)
            int x;          // Координати X (для миші)
            int y;          // Координати Y (для миші)
            bool isDown;    // Стан кнопки/клавіші
            int data;       // Додаткові дані (для колеса)
        } event;

        // Перевірка зміни позиції миші
        if (currentPos.x != lastPos.x || currentPos.y != lastPos.y) {
            event.type = 0;
            event.x = (currentPos.x * 65535) / (windowRect.right - windowRect.left);
            event.y = (currentPos.y * 65535) / (windowRect.bottom - windowRect.top);
            event.isDown = false;
            send(clientSocket, (char*)&event, sizeof(event), 0);
        }

        // Перевірка лівої кнопки миші
        if (lButtonNow != lButtonDown) {
            event.type = 1;
            event.isDown = lButtonNow;
            send(clientSocket, (char*)&event, sizeof(event), 0);
            lButtonDown = lButtonNow;
        }

        // Перевірка правої кнопки миші
        if (rButtonNow != rButtonDown) {
            event.type = 2;
            event.isDown = rButtonNow;
            send(clientSocket, (char*)&event, sizeof(event), 0);
            rButtonDown = rButtonNow;
        }

        // Перевірка середньої кнопки миші
        if (mButtonNow != mButtonDown) {
            event.type = 3;
            event.isDown = mButtonNow;
            send(clientSocket, (char*)&event, sizeof(event), 0);
            mButtonDown = mButtonNow;
        }

        // Обробка колеса миші
        if (GetAsyncKeyState(VK_UP) & 0x8000) {
            event.type = 4;
            event.data = 120;  // WHEEL_DELTA
            send(clientSocket, (char*)&event, sizeof(event), 0);
        }
        else if (GetAsyncKeyState(VK_DOWN) & 0x8000) {
            event.type = 4;
            event.data = -120; // -WHEEL_DELTA
            send(clientSocket, (char*)&event, sizeof(event), 0);
        }

        lastPos = currentPos;

        // Обробка клавіатури
        for (int vk = 0; vk < 256; ++vk) {
            SHORT state = GetAsyncKeyState(vk);
            bool isPressed = (state & 0x8000) != 0;

            if (isPressed != keyStates[vk]) {
                keyStates[vk] = isPressed;
                static bool debounce = false;
                if ((GetAsyncKeyState(VK_CONTROL) & 0x8000) &&        // Ctrl утримується
                    (GetAsyncKeyState(VK_SHIFT) & 0x8000)) {       // Shift утримується
                    debounce = false;           // щоб не спрацьовувало безперервно

                    if (!debounce) {               // перший «кадр» натискання
                        blockHotkeys = !blockHotkeys;                // перемикаємо
                        debounce = true;                             // ставимо затримку
                    }
                }
                else {
                    debounce = false;               // обидві клавіші відпущено — можна знову спрацьовувати
                }

                // Пропуск спецклавіш у режимі блокування
                if (blockHotkeys &&
                    (vk == VK_LWIN || vk == VK_RWIN || vk == VK_CONTROL ||
                        vk == VK_MENU || vk == VK_SHIFT || vk == VK_ESCAPE)) {
                    continue;
                }

                event.type = 10 + vk; // Клавіші починаються з типу 10
                event.isDown = isPressed;
                send(clientSocket, (char*)&event, sizeof(event), 0);
            }
        }

        Sleep(10); // Затримка для зменшення навантаження
    }
    });
    std::thread imageThread([&]() {
        const std::string windowName = "Remote Screen";
        cv::namedWindow(windowName, cv::WINDOW_NORMAL);
        cv::setWindowProperty(windowName, cv::WND_PROP_FULLSCREEN, cv::WINDOW_FULLSCREEN);

        // Отримуємо розміри екрана
        int screenWidth = GetSystemMetrics(SM_CXSCREEN);
        int screenHeight = GetSystemMetrics(SM_CYSCREEN);

        while (isRunning) {
            int imgSize = 0;
            int received = recv(clientSocket, (char*)&imgSize, sizeof(imgSize), MSG_WAITALL);
            if (received <= 0 || imgSize <= 0) break;

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
            if (img.empty()) {
                std::cerr << "Помилка декодування зображення!" << std::endl;
                continue;
            }

            // Масштабування зі збереженням пропорцій
            float scale = std::min(
                (float)screenWidth / img.cols,
                (float)screenHeight / img.rows
            );
            cv::resize(img, img, cv::Size(), scale, scale);

            // Відображення у повноекранному режимі
            cv::imshow(windowName, img);

            if (cv::waitKey(1) == 27) { // Вихід по ESC
                isRunning = false;
                break;
            }
        }

        cv::destroyWindow(windowName);
        });

    // Очікуємо завершення
    imageThread.join();
    isRunning = false;
    inputThread.join();
    //keyboardThread.join();

    closesocket(clientSocket);
    WSACleanup();
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

