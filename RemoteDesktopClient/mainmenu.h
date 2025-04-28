#ifndef MAINMENU_H
#define MAINMENU_H

#include <windows.h>

// ������� ��� ��������� �������
void AddTabs(HWND hwnd);

// ����������� �������
void InitTabs(HWND hwnd);

// ������� ��� ����������� �������
void ShowTab(int index);

// �������� �������� ����
LRESULT CALLBACK MainWndProcMenu(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

void InitMainWindow(HINSTANCE hInstance);
#endif // MAINMENU_H