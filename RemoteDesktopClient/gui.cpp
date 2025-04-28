#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <windows.h>
#include <string>
#include <ws2tcpip.h>
#include "auth.h"
#include "user.h"
#pragma comment(lib, "ws2_32.lib")



// Визначаємо ці змінні
HWND hUsernameEdit = NULL;
HWND hPasswordEdit = NULL;
extern bool loginSuccessful = false;
extern Auth auth;
extern HINSTANCE hInst;
UserInfo currentUser;
// Прототипи функцій
LRESULT CALLBACK WndProcLogin(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

// Контроли логіну
void CreateControls(HWND hwnd) {
    CreateWindowEx(0, L"STATIC", L"Username:", WS_VISIBLE | WS_CHILD,
        20, 50, 100, 25, hwnd, (HMENU)1, NULL, NULL);
    hUsernameEdit = CreateWindowEx(0, L"EDIT", NULL, WS_VISIBLE | WS_CHILD | WS_BORDER,
        130, 50, 200, 25, hwnd, (HMENU)2, NULL, NULL);

    CreateWindowEx(0, L"STATIC", L"Password:", WS_VISIBLE | WS_CHILD,
        20, 90, 100, 25, hwnd, (HMENU)3, NULL, NULL);
    hPasswordEdit = CreateWindowEx(0, L"EDIT", NULL, WS_VISIBLE | WS_CHILD | WS_BORDER | ES_PASSWORD,
        130, 90, 200, 25, hwnd, (HMENU)4, NULL, NULL);

    CreateWindowEx(0, L"BUTTON", L"Login", WS_VISIBLE | WS_CHILD,
        130, 130, 100, 30, hwnd, (HMENU)5, NULL, NULL);
    CreateWindowEx(0, L"BUTTON", L"Register", WS_VISIBLE | WS_CHILD,
        240, 130, 100, 30, hwnd, (HMENU)6, NULL, NULL);
}
// ФУНКЦІЯ ДЛЯ ОТРИМАННЯ IP, можливо потім треба буде змінити якщо не буде пахати ця. 18.04.2025 16:02
std::string GetLocalIPAddress() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) == SOCKET_ERROR) {
        WSACleanup();
        return "";
    }

    struct hostent* host = gethostbyname(hostname);
    if (!host) {
        WSACleanup();
        return "";
    }

    struct in_addr addr;
    memcpy(&addr, host->h_addr_list[0], sizeof(struct in_addr));
    std::string ipStr = inet_ntoa(addr);
    WSACleanup();

    // Конвертація у wstring
    return std::string(ipStr.begin(), ipStr.end());
}
// Обробка повідомлень для вікна логіну
LRESULT CALLBACK WndProcLogin(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_CREATE:
        CreateControls(hwnd);
        break;

    case WM_COMMAND: {
        if (LOWORD(wp) == 5 || LOWORD(wp) == 6) {
            char username[256], password[256];
            GetWindowTextA(hUsernameEdit, username, sizeof(username));
            GetWindowTextA(hPasswordEdit, password, sizeof(password));

            std::string strUsername(username);
            std::string strPassword(password);

            if (LOWORD(wp) == 5) {  // Логін кнопка
                if (auth.Authenticate(strUsername, strPassword)) {
                    loginSuccessful = true;
                    currentUser.login = strUsername;
                    currentUser.password = strPassword;
                    currentUser.ip = GetLocalIPAddress();

                    //MessageBox(hwnd, L"Login successful!", L"Success", MB_OK);
                }
                else {
                    MessageBox(hwnd, L"Invalid credentials!", L"Error", MB_OK | MB_ICONERROR);
                }
            }
            else if (LOWORD(wp) == 6) {
                if (auth.RegisterUser(strUsername, strPassword)) {
                    MessageBox(hwnd, L"Registration successful!", L"Success", MB_OK);
                }
                else {
                    MessageBox(hwnd, L"Registration failed or user already exists!", L"Error", MB_OK | MB_ICONERROR);
                }
            }
        }
        break;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    }

    return DefWindowProc(hwnd, msg, wp, lp);
}

// Реєстрація класу для логін-вікна
void RegisterLoginClass(HINSTANCE hInstance) {
    WNDCLASS wcLogin = { 0 };
    wcLogin.lpfnWndProc = WndProcLogin;
    wcLogin.hInstance = hInstance;
    wcLogin.lpszClassName = L"RemoteDesktopClientLogin";
    wcLogin.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    RegisterClass(&wcLogin);
}
