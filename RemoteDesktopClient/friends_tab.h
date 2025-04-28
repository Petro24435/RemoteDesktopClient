#ifndef FRIENDS_TAB_H
#define FRIENDS_TAB_H

#include <windows.h>
#include <string>      // ��� std::wstring
#include <vector>      // ��� std::vector
#include <fstream>     // ��� ������ � �������

// ��������� ��� ���������� ���������� ��� �����
struct FriendInfo {
    std::wstring date;
    std::wstring time;
    std::wstring serverLogin;
    std::wstring clientLogin;
};

// ���������� �������
void DrawFriendsTab(HWND hwnd);
void InitFriendsTab(HWND hwnd);
void LoadFriendsFromCSV(const std::wstring& filename, std::vector<FriendInfo>& friends);

#endif // FRIENDS_TAB_H
