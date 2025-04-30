//#include "mainmenu.h"
//#include "user.h"
//#include <windows.h>
//#include <commctrl.h>
//#pragma comment(lib, "comctl32.lib")
//
//HWND hTab;
//HWND hTabPages[4]; // 4 вкладки
//extern bool loginSuccessful;
//HWND hShowKeyBtn;
//HWND hKeyEdit;
//bool isPasswordVisible = false;
//LRESULT CALLBACK MainWndProcMenu(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
//void AddTabs(HWND hwnd) {
//    hTab = CreateWindowEx(0, WC_TABCONTROL, L"",
//        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
//        0, 0, 600, 40,
//        hwnd, NULL, NULL, NULL);
//
//    TCITEM tie = { 0 };
//    tie.mask = TCIF_TEXT;
//
//    wchar_t* tabNames[4] = { (LPWSTR)L"Мій профіль", (LPWSTR)L"Мої друзі", (LPWSTR)L"Клієнт", (LPWSTR)L"Сервер" };
//
//    for (int i = 0; i < 4; i++) {
//        tie.pszText = tabNames[i];
//        TabCtrl_InsertItem(hTab, i, &tie);
//    }
//}
//
//HWND CreateTabPage(HWND hwndParent) {
//    return CreateWindowEx(0, L"STATIC", NULL,
//        WS_CHILD | WS_VISIBLE,
//        0, 40, 600, 360, hwndParent, NULL, NULL, NULL);
//}
//
//void DrawProfileTab(HWND hwnd) {
//    std::wstring wip(currentUser.login.begin(), currentUser.login.end());
//    CreateWindowEx(0, L"STATIC", L"Ім’я користувача:", WS_CHILD | WS_VISIBLE,
//        20, 20, 120, 20, hwnd, NULL, NULL, NULL);
//
//    CreateWindowEx(0, L"EDIT", wip.c_str(), WS_CHILD | WS_VISIBLE | WS_BORDER,
//        150, 20, 200, 25, hwnd, (HMENU)1001, NULL, NULL);
//
//    CreateWindowEx(0, L"BUTTON", L"Змінити пароль", WS_CHILD | WS_VISIBLE,
//        150, 60, 150, 30, hwnd, (HMENU)1002, NULL, NULL);
//}
//
//void DrawFriendsTab(HWND hwnd) {
//    CreateWindowEx(0, L"LISTBOX", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | LBS_NOTIFY,
//        20, 20, 300, 200, hwnd, (HMENU)2001, NULL, NULL);
//}
//
//void DrawClientTab(HWND hwnd) {
//    CreateWindowEx(0, L"STATIC", L"IP адреса:", WS_CHILD | WS_VISIBLE,
//        20, 20, 100, 20, hwnd, NULL, NULL, NULL);
//    CreateWindowEx(0, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER,
//        130, 20, 200, 25, hwnd, (HMENU)3001, NULL, NULL);
//
//    CreateWindowEx(0, L"STATIC", L"Логін:", WS_CHILD | WS_VISIBLE,
//        20, 60, 100, 20, hwnd, NULL, NULL, NULL);
//    CreateWindowEx(0, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER,
//        130, 60, 200, 25, hwnd, (HMENU)3002, NULL, NULL);
//
//    CreateWindowEx(0, L"STATIC", L"Порт:", WS_CHILD | WS_VISIBLE,
//        20, 100, 100, 20, hwnd, NULL, NULL, NULL);
//    CreateWindowEx(0, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER,
//        130, 100, 100, 25, hwnd, (HMENU)3003, NULL, NULL);
//}
//
//void DrawServerTab(HWND hwnd) {
//    std::wstring wip(currentUser.ip.begin(), currentUser.ip.end());
//    CreateWindowEx(0, L"STATIC", L"Моя IP адреса:", WS_CHILD | WS_VISIBLE,
//        20, 20, 100, 20, hwnd, NULL, NULL, NULL);
//    CreateWindowEx(0, L"EDIT", wip.c_str(), WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY,
//        130, 20, 200, 25, hwnd, (HMENU)4001, NULL, NULL);
//
//// Створення радіо-кнопки "Глядач" (за замовчуванням вибрана)
//CreateWindowEx(0, L"BUTTON", L"Глядач", WS_CHILD | WS_VISIBLE | BS_RADIOBUTTON | WS_GROUP,
//    20, 60, 150, 20, hwnd, (HMENU)4002, NULL, NULL);
//
//// Створення радіо-кнопки "Повний доступ"
//CreateWindowEx(0, L"BUTTON", L"Повний доступ", WS_CHILD | WS_VISIBLE | BS_RADIOBUTTON,
//    20, 90, 150, 20, hwnd, (HMENU)4003, NULL, NULL);
//
//
//    CreateWindowEx(0, L"STATIC", L"Порт:", WS_CHILD | WS_VISIBLE,
//        20, 130, 100, 20, hwnd, NULL, NULL, NULL);
//    CreateWindowEx(0, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER,
//        130, 130, 100, 25, hwnd, (HMENU)4004, NULL, NULL);
//
//    CreateWindowEx(0, L"STATIC", L"Ключ:", WS_VISIBLE | WS_CHILD,
//        20, 170, 100, 20, hwnd, NULL, NULL, NULL);
//    hKeyEdit = CreateWindowEx(0, L"EDIT", NULL, WS_VISIBLE | WS_CHILD | WS_BORDER ,
//        130, 170, 100, 25, hwnd, (HMENU)4005, NULL, NULL);
//    //CreateWindowEx(0, L"BUTTON", L"О_О", WS_VISIBLE | WS_CHILD,
//    //    240, 170, 30, 25, hwnd, (HMENU)5001, NULL, NULL);
//
//    CreateWindowEx(0, L"STATIC", L"Логін:", WS_CHILD | WS_VISIBLE,
//        20, 210, 100, 20, hwnd, NULL, NULL, NULL);
//    CreateWindowEx(0, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY,
//        130, 210, 200, 25, hwnd, (HMENU)4006, NULL, NULL);
//}
//
//void InitTabs(HWND hwnd) {
//    for (int i = 0; i < 4; ++i) {
//        hTabPages[i] = CreateTabPage(hwnd);
//        ShowWindow(hTabPages[i], (i == 0) ? SW_SHOW : SW_HIDE);
//    }
//
//    DrawProfileTab(hTabPages[0]);
//    DrawFriendsTab(hTabPages[1]);
//    DrawClientTab(hTabPages[2]);
//    DrawServerTab(hTabPages[3]);
//}
//
//void ShowTab(int index) {
//    for (int i = 0; i < 4; ++i) {
//        ShowWindow(hTabPages[i], (i == index) ? SW_SHOW : SW_HIDE);
//    }
//}
//
//LRESULT CALLBACK MainWndProcMenu(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
//    switch (msg) {
//    case WM_COMMAND:
//        switch (LOWORD(wp)) {
//        /*case 5001: // Кнопка "око"
//        {
//            isPasswordVisible = !isPasswordVisible;
//
//            wchar_t buffer[256];
//            GetWindowTextW(hKeyEdit, buffer, 256);
//
//            // Знищуємо старий контрол
//            DestroyWindow(hKeyEdit);
//
//            // Створюємо новий стиль
//            DWORD style = WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL;
//            if (!isPasswordVisible) {
//                style |= ES_PASSWORD;
//            }
//
//            // Створюємо нове поле на тій самій вкладці з тим самим HMENU
//            hKeyEdit = CreateWindowEx(
//                0,
//                L"EDIT",
//                buffer,
//                style,
//                130, 170, 100, 25,
//                hTabPages[3],    // ВАЖЛИВО! Має бути вкладка, не головне вікно
//                (HMENU)4005,
//                (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
//                NULL
//            );
//
//            SetFocus(hKeyEdit);
//            break;
//        }*/
//        }
//        break;
//
//    case WM_CREATE:
//        AddTabs(hwnd);
//        InitTabs(hwnd);
//        break;
//
//    case WM_NOTIFY: {
//        LPNMHDR lpnmhdr = (LPNMHDR)lp;
//        if (lpnmhdr->hwndFrom == hTab && lpnmhdr->code == TCN_SELCHANGE) {
//            int iSel = TabCtrl_GetCurSel(hTab);
//            ShowTab(iSel);
//        }
//        break;
//    }
//
//    case WM_DESTROY:
//        PostQuitMessage(0);
//        break;
//    }
//
//    return DefWindowProc(hwnd, msg, wp, lp);
//}
//
//void InitMainWindow(HINSTANCE hInstance) {
//    WNDCLASS wc = { 0 };
//    wc.lpfnWndProc = MainWndProcMenu;
//    wc.hInstance = hInstance;
//    wc.lpszClassName = L"MainAppWindowClass";
//    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
//    RegisterClass(&wc);
//}
#include "mainmenu.h"
#include "user.h"
#include <commctrl.h>
#include "mainmenu.h"
#include "profile_tab.h"
#include "friends_tab.h"
#include "client_tab.h"
#include "server_tab.h"

#pragma comment(lib, "comctl32.lib")

#include "mainmenu.h"
#include "profile_tab.h"
#include "friends_tab.h"
#include "client_tab.h"
#include "server_tab.h"
#include <windows.h>
HWND hTab;
HWND hTabPages[4]; // 4 вкладки

// Додавання вкладок до TabControl
void AddTabs(HWND hwnd) {
    hTab = CreateWindowEx(0, WC_TABCONTROL, L"",
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
        0, 0, 600, 40,
        hwnd, NULL, NULL, NULL);

    TCITEM tie = { 0 };
    tie.mask = TCIF_TEXT;

    wchar_t* tabNames[4] = { (LPWSTR)L"Мій профіль", (LPWSTR)L"Мої друзі", (LPWSTR)L"Клієнт", (LPWSTR)L"Сервер" };

    // Додавання назв вкладок
    for (int i = 0; i < 4; i++) {
        tie.pszText = tabNames[i];
        TabCtrl_InsertItem(hTab, i, &tie);
    }
}

// Ініціалізація вкладок
void InitTabs(HWND hwnd) {
    // Створення окремих вкладок
    for (int i = 0; i < 4; ++i) {
        hTabPages[i] = CreateWindowEx(0, L"STATIC", L"",
            WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
            0, 40, 600, 400, hwnd, NULL, NULL, NULL);

        // Сховати всі вкладки на початку
        ShowWindow(hTabPages[i], (i == 0) ? SW_SHOW : SW_HIDE);
    }

    // Ініціалізація вкладок
    InitProfileTab(hTabPages[0]);
    InitFriendsTab(hTabPages[1]);
    InitClientTab(hTabPages[2]);
    InitServerTab(hTabPages[3]);
}

// Перемикання вкладок
void ShowTab(int index) {
    for (int i = 0; i < 4; ++i) {
        ShowWindow(hTabPages[i], (i == index) ? SW_SHOW : SW_HIDE);
    }
}

// Основний обробник вікна
LRESULT CALLBACK MainWndProcMenu(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
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
        AddTabs(hwnd);      // Додавання вкладок до вікна
        InitTabs(hwnd);     // Ініціалізація вкладок
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    }

    return DefWindowProc(hwnd, msg, wp, lp);
}
