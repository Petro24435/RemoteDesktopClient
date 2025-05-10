#include "mainmenu.h"
#include "user.h"
#include <commctrl.h>
#include "profile_tab.h"
#include "friends_tab.h"
#include "client_tab.h"
#include "server_tab.h"

#pragma comment(lib, "comctl32.lib")

#include <windows.h>
HWND hTab;
HWND hTabPages[4]; // 4 вкладки

// Додавання вкладок до TabControl
void AddTabsClient(HWND hwnd) {
    hTab = CreateWindowEx(0, WC_TABCONTROL, L"",
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
        0, 0, 600, 40,
        hwnd, NULL, NULL, NULL);

    TCITEM tie = { 0 };
    tie.mask = TCIF_TEXT;

    wchar_t* tabNames[4] = { (LPWSTR)L"Мої Дані", (LPWSTR)L"Останні підключення", (LPWSTR)L"Підключення", (LPWSTR)L"Активне з'єднання" };

    // Додавання назв вкладок
    for (int i = 0; i < 4; i++) {
        tie.pszText = tabNames[i];
        TabCtrl_InsertItem(hTab, i, &tie);
    }
}

// Ініціалізація вкладок
void InitTabsClient(HWND hwnd) {
    // Створення окремих вкладок
    for (int i = 0; i < 4; ++i) {
        hTabPages[i] = CreateWindowEx(0, L"STATIC", L"",
            WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
            0, 60, 600, 400, hwnd, NULL, NULL, NULL);

        // Сховати всі вкладки на початку
        ShowWindow(hTabPages[i], (i == 0) ? SW_SHOW : SW_HIDE);
    }

    // Ініціалізація вкладок
    InitProfileTab(hTabPages[0]);
    InitLastConnectionsTab(hTabPages[1]);
    //InitConnectToTab(hTabPages[2]);
    //InitActiveConnetionTab(hTabPages[3]);
}

// Додавання вкладок до TabControl
void AddTabsServer(HWND hwnd) {
    hTab = CreateWindowEx(0, WC_TABCONTROL, L"",
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
        0, 0, 600, 40,
        hwnd, NULL, NULL, NULL);

    TCITEM tie = { 0 };
    tie.mask = TCIF_TEXT;

    wchar_t* tabNames[4] = { (LPWSTR)L"Мій профіль", (LPWSTR)L"Останні підключення", (LPWSTR)L"Відкриття з'єднання ", (LPWSTR)L"Управління з'єдананням" };

    // Додавання назв вкладок
    for (int i = 0; i < 4; i++) {
        tie.pszText = tabNames[i];
        TabCtrl_InsertItem(hTab, i, &tie);
    }
}

// Ініціалізація вкладок
void InitTabsServer(HWND hwnd) {
    // Створення окремих вкладок
    for (int i = 0; i < 4; ++i) {
        hTabPages[i] = CreateWindowEx(0, L"STATIC", L"",
            WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
            0, 60, 600, 400, hwnd, NULL, NULL, NULL);

        // Сховати всі вкладки на початку
        ShowWindow(hTabPages[i], (i == 0) ? SW_SHOW : SW_HIDE);
    }

    // Ініціалізація вкладок
    InitProfileTab(hTabPages[0]);
    InitLastConnectionsTab(hTabPages[1]);
    InitOpenConnectTab(hTabPages[2]);
    //InitServerTab(hTabPages[3]);
    InitConnectManagingTab(hTabPages[3]);
}

// Перемикання вкладок
void ShowTab(int index) {
    for (int i = 0; i < 4; ++i) {
        ShowWindow(hTabPages[i], (i == index) ? SW_SHOW : SW_HIDE);
    }
}

// Основний обробник вікна для клієнта
LRESULT CALLBACK ClientWndProcMenu(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_COMMAND:
        // Тут може бути обробка подій для кнопок на вкладках
        break;
    case WM_NOTIFY:
        // Обробка подій вкладок
        if (((LPNMHDR)lp)->hwndFrom == hTab && ((LPNMHDR)lp)->code == TCN_SELCHANGE) {
            int iSel = TabCtrl_GetCurSel(hTab);
            ShowTab(iSel);
        }
        break;
    case WM_CREATE:
        AddTabsClient(hwnd);      // Додавання вкладок до вікна
        InitTabsClient(hwnd);     // Ініціалізація вкладок
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    }

    return DefWindowProc(hwnd, msg, wp, lp);
}

// Основний обробник вікна для сервера
LRESULT CALLBACK ServerWndProcMenu(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_COMMAND:
        // Тут може бути обробка подій для кнопок на вкладках
        break;
    case WM_NOTIFY:
        // Обробка подій вкладок
        if (((LPNMHDR)lp)->hwndFrom == hTab && ((LPNMHDR)lp)->code == TCN_SELCHANGE) {
            int iSel = TabCtrl_GetCurSel(hTab);
            ShowTab(iSel);
        }
        break;
    case WM_CREATE:
        AddTabsServer(hwnd);      // Додавання вкладок до вікна
        InitTabsServer(hwnd);     // Ініціалізація вкладок
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_SIZE: {
        // Отримання нових розмірів вікна
        int width = LOWORD(lp);
        int height = HIWORD(lp);

        // Обчислення масштабу
        float scaleX = (float)width / UI_BASE_WIDTH;
        float scaleY = (float)height / UI_BASE_HEIGHT;

        auto sx = [&](int val) { return (int)(val * scaleX); };
        auto sy = [&](int val) { return (int)(val * scaleY); };

        // Масштабування вкладок
        MoveWindow(hTab, 0, 0, sx(600), 40, TRUE);

        // Масштабування вкладок
        for (int i = 0; i < 4; ++i) {
            MoveWindow(hTabPages[i], 0, 40, sx(600), sy(400), TRUE);
        }

        return 0;
    }
    }

    return DefWindowProc(hwnd, msg, wp, lp);
}
