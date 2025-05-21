#include "friends_tab.h"
#include <windows.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <fstream>
#include <string>
#include <vector>
#include <ctime>
#include <iomanip>
#include <sstream>

std::vector<FriendInfo> friendsList;

// Функція для малювання елементів вкладки Friends
void DrawFriendsTab(HWND hwnd) {
    HWND hwndList = CreateWindowEx(0, L"LISTBOX", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | LBS_NOTIFY,
        20, 20, 300, 200, hwnd, (HMENU)2001, NULL, NULL);

    // Завантажуємо список друзів з CSV файлу
    LoadFriendsFromCSV(L"friends.csv", friendsList);

    // Додаємо дані в список
    for (const auto& friendInfo : friendsList) {
        std::wstring listItem = friendInfo.date + L" " + friendInfo.time + L" | " + friendInfo.serverLogin + L" | " + friendInfo.clientLogin;
        SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)listItem.c_str());
    }
}

// Функція для завантаження даних з CSV файлу
void LoadFriendsFromCSV(const std::wstring& filename, std::vector<FriendInfo>& friends) {
    std::wifstream file(filename);
    std::wstring line;
    while (std::getline(file, line)) {
        std::wistringstream ss(line);
        std::wstring date, time, serverLogin, clientLogin;

        // Зчитуємо кожне поле CSV
        std::getline(ss, date, L',');
        std::getline(ss, time, L',');
        std::getline(ss, serverLogin, L',');
        std::getline(ss, clientLogin, L',');

        // Додаємо до списку
        friends.push_back({ date, time, serverLogin, clientLogin });
    }
}

void SaveFriendsToCSV(const std::wstring& filename, const std::vector<FriendInfo>& friends) {
    std::wofstream file(filename, std::ios::trunc); // перезапис файлу
    if (!file.is_open()) {
        //std::cerr << L"Не вдалося відкрити файл для запису: " << filename << std::endl;
        return;
    }

    for (const auto& f : friends) {
        file << f.date << L","
            << f.time << L","
            << f.serverLogin << L","
            << f.clientLogin << L"\n";
    }
}

std::wstring GetCurrentDate() {
    std::wstringstream wss;
    std::time_t now = std::time(nullptr);
    std::tm local{};
    localtime_s(&local, &now);
    wss << std::put_time(&local, L"%Y-%m-%d");
    return wss.str();
}

std::wstring GetCurrentTimes() {
    std::wstringstream wss;
    std::time_t now = std::time(nullptr);
    std::tm local{};
    localtime_s(&local, &now);
    wss << std::put_time(&local, L"%H:%M:%S");
    return wss.str();
}

void AddFriendToCSV(const std::wstring& serverLogin, const std::wstring& clientLogin) {
    std::wofstream file(L"friends.csv", std::ios::app); // відкриваємо для допису
    if (!file.is_open()) {
        //std::wcerr << L"Не вдалося відкрити файл для запису: " << filename << std::endl;
        return;
    }

    std::wstring date = GetCurrentDate();
    std::wstring time = GetCurrentTimes();

    file << date << L"," << time << L"," << serverLogin << L"," << clientLogin << L"\n";
}

// Ініціалізація вкладки Friends
void InitLastConnectionsTab(HWND hwnd) {
    DrawFriendsTab(hwnd);
}
