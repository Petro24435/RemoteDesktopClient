#include "auth.h"
#include "mainmenu.h"
#include <windows.h>
#include <string>
#include <iostream>
#include <sstream>
#include "serverUserRegistration.h"
extern HINSTANCE hInst = NULL;
Auth auth; // ������ ����� ��� �������� ������� �����
// ������� ���������� ��� ��������� ����
//LRESULT CALLBACK WndProcLogin(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
//    switch (msg) {
//    case WM_CREATE:
//        CreateWindowEx(0, L"STATIC", L"Welcome to the main menu!", WS_VISIBLE | WS_CHILD,
//            20, 20, 300, 30, hwnd, NULL, NULL, NULL);
//        break;
//
//    case WM_COMMAND:
//        switch (LOWORD(wp))
//        {
//        case IDC_RADIO_SERVER: // ������
//            
//            break;
//
//        case IDC_RADIO_CLIENT: // ������ ������
//            
//            break;
//        }
//    case WM_DESTROY:
//        PostQuitMessage(0);
//        break;
//    }
//
//    return DefWindowProc(hwnd, msg, wp, lp);
//}

int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow) {
    hInst = hInstance;
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    // ��������� ����� ��� Client ����
    WNDCLASS wcClientMenu = { 0 };
    wcClientMenu.lpfnWndProc = ClientWndProcMenu;
    wcClientMenu.hInstance = hInstance;
    wcClientMenu.lpszClassName = L"ClientAppWindowClass";
    wcClientMenu.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    RegisterClass(&wcClientMenu);

    // ��������� ����� ��� Server ����
    WNDCLASS wcServerMenu = { 0 };
    wcServerMenu.lpfnWndProc = ServerWndProcMenu;
    wcServerMenu.hInstance = hInstance;
    wcServerMenu.lpszClassName = L"ServerAppWindowClass";
    wcServerMenu.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    RegisterClass(&wcServerMenu);


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
    /*if (GetLocalIPAddress() == globalConfig.server_ip) {
        //system("start cmd /k \"cd /d C:\\Users\\nazar\\PycharmProjects\\sqlSPZdb &&  uvicorn main:app --host {globalConfig.server_ip} --port {globalConfig.port}\"");
        std::stringstream ss;
        ss << "start cmd /k \"cd /d C:\\Users\\nazar\\PycharmProjects\\sqlSPZdb && uvicorn main:app --host "
            << globalConfig.server_ip << " --port " << globalConfig.port << "\"";
        system(ss.str().c_str());
    }*/
    // �������� ���� ������� ����������
    MSG msg;
    while (GetMessage(&msg, hwndLogin, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);

        // ���� ���� ������� (loginSuccessful == true), ��������� ���� ���� � ��������� �����
        if (loginSuccessful) {
            ShowWindow(hwndLogin, SW_HIDE); // ��������� ���� �����
            if (serverBool) {


                // ��������� ��������� ����
                HWND hwndServerMenu = CreateWindowEx(
                    0,
                    L"ServerAppWindowClass",  // ������������� ������������� ���� ��� ��������� ����
                    L"Server Menu",
                    WS_OVERLAPPEDWINDOW,  // ��� �����, ������ � ���������
                    CW_USEDEFAULT, CW_USEDEFAULT,      // ���������� ��������� ����� ����
                    screenWidth, screenHeight,  // ����� ���� �� ���� �����
                    NULL, NULL, hInst, NULL
                );

                if (!hwndServerMenu) {
                    DWORD dwError = GetLastError();
                    MessageBox(NULL, L"Failed to create main menu window!", L"Error", MB_OK | MB_ICONERROR);
                    return -1;
                }

                // �������� ������� ����
                ShowWindow(hwndServerMenu, SW_MAXIMIZE);
                UpdateWindow(hwndServerMenu);
            }
            else
            {
                // ��������� ��������� ����
                HWND hwndClientMenu = CreateWindowEx(
                    0,
                    L"ClientAppWindowClass",  // ������������� ������������� ���� ��� ��������� ����
                    L"Client Menu",
                    WS_OVERLAPPEDWINDOW,
                    CW_USEDEFAULT, CW_USEDEFAULT, 600, 400,
                    NULL, NULL, hInst, NULL
                );

                if (!hwndClientMenu) {
                    DWORD dwError = GetLastError();
                    MessageBox(NULL, L"Failed to create main menu window!", L"Error", MB_OK | MB_ICONERROR);
                    return -1;
                }

                // �������� ������� ����
                ShowWindow(hwndClientMenu, nCmdShow);
                UpdateWindow(hwndClientMenu);
            }
            
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
