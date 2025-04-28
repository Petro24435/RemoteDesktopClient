#include "server_tab.h"
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



int port;
extern UserInfo currentUser;
static HWND hKeyEdit;
static HWND hPortEdit;

// Функція для малювання елементів вкладки Server
void DrawServerTab(HWND hwnd) {
    std::wstring wip(currentUser.ip.begin(), currentUser.ip.end());

    // Моя IP адреса
    CreateWindowEx(0, L"STATIC", L"Моя IP адреса:", WS_CHILD | WS_VISIBLE,
        20, 20, 100, 20, hwnd, NULL, NULL, NULL);
    CreateWindowEx(0, L"EDIT", wip.c_str(), WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY,
        130, 20, 200, 25, hwnd, (HMENU)4001, NULL, NULL);
    // Запуск
    CreateWindowEx(0, L"BUTTON", L"Відкрити сервер", WS_CHILD | WS_VISIBLE | WS_BORDER,
        370, 20, 150, 25, hwnd, (HMENU)4007, NULL, NULL);
    // Закрити з'єднання
    CreateWindowEx(0, L"BUTTON", L"Закрити з'єднання", WS_CHILD | WS_VISIBLE | WS_BORDER,
        370, 60, 150, 25, hwnd, (HMENU)4008, NULL, NULL);

    // Закрити з'єднання
    CreateWindowEx(0, L"BUTTON", L"<-|->", WS_CHILD | WS_VISIBLE | WS_BORDER,
        370, 100, 100, 25, hwnd, (HMENU)4011, NULL, NULL);

    // Індикатор стану
    CreateWindowEx(0, L"STATIC", NULL, WS_CHILD | WS_VISIBLE | SS_OWNERDRAW,
        530, 23, 20, 20, hwnd, (HMENU)4010, NULL, NULL);  


    // Рамка
    CreateWindowEx(0, L"BUTTON", L"Доступ:", WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
        10, 40, 180, 80, hwnd, NULL, NULL, NULL);

    // Перша кнопка (початок групи)
    CreateWindowEx(0, L"BUTTON", L"Глядач", WS_CHILD | WS_VISIBLE | BS_RADIOBUTTON | WS_GROUP | WS_TABSTOP,
        20, 60, 150, 20, hwnd, (HMENU)4002, NULL, NULL);

    // Друга кнопка
    CreateWindowEx(0, L"BUTTON", L"Повний доступ", WS_CHILD | WS_VISIBLE | BS_RADIOBUTTON,
        20, 90, 150, 20, hwnd, (HMENU)4003, NULL, NULL);

    // Порт
    CreateWindowEx(0, L"STATIC", L"Порт:", WS_CHILD | WS_VISIBLE,
        20, 130, 100, 20, hwnd, NULL, NULL, NULL);
    hPortEdit = CreateWindowEx(0, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER,
        130, 130, 100, 25, hwnd, (HMENU)4004, NULL, NULL);

    // Ключ
    CreateWindowEx(0, L"STATIC", L"Ключ:", WS_VISIBLE | WS_CHILD,
        20, 170, 100, 20, hwnd, NULL, NULL, NULL);
    hKeyEdit = CreateWindowEx(0, L"EDIT", NULL, WS_VISIBLE | WS_CHILD | WS_BORDER ,
        130, 170, 300, 25, hwnd, (HMENU)4005, NULL, NULL);

    // Логін
    CreateWindowEx(0, L"STATIC", L"Логін:", WS_CHILD | WS_VISIBLE,
        20, 210, 100, 20, hwnd, NULL, NULL, NULL);
    CreateWindowEx(0, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY,
        130, 210, 200, 25, hwnd, (HMENU)4006, NULL, NULL);

    //// Стан з'єднання
    //CreateWindowEx(0, L"STATIC", L"Стан з'єднання:", WS_CHILD | WS_VISIBLE,
    //    20, 250, 100, 20, hwnd, NULL, NULL, NULL);
    CreateWindowEx(0, L"EDIT", L"Очікування...", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY,
        20, 250, 550, 50, hwnd, (HMENU)4009, NULL, NULL);

    FillPort(hwnd);
    FillKey(hwnd);
}

// Функція для обробки подій вкладки Server
LRESULT CALLBACK ServerTabWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {

    switch (msg) {
    case WM_COMMAND: {
        switch (LOWORD(wParam))
        {
        case 4002: // Глядач
            CheckRadioButton(hwnd, 4002, 4003, 4002);
            break;

        case 4003: // Повний доступ
            CheckRadioButton(hwnd, 4002, 4003, 4003);
            break;
        
        case 4004:
            if (HIWORD(wParam) == EN_CHANGE) {
                wchar_t buffer[16]; // до 5 цифр + запас
                GetWindowText(GetDlgItem(hwnd, 4004), buffer, 16);
                port = wcstol(buffer, NULL, 10); // або wcstol(buffer, NULL, 10)
                FillKey(hwnd);
            }
            break;
        case 4007: {
            wchar_t portBuffer[256];
            GetWindowText(hPortEdit, portBuffer, 256);
            std::wstring wport(portBuffer);
            std::string newPort(wport.begin(), wport.end());

            wchar_t keyBuffer[256];
            GetWindowText(hKeyEdit, keyBuffer, 256);
            std::wstring wkey(keyBuffer);
            std::string newKey(wkey.begin(), wkey.end());

            updatePortsTable(newPort);
            updateKeysTable(newKey);
            addConnection(hwnd, currentUser.login, port, newKey, currentUser.ip);
            //SetWindowText(GetDlgItem(hwnd, 4009), L"Помилка при підключенні!");4006
            setStatusColor(hwnd, RGB(255, 255, 0)); // Жовтий
            MessageBox(hwnd, L"Сервер відкрито", L"Успіх", MB_OK);
            break;
        }
        case 4008:
            cleanUnusedPortsAndKeys();
            removeConnection(hwnd, currentUser.login, port);
            MessageBox(hwnd, L"Закрито з'єднання і очищено таблиці", L"Успіх", MB_OK);
            break;

        case 4011:
            // Отримуємо текст з 4004 і передаємо в 3003

            GetDlgItemText(hwnd, 4004, bufferPort, sizeof(bufferPort) / sizeof(WCHAR));
            // Отримуємо текст з 4005 і передаємо в 3001
            GetDlgItemText(hwnd, 4005, bufferKey, sizeof(bufferKey) / sizeof(WCHAR));
            break;



        }

        break;
    }
    case WM_CREATE: {
        // Пошук дескриптора поля для ключа
        //hKeyEdit = GetDlgItem(hwnd, 4005);
        break;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    case WM_DRAWITEM:
    {
        LPDRAWITEMSTRUCT lpDrawItem = (LPDRAWITEMSTRUCT)lParam;

        if (lpDrawItem->CtlID == 4010) // Якщо це наш кружечок
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
    return 0;
}

// Ініціалізація вкладки Server
void InitServerTab(HWND hwnd) {
    DrawServerTab(hwnd);
        currentStatusColor = RGB(192, 192, 192);
    // Встановлюємо процедуру обробки подій для вкладки
    SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)ServerTabWndProc);
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
    savePortsToCSV(usedPorts, "used_ports.csv");

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
    SetWindowText(GetDlgItem(hwnd, 4005), wkey.c_str());
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

// Функція для завантаження CSV файлу з Dropbox
/*void downloadCSVFromDropbox(const std::string& dropboxFilePath) {
    // Тут буде код для доступу до Dropbox API та завантаження CSV
    // Це можна зробити через HTTP запити або з використанням Dropbox SDK

    // Для початку, завантажуємо файл через API (відповідно до Dropbox API)
    std::string downloadURL = "https://www.dropbox.com/scl/fi/zead1zv18sj13uurkxdmq/used_ports.csv?rlkey=jffef5xy61c4v6ozhblcapd9d&st=fnql71bz&dl=0";
    std::string token = "vcfyyrxwlspqebt"; // Токен доступу, який ти отримаєш через Dropbox API

    // Виконання HTTP запиту для завантаження файлу
    // Тут треба використовувати відповідні бібліотеки або інструменти для HTTP запитів, наприклад, cURL або WinINet
    // Можна зробити це через POST-запит для отримання файлу на основі Dropbox API
    //std::cout << "Downloading CSV from Dropbox: " << dropboxFilePath << std::endl;
}*/

void loadCSVFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        MessageBox(NULL, L"Не вдалося відкрити файл", L"Помилка", MB_ICONERROR);
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        // Тут можна парсити CSV рядки
        // Наприклад вивести:
        // std::cout << "Рядок: " << line << std::endl;
    }

    file.close();
}
// Функція для оновлення таблиці портів
void updatePortsTable(const std::string& newPort) {
    std::ofstream file("used_ports.csv", std::ios::app); // Відкрити у режимі дозапису
    if (!file.is_open()) {
        MessageBox(NULL, L"Не вдалося відкрити файл для портів", L"Помилка", MB_ICONERROR);
        return;
    }
    file << newPort << "\n";
    file.close();
}


void updateKeysTable(const std::string& newKey) {
    std::ofstream file("keys.csv", std::ios::app); // Відкрити у режимі дозапису
    if (!file.is_open()) {
        MessageBox(NULL, L"Не вдалося відкрити файл для ключів", L"Помилка", MB_ICONERROR);
        return;
    }
    file << newKey << "\n";
    file.close();
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
    default:
    currentStatusColor = RGB(192, 192, 192);
    break;
    }

    // Перемалюємо контрол з ID 4010
    InvalidateRect(GetDlgItem(hwnd, 4010), NULL, TRUE); // Перемалювання для оновлення
}

