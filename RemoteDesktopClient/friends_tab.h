#ifndef FRIENDS_TAB_H
#define FRIENDS_TAB_H

#include <winsock2.h>
#include <ws2tcpip.h>
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
void InitLastConnectionsTab(HWND hwnd);
void LoadFriendsFromCSV(const std::wstring& filename, std::vector<FriendInfo>& friends);
void SaveFriendsToCSV(const std::wstring& filename, std::vector<FriendInfo>& friends);
void AddFriendToCSV(const std::wstring& serverLogin, const std::wstring& clientLogin);

#endif // FRIENDS_TAB_H
