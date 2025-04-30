#include "serverUserRegistration.h"
#include "auth.h"
#include "mainmenu.h"
#include <windows.h>
#include <string>
#include <iostream>
#include <sstream>

HINSTANCE hInst;
Auth auth; // ������ ����� ��� �������� ������� �����
//// ������� ���������� ��� ��������� ����
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

    // ��������� ����� ��� ��������� ����
    WNDCLASS wcMainMenu = { 0 };
    wcMainMenu.lpfnWndProc = MainWndProcMenu;
    wcMainMenu.hInstance = hInstance;
    wcMainMenu.lpszClassName = L"MainAppWindowClass";
    wcMainMenu.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    RegisterClass(&wcMainMenu);


    // ��������� ����� ��� �����
    WNDCLASS wcLogin = { 0 };
    wcLogin.lpfnWndProc = WndProcLogin;   // ������ ������� ���������� ��� �����
    wcLogin.hInstance = hInstance;
    wcLogin.lpszClassName = L"RemoteDesktopClientLogin";
    wcLogin.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    RegisterClass(&wcLogin);

    // ��������� ���� �����
    HWND hwndLogin = CreateWindowEx(
        0,
        L"RemoteDesktopClientLogin",  // ������������� ���� ��� ����-����
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

    // �������� ����-����
    ShowWindow(hwndLogin, nCmdShow);
    UpdateWindow(hwndLogin);
    if (GetLocalIPAddress() == "192.168.0.200") {
        system("start cmd /k \"cd /d C:\\Users\\nazar\\PycharmProjects\\sqlSPZdb &&  uvicorn main:app --host 192.168.0.200 --port 8000\"");
    }

    // �������� ���� ������� ����������
    MSG msg;
    while (GetMessage(&msg, hwndLogin, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);

        // ���� ���� ������� (loginSuccessful == true), ��������� ���� ���� � ��������� �����
        if (loginSuccessful) {
            ShowWindow(hwndLogin, SW_HIDE); // ��������� ���� �����

            // ��������� ��������� ����
            HWND hwndMainMenu = CreateWindowEx(
                0,
                L"MainAppWindowClass",  // ������������� ������������� ���� ��� ��������� ����
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

            // �������� ������� ����
            ShowWindow(hwndMainMenu, nCmdShow);
            UpdateWindow(hwndMainMenu);
            
            // ���������� ���� ������� ���������� ��� �����
            break;
        }
    }
    // ���������� ��������� ����������� ��� ��������� ����
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}
