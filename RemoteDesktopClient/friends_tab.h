#ifndef FRIENDS_TAB_H
#define FRIENDS_TAB_H

#include <winsock2.h>
#include <ws2tcpip.h>
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
void InitLastConnectionsTab(HWND hwnd);
void LoadFriendsFromCSV(const std::wstring& filename, std::vector<FriendInfo>& friends);

#endif // FRIENDS_TAB_H
