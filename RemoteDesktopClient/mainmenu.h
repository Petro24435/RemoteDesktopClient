#ifndef MAINMENU_H
#define MAINMENU_H

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

// ������� ��� ��������� �������
void AddTabsClient(HWND hwnd);

// ����������� �������
void InitTabsClient(HWND hwnd);

// ������� ��� ��������� �������
void AddTabsServer(HWND hwnd);

// ����������� �������
void InitTabsServer(HWND hwnd);

// ������� ��� ����������� �������
void ShowTab(int index);

// �������� �������� ����
LRESULT CALLBACK ClientWndProcMenu(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
LRESULT CALLBACK ServerWndProcMenu(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

void InitMainWindow(HINSTANCE hInstance);
#endif // MAINMENU_H