#ifndef CLIENT_TAB_H
#define CLIENT_TAB_H
#include <winsock2.h>
#include <ws2tcpip.h>  // Add this line
#include <iostream> // <-- ����� ��
#include <windows.h>

// ��������� ��� ����� ������� �볺���
struct ClientTabData {
    HWND hwndKey;
    HWND hwndLogin;
    HWND hwndPort;
};

// ���������� �������
void DrawClientTab(HWND hwnd);
void InitClientTab(HWND hwnd);
LRESULT CALLBACK ClientTabWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);


#endif // CLIENT_TAB_H
