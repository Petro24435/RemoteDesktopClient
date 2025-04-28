#include "friends_tab.h"
#include <windows.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

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

// Ініціалізація вкладки Friends
void InitFriendsTab(HWND hwnd) {
    DrawFriendsTab(hwnd);
}
