#ifndef MAINMENU_H
#define MAINMENU_H

#include <windows.h>

// Функція для додавання вкладок
void AddTabs(HWND hwnd);

// Ініціалізація вкладок
void InitTabs(HWND hwnd);

// Функція для перемикання вкладок
void ShowTab(int index);

// Основний обробник вікна
LRESULT CALLBACK MainWndProcMenu(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

void InitMainWindow(HINSTANCE hInstance);
#endif // MAINMENU_H