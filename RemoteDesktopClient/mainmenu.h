#ifndef MAINMENU_H
#define MAINMENU_H

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

// Функція для додавання вкладок
void AddTabsClient(HWND hwnd);

// Ініціалізація вкладок
void InitTabsClient(HWND hwnd);

// Функція для додавання вкладок
void AddTabsServer(HWND hwnd);

// Ініціалізація вкладок
void InitTabsServer(HWND hwnd);

// Функція для перемикання вкладок
void ShowTab(int index);

// Основний обробник вікна
LRESULT CALLBACK ClientWndProcMenu(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
LRESULT CALLBACK ServerWndProcMenu(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

void InitMainWindow(HINSTANCE hInstance);
#endif // MAINMENU_H