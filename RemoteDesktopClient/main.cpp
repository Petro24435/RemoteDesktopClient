#include "serverUserRegistration.h"
#include "auth.h"
#include "mainmenu.h"
#include <windows.h>
#include <string>
#include <iostream>
#include <sstream>

HINSTANCE hInst;
Auth auth; // Додаємо змінну для перевірки статусу логіну
//// Обробка повідомлень для головного вікна
//LRESULT CALLBACK WndProcMainMenu(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
//    switch (msg) {
//    case WM_CREATE:
//        CreateWindowEx(0, L"STATIC", L"Welcome to the main menu!", WS_VISIBLE | WS_CHILD,
//            20, 20, 300, 30, hwnd, NULL, NULL, NULL);
//        break;
//
//    case WM_DESTROY:
//        PostQuitMessage(0);
//        break;
//    }
//
//    return DefWindowProc(hwnd, msg, wp, lp);
//}

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow) {
    hInst = hInstance;

    // Реєстрація класу для головного вікна
    WNDCLASS wcMainMenu = { 0 };
    wcMainMenu.lpfnWndProc = MainWndProcMenu;
    wcMainMenu.hInstance = hInstance;
    wcMainMenu.lpszClassName = L"MainAppWindowClass";
    wcMainMenu.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    RegisterClass(&wcMainMenu);


    // Реєстрація класу для логіну
    WNDCLASS wcLogin = { 0 };
    wcLogin.lpfnWndProc = WndProcLogin;   // Процес обробки повідомлень для логіну
    wcLogin.hInstance = hInstance;
    wcLogin.lpszClassName = L"RemoteDesktopClientLogin";
    wcLogin.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    RegisterClass(&wcLogin);

    // Створення вікна логіну
    HWND hwndLogin = CreateWindowEx(
        0,
        L"RemoteDesktopClientLogin",  // Зареєстрований клас для логін-вікна
        L"Remote Desktop Client",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 400, 250,
        NULL, NULL, hInstance, NULL
    );

    if (!hwndLogin) {
        DWORD dwError = GetLastError();
        MessageBox(NULL, L"Window creation failed!", L"Error", MB_OK | MB_ICONERROR);
        std::wstringstream ss;
        ss << L"Error code: " << dwError;
        MessageBox(NULL, ss.str().c_str(), L"Error Code", MB_OK | MB_ICONERROR);
        return -1;
    }

    // Показуємо логін-вікно
    ShowWindow(hwndLogin, nCmdShow);
    UpdateWindow(hwndLogin);
    if (GetLocalIPAddress() == "192.168.0.200") {
        system("start cmd /k \"cd /d C:\\Users\\nazar\\PycharmProjects\\sqlSPZdb &&  uvicorn main:app --host 192.168.0.200 --port 8000\"");
    }

    // Основний цикл обробки повідомлень
    MSG msg;
    while (GetMessage(&msg, hwndLogin, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);

        // Якщо логін успішний (loginSuccessful == true), створюємо нове вікно і закриваємо старе
        if (loginSuccessful) {
            ShowWindow(hwndLogin, SW_HIDE); // Закриваємо вікно логіну

            // Створення головного вікна
            HWND hwndMainMenu = CreateWindowEx(
                0,
                L"MainAppWindowClass",  // Використовуємо зареєстрований клас для головного вікна
                L"Main Menu",
                WS_OVERLAPPEDWINDOW,
                CW_USEDEFAULT, CW_USEDEFAULT, 600, 400,
                NULL, NULL, hInst, NULL
            );

            if (!hwndMainMenu) {
                DWORD dwError = GetLastError();
                MessageBox(NULL, L"Failed to create main menu window!", L"Error", MB_OK | MB_ICONERROR);
                return -1;
            }

            // Показуємо головне вікно
            ShowWindow(hwndMainMenu, nCmdShow);
            UpdateWindow(hwndMainMenu);
            
            // Перериваємо цикл обробки повідомлень для логіну
            break;
        }
    }
    // Продовжуємо обробляти повідомлення для головного вікна
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}
