#ifndef FRIENDS_TAB_H
#define FRIENDS_TAB_H

#include <windows.h>
#include <string>      // Для std::wstring
#include <vector>      // Для std::vector
#include <fstream>     // Для роботи з файлами

// Структура для збереження інформації про друга
struct FriendInfo {
    std::wstring date;
    std::wstring time;
    std::wstring serverLogin;
    std::wstring clientLogin;
};

// Оголошення функцій
void DrawFriendsTab(HWND hwnd);
void InitFriendsTab(HWND hwnd);
void LoadFriendsFromCSV(const std::wstring& filename, std::vector<FriendInfo>& friends);

#endif // FRIENDS_TAB_H
