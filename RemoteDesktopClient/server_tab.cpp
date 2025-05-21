#include <string>
#include <set>
#include <vector>
#include <fstream>
#include <sstream>
#include <ctime>
#include <cstdlib>
#include <string>
#include <sstream>
#include <iomanip>
#include <cctype>
#include <cstdio>
#include "server_tab.h"
#include "serverUserRegistration.h"
#include "resource.h"

int port;
extern UserInfo currentUser;
void savePortsToCSV(const std::set<int>& ports, const std::string& filename);
extern HWND hLabelIp = NULL; 
extern HWND hIpEdit = NULL;
extern HWND hStartBtn = NULL;
extern HWND hCloseBtn = NULL;
extern HWND hDisconnectBtn = NULL;
extern HWND hStatusIcon = NULL;
extern HWND hGroupBoxAccess = NULL;
extern HWND hMouseAccess = NULL;
extern HWND hKeyboardAccess = NULL;
extern HWND hLabelPort = NULL;
extern HWND hPortEdit = NULL;
extern HWND hLabelKey = NULL;
extern HWND hKeyEdit = NULL;
extern HWND hLabelClient = NULL;
extern HWND hClientEdit = NULL;
extern HWND hLogEdit = NULL;
static HBRUSH hWhiteBrush = CreateSolidBrush(RGB(255, 255, 255)); // білий фон
HFONT hServerFont = CreateFont(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
    DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
    DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
HFONT hServerFont2 = CreateFont(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
    DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
    DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
extern std::atomic<bool> mouseAccess = false, keyboardAccess = false;  // Перемикачі для флажків
void DrawOpenConnectTab(HWND hwnd) 
{
    std::wstring wip(currentUser.ip.begin(), currentUser.ip.end());

    hLabelIp = CreateWindowEx(0, L"STATIC", L"Моя IP адреса:", WS_CHILD | WS_VISIBLE,
        20, 20, 100, 20, hwnd, NULL, NULL, NULL);

    hIpEdit = CreateWindowEx(0, L"EDIT", wip.c_str(), WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY,
        130, 20, 200, 25, hwnd, (HMENU)4001, NULL, NULL);

    hStartBtn = CreateWindowEx(0, L"BUTTON", L"Відкрити сервер", WS_CHILD | WS_VISIBLE | WS_BORDER,\
        370, 20, 150, 25, hwnd, (HMENU)4002, NULL, NULL);

    //hCloseBtn = CreateWindowEx(0, L"BUTTON", L"Закрити з'єднання", WS_CHILD | WS_VISIBLE | WS_BORDER,\
        370, 60, 150, 25, hwnd, (HMENU)4008, NULL, NULL);

    //hDisconnectBtn = CreateWindowEx(0, L"BUTTON", L"Від'єднати Клієнта", WS_CHILD | WS_VISIBLE | WS_BORDER,\
        370, 100, 150, 25, hwnd, (HMENU)4011, NULL, NULL);

    hStatusIcon = CreateWindowEx(0, L"STATIC", NULL, WS_CHILD | WS_VISIBLE | SS_OWNERDRAW,
        530, 23, 20, 20, hwnd, (HMENU)4003, NULL, NULL);

    //hGroupBoxAccess = CreateWindowEx(0, L"STATIC", L"Доступ:", WS_CHILD | WS_VISIBLE ,\
        10, 40, 180, 80, hwnd, NULL, NULL, NULL);

    //hMouseAccess = CreateWindowEx(0, L"BUTTON", L"Миша", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,\
        20, 60, 150, 20, hwnd, (HMENU)4002, NULL, NULL);

    //hKeyboardAccess = CreateWindowEx(0, L"BUTTON", L"Клавіатура", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,\
        20, 90, 150, 20, hwnd, (HMENU)4003, NULL, NULL);

    hLabelPort = CreateWindowEx(0, L"STATIC", L"Порт:", WS_CHILD | WS_VISIBLE,
        20, 130, 100, 20, hwnd, NULL, NULL, NULL);

    hPortEdit = CreateWindowEx(0, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER,
        130, 130, 100, 25, hwnd, (HMENU)4004, NULL, NULL);

    hLabelKey = CreateWindowEx(0, L"STATIC", L"Ключ:", WS_VISIBLE | WS_CHILD,
        20, 170, 100, 20, hwnd, NULL, NULL, NULL);

    hKeyEdit = CreateWindowEx(0, L"EDIT", NULL, WS_VISIBLE | WS_CHILD | WS_BORDER,
        130, 170, 300, 25, hwnd, (HMENU)4005, NULL, NULL);

    //hLabelClient = CreateWindowEx(0, L"STATIC", L"Клієнт:", WS_CHILD | WS_VISIBLE,\
        20, 210, 100, 20, hwnd, NULL, NULL, NULL);

    //hClientEdit = CreateWindowEx(0, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY,\
        130, 210, 200, 25, hwnd, (HMENU)4006, NULL, NULL);

    //hLogEdit = CreateWindowEx(0, L"EDIT", L"Очікування...", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY | ES_MULTILINE,\
        20, 220, 550, 50, hwnd, (HMENU)4009, NULL, NULL);

    FillPort(hwnd);
    FillKey(hwnd);
}

void DrawConnectManagingTab(HWND hwnd)
{

    // Створення кнопок
    hCloseBtn = CreateWindowEx(0, L"BUTTON", L"Закрити З'єданння", WS_CHILD | WS_VISIBLE | WS_BORDER ,
        370, 60, 150, 25, hwnd, (HMENU)4006, NULL, NULL);
    hDisconnectBtn = CreateWindowEx(0, L"BUTTON", L"Відключити Клієнта", WS_CHILD | WS_VISIBLE | WS_BORDER ,
        370, 100, 150, 25, hwnd, (HMENU)4007, NULL, NULL);

    // Кнопки з флажками
    hMouseAccess = CreateWindowEx(0, L"BUTTON", L"Миша", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, //| BS_BITMAP,
        20, 60, 150, 20, hwnd, (HMENU)4008, NULL, NULL);
    hKeyboardAccess = CreateWindowEx(0, L"BUTTON", L"Клавіатура", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
        20, 90, 150, 20, hwnd, (HMENU)4009, NULL, NULL);

    hLabelClient = CreateWindowEx(0, L"STATIC", L"Клієнт:", WS_CHILD | WS_VISIBLE,\
        20, 210, 100, 20, hwnd, NULL, NULL, NULL);
    // Створення інших елементів інтерфейсу
    hClientEdit = CreateWindowEx(0, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY,
        130, 210, 200, 25, hwnd, (HMENU)4010, NULL, NULL);
    hLogEdit = CreateWindowEx(0, L"EDIT", L"Очікування...", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY | ES_MULTILINE,
        20, 220, 550, 50, hwnd, (HMENU)4011, NULL, NULL);

    //// Встановлення зображень для кнопок
    //SendMessage(hCloseBtn, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP_CLOSE)));
    //SendMessage(hDisconnectBtn, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP_DISCONNECT)));

    //// Для флажків змінюємо зображення залежно від стану
    //if (mouseAccess) {
    //    SendMessage(hMouseAccess, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP_MOUSE_ON)));
    //}
    //else {
    //    SendMessage(hMouseAccess, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP_MOUSE_OFF)));
    //}

    //if (keyboardAccess) {
    //    SendMessage(hKeyboardAccess, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP_KEYBOARD_ON)));
    //}
    //else {
    //    SendMessage(hKeyboardAccess, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BITMAP_KEYBOARD_OFF)));
    //}
}

// Функція для обробки подій вкладки Server
LRESULT CALLBACK ConnectManagingTabWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {

    switch (msg) {
    case WM_COMMAND: {
        switch (LOWORD(wParam))
        {

        case 4008: {  // Флажок миші
                mouseAccess = !mouseAccess;
            }
        case 4009: {  // Флажок клавіатури
                keyboardAccess = !keyboardAccess;
            }
            break;
        case 4006:
            cleanUnusedPortsAndKeys();

            removeConnection(hwnd, currentUser.login, port);
            MessageBox(hwnd, L"Закрито з'єднання і очищено таблиці", L"Успіх", MB_OK);
            break;
            case 4007:
                disconnectClient(hwnd, port);
                MessageBox(hwnd, L"Клієнта від'єднано", L"Успіх", MB_OK);
                break;
        }
        break;
    }
    case WM_SIZE: {
        // Отримання нових розмірів вікна
        int width = LOWORD(lParam);
        int height = HIWORD(lParam);

        // Обчислення масштабу
        float scaleX = (float)width / UI_BASE_WIDTH;
        float scaleY = (float)height / UI_BASE_HEIGHT;

        auto sx = [&](int val) { return (int)(val * scaleX); };
        auto sy = [&](int val) { return (int)(val * scaleY); };


        // Перша група — дві кнопки одна на одній (лівий верхній кут)
        MoveWindow(hCloseBtn, sx(20), sy(20), sx(120), sy(30), TRUE);
        MoveWindow(hDisconnectBtn, sx(20), sy(60), sx(120), sy(30), TRUE);

        // Друга група — теж одна на одній, поруч
        MoveWindow(hMouseAccess, sx(150), sy(20), sx(80), sy(30), TRUE);
        MoveWindow(hKeyboardAccess, sx(150), sy(60), sx(80), sy(30), TRUE);

        // Поле для введення клієнта — без змін
        MoveWindow(hLabelClient, sx(240), sy(20), sx(160), sy(30), TRUE);
        MoveWindow(hClientEdit, sx(240), sy(40), sx(160), sy(30), TRUE);

        // Лог — без змін
        MoveWindow(hLogEdit, sx(20), sy(100), sx(550), sy(250), TRUE);

        if (hServerFont) DeleteObject(hServerFont);
        int fontSize = min(sx(16), sy(16));
        hServerFont = CreateFont(fontSize, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
            DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");

        HWND controls[] = {
            hCloseBtn, hDisconnectBtn,
            hGroupBoxAccess, hMouseAccess, hKeyboardAccess,
            hLabelClient, hClientEdit, hLogEdit,
        };

        for (HWND ctrl : controls) {
            SendMessage(ctrl, WM_SETFONT, (WPARAM)hServerFont, TRUE);
        }
        break;
    }
    case WM_CTLCOLORSTATIC:
    case WM_CTLCOLORBTN:
    {
        HDC hdcStatic = (HDC)wParam;
        SetBkMode(hdcStatic, TRANSPARENT); // фон прозорий
        SetBkColor(hdcStatic, RGB(255, 255, 255)); // білий фон
        return (INT_PTR)hWhiteBrush;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    case WM_DRAWITEM:
    {
        LPDRAWITEMSTRUCT lpDrawItem = (LPDRAWITEMSTRUCT)lParam;

        if (hStatusIcon) // Якщо це наш кружечок
        {
            HBRUSH hBrush = CreateSolidBrush(currentStatusColor); // Колір беремо з змінної
            FillRect(lpDrawItem->hDC, &lpDrawItem->rcItem, hBrush);
            DeleteObject(hBrush);
            return TRUE;
        }
    }

    break;

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
}

// Функція для обробки подій вкладки Server
LRESULT CALLBACK OpenConnectTabWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {

    switch (msg) {
    case WM_COMMAND: {
        switch (LOWORD(wParam))
        {
        case 4004:
            if (HIWORD(wParam) == EN_CHANGE) {
                wchar_t buffer[16]; // до 5 цифр + запас
                GetWindowText(GetDlgItem(hwnd, 4004), buffer, 16);
                port = wcstol(buffer, NULL, 10); // або wcstol(buffer, NULL, 10)
                FillKey(hwnd);
            }
            break;
        case 4002: {
            wchar_t portBuffer[256];
            GetWindowText(hPortEdit, portBuffer, 256);
            std::wstring wport(portBuffer);
            std::string newPort(wport.begin(), wport.end());

            wchar_t keyBuffer[256];
            GetWindowText(hKeyEdit, keyBuffer, 256);
            std::wstring wkey(keyBuffer);
            std::string newKey(wkey.begin(), wkey.end());

            savePortsToCSV(getUsedPorts(), "C:/opencv/used_ports.csv");
            addConnection(hwnd, currentUser.login, port, newKey, currentUser.ip);
            //SetWindowText(GetDlgItem(hwnd, 4009), L"Помилка при підключенні!");4006
            MessageBox(hwnd, L"Сервер відкрито", L"Успіх", MB_OK);
            break;
        }
        }
        break;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    case WM_DRAWITEM:
    {
        LPDRAWITEMSTRUCT lpDrawItem = (LPDRAWITEMSTRUCT)lParam;

        if (hStatusIcon) // Якщо це наш кружечок
        {
            HBRUSH hBrush = CreateSolidBrush(currentStatusColor); // Колір беремо з змінної
            FillRect(lpDrawItem->hDC, &lpDrawItem->rcItem, hBrush);
            DeleteObject(hBrush);
            return TRUE;
        }
    }
    case WM_SIZE: {
        // Отримання нових розмірів вікна
        int width = LOWORD(lParam);
        int height = HIWORD(lParam);

        // Обчислення масштабу
        float scaleX = (float)width / UI_BASE_WIDTH;
        float scaleY = (float)height / UI_BASE_HEIGHT;

        auto sx = [&](int val) { return (int)(val * scaleX); };
        auto sy = [&](int val) { return (int)(val * scaleY); };

        // Масштабування елементів на вкладках
        MoveWindow(hLabelIp, sx(20), sy(0), sx(100), sy(20), TRUE);
        MoveWindow(hIpEdit, sx(130), sy(0), sx(200), sy(25), TRUE);


        MoveWindow(hLabelPort, sx(20), sy(30), sx(100), sy(20), TRUE);
        MoveWindow(hPortEdit, sx(130), sy(30), sx(100), sy(25), TRUE);
        MoveWindow(hLabelKey, sx(20), sy(65), sx(100), sy(20), TRUE);
        MoveWindow(hKeyEdit, sx(130), sy(65), sx(300), sy(25), TRUE);

        MoveWindow(hStartBtn, sx(20), sy(100), sx(250), sy(25), TRUE);
        MoveWindow(hStatusIcon, sx(527), sy(3), 30, 30, TRUE);

        if (hServerFont2) DeleteObject(hServerFont2);
        int fontSize = min(sx(16), sy(16));
        hServerFont2 = CreateFont(fontSize, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
            DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");

        HWND controls[] = {
            hLabelIp, hIpEdit, hStartBtn, hStatusIcon,
            hLabelPort, hPortEdit, hLabelKey, hKeyEdit,
        };

        for (HWND ctrl : controls) {
            SendMessage(ctrl, WM_SETFONT, (WPARAM)hServerFont2, TRUE);
        }
        break;
    }
    case WM_CTLCOLORSTATIC:
    case WM_CTLCOLORBTN:
    {
        HDC hdcStatic = (HDC)wParam;
        SetBkMode(hdcStatic, TRANSPARENT); // фон прозорий
        SetBkColor(hdcStatic, RGB(255, 255, 255)); // білий фон
        return (INT_PTR)hWhiteBrush;
    }
    break;

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

// Ініціалізація вкладки Server
void InitOpenConnectTab(HWND hwnd) {
    DrawOpenConnectTab(hwnd);
        currentStatusColor = RGB(192, 192, 192);
    // Встановлюємо процедуру обробки подій для вкладки
    SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)OpenConnectTabWndProc);
}

// Ініціалізація вкладки Server
void InitConnectManagingTab(HWND hwnd) {
    DrawConnectManagingTab(hwnd);
        currentStatusColor = RGB(192, 192, 192);
    // Встановлюємо процедуру обробки подій для вкладки
    SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)ConnectManagingTabWndProc);
}


// Отримати зайняті порти
std::set<int> getUsedPorts() {
    std::set<int> usedPorts;
    FILE* pipe = _popen("netstat -an", "r");
    if (!pipe) return usedPorts;

    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), pipe)) {
        std::string line(buffer);
        std::istringstream iss(line);
        std::string proto, localAddr, foreignAddr, state;
        iss >> proto >> localAddr >> foreignAddr >> state;

        size_t colonPos = localAddr.rfind(':');
        if (colonPos != std::string::npos) {
            std::string portStr = localAddr.substr(colonPos + 1);
            try {
                int port = std::stoi(portStr);
                usedPorts.insert(port);
            }
            catch (...) {}
        }
    }
    _pclose(pipe);
    return usedPorts;
}

// Зберегти у CSV
void savePortsToCSV(const std::set<int>& ports, const std::string& filename) {
    std::ofstream file(filename);
    file << "Used Ports\n";
    for (int port : ports) {
        file << port << "\n";
    }
    file.close();
}

// Отримати випадковий вільний порт
int getRandomFreePort() {
    std::set<int> usedPorts = getUsedPorts();
    savePortsToCSV(usedPorts, "C:/opencv/used_ports.csv");

    std::vector<int> freePorts;
    for (int port = 1000; port <= 9999; ++port) {
        if (usedPorts.find(port) == usedPorts.end()) {
            freePorts.push_back(port);
        }
    }

    if (freePorts.empty()) return -1;

    std::srand((unsigned int)std::time(nullptr));
    int index = std::rand() % freePorts.size();
    return freePorts[index];
}

void FillPort(HWND hwnd)
{
    port = getRandomFreePort();
    if (port != -1) {
        std::wstring wport = std::to_wstring(port);
        SetWindowText(GetDlgItem(hwnd, 4004), wport.c_str());
    }
    else {
        MessageBox(hwnd, L"Не знайдено вільного порту!", L"Помилка", MB_ICONERROR);
    }
}

void FillKey(HWND hwnd)
{
    std::string key = generateKey(currentUser.ip, currentUser.login, port);
    std::wstring wkey(key.begin(), key.end());
    //std::wstring wport = std::to_wstring(port);
    //MessageBox(hwnd, wkey.c_str(), L"Згенерований ключ", MB_OK | MB_ICONINFORMATION);
    //MessageBox(hwnd, wport.c_str(), L"Port", MB_OK | MB_ICONINFORMATION);
    SetWindowText(hKeyEdit, wkey.c_str());
}

std::string generateKey(const std::string& ip, const std::string& login, int port) {
    std::ostringstream keyStream;

    // Перетворення IP в байти
    std::vector<int> ipBytes;
    std::stringstream ss(ip);
    std::string segment;
    while (std::getline(ss, segment, '.')) {
        ipBytes.push_back(std::stoi(segment));
    }

    // Порт у цифри
    std::string portStr = std::to_string(port);
    std::vector<int> portDigits;
    for (char c : portStr) {
        if (std::isdigit(c)) {
            portDigits.push_back(c - '0');
        }
    }

    // IP ^ PortDigits
    for (size_t i = 0; i < ipBytes.size(); ++i) {
        int portDigit = portDigits[i % portDigits.size()];
        int result = ipBytes[i] ^ portDigit;
        keyStream << std::setw(2) << std::setfill('0') << std::hex << std::uppercase << result;
    }

    // Логін ASCII ^ PortDigits
    for (size_t i = 0; i < login.size(); ++i) {
        int portDigit = portDigits[i % portDigits.size()];
        int result = static_cast<unsigned char>(login[i]) ^ portDigit;
        keyStream << std::setw(2) << std::setfill('0') << std::hex << std::uppercase << result;
    }

    return keyStream.str();
}


// Оновлюємо колір
void setStatusColor(HWND hwnd, char c) {
    switch (c)
    {
    case'r': currentStatusColor = RGB(255, 0, 0); break;
    case'g': currentStatusColor = RGB(0, 255, 0); break;
    case'b': currentStatusColor = RGB(0, 0, 255); break;
    case'y': currentStatusColor = RGB(255, 255, 0); break;
    case'w': currentStatusColor = RGB(255, 255, 255); break;
    case'0': currentStatusColor = RGB(192, 192, 192); break;
    default:
    break;
    }

    // Перемалюємо контрол з ID 4010
    InvalidateRect(hStatusIcon, NULL, TRUE); // Перемалювання для оновлення
}

