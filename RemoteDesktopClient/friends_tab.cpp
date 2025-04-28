#include "friends_tab.h"
#include <windows.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

std::vector<FriendInfo> friendsList;

// ������� ��� ��������� �������� ������� Friends
void DrawFriendsTab(HWND hwnd) {
    HWND hwndList = CreateWindowEx(0, L"LISTBOX", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | LBS_NOTIFY,
        20, 20, 300, 200, hwnd, (HMENU)2001, NULL, NULL);

    // ����������� ������ ����� � CSV �����
    LoadFriendsFromCSV(L"friends.csv", friendsList);

    // ������ ��� � ������
    for (const auto& friendInfo : friendsList) {
        std::wstring listItem = friendInfo.date + L" " + friendInfo.time + L" | " + friendInfo.serverLogin + L" | " + friendInfo.clientLogin;
        SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM)listItem.c_str());
    }
}

// ������� ��� ������������ ����� � CSV �����
void LoadFriendsFromCSV(const std::wstring& filename, std::vector<FriendInfo>& friends) {
    std::wifstream file(filename);
    std::wstring line;
    while (std::getline(file, line)) {
        std::wistringstream ss(line);
        std::wstring date, time, serverLogin, clientLogin;

        // ������� ����� ���� CSV
        std::getline(ss, date, L',');
        std::getline(ss, time, L',');
        std::getline(ss, serverLogin, L',');
        std::getline(ss, clientLogin, L',');

        // ������ �� ������
        friends.push_back({ date, time, serverLogin, clientLogin });
    }
}

// ����������� ������� Friends
void InitFriendsTab(HWND hwnd) {
    DrawFriendsTab(hwnd);
}
