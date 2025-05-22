#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <string>
#include "auth.h"
#include "user.h"
#include "main.h"
#include "client_tab.h"
#pragma comment(lib, "ws2_32.lib")


extern bool serverBool = false;
extern bool loginSuccessful = false;
extern Auth auth;
extern HINSTANCE hInst;
UserInfo currentUser;
ClientInfo currentClient;
HWND hwndLabelUsername, hwndEditUsername;
HWND hwndLabelPassword, hwndEditPassword;
HWND hwndButtonLogin, hwndButtonRegister;
HWND hwndLabelHelpInfo, hwndLabelDevelopersInfo;
static HBRUSH hWhiteBrush = CreateSolidBrush(RGB(255, 255, 255)); // білий фон
// Прототипи функцій
LRESULT CALLBACK WndProcLogin(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

// Контроли логіну
void CreateControls(HWND hwnd) {
    CreateWindowEx(0, L"STATIC", L"Підключитися як:", WS_CHILD | WS_VISIBLE,
        0, 0, 0, 0, hwnd, (HMENU)IDC_LABEL_ROLE, NULL, NULL);

    CreateWindowEx(0, L"BUTTON", L"Сервер", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | WS_GROUP,
        0, 0, 0, 0, hwnd, (HMENU)IDC_RADIO_SERVER, NULL, NULL);

    CreateWindowEx(0, L"BUTTON", L"Клієнт", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
        0, 0, 0, 0, hwnd, (HMENU)IDC_RADIO_CLIENT, NULL, NULL);

    hwndLabelUsername = CreateWindowEx(0, L"STATIC", L"Логін:", WS_CHILD | WS_VISIBLE,
        0, 0, 0, 0, hwnd, (HMENU)IDC_LABEL_USERNAME, NULL, NULL);

    hwndEditUsername = CreateWindowEx(0, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_LEFT,
        0, 0, 0, 0, hwnd, (HMENU)IDC_EDIT_USERNAME, NULL, NULL);

    hwndLabelPassword = CreateWindowEx(0, L"STATIC", L"Пароль:", WS_CHILD | WS_VISIBLE,
        0, 0, 0, 0, hwnd, (HMENU)IDC_LABEL_PASSWORD, NULL, NULL);

    hwndEditPassword = CreateWindowEx(0, L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_PASSWORD,
        0, 0, 0, 0, hwnd, (HMENU)IDC_EDIT_PASSWORD, NULL, NULL);

    hwndButtonLogin = CreateWindowEx(0, L"BUTTON", L"Залогінитись", WS_CHILD | WS_VISIBLE,
        0, 0, 0, 0, hwnd, (HMENU)IDC_BUTTON_LOGIN, NULL, NULL);

    hwndButtonRegister = CreateWindowEx(0, L"BUTTON", L"Зареєструватись", WS_CHILD | WS_VISIBLE,
        0, 0, 0, 0, hwnd, (HMENU)IDC_BUTTON_REGISTER, NULL, NULL);

    hwndLabelHelpInfo = CreateWindowEx(0, L"STATIC", L"Вітаємо. Ця програма була розроблена для з'єднання між різними ПК під ОС Windows\
\nДля показу свого екрану виберіть вгорі цього вікна пункт \"Сервер\", та залогіньтесь. Введіть бажаний порт, або виберіть наданий і відкрийте з'єднання\
\nПерейдіть на вкладку \"Управління З'єднанням\" та надайте бажані дозволи Клієнту\
\n\nДля перегляду екрану Сервера виберіть вгорі цього вікна пункт \"Клієнт\", та залогіньтесь\
\nВведіть Логін користувача, з яким плануєте з'єднатись. Введіть також надані ним ключ і порт. Після того натисність \"Підключитись\"\
\nЯкщо ви маєте будь-які питання чи пропозиції напишіть е-листа на тему проблеми на пошту nazarii.antonyk.ki.2022@lpnu.ua", WS_CHILD | WS_VISIBLE,
        0, 0, 0, 0, hwnd, (HMENU)IDC_LABEL_DEVELOPERS_INFO, NULL, NULL);

    hwndLabelDevelopersInfo = CreateWindowEx(0, L"STATIC", L"Developers: Antonyk Nazarii, Solomchak Petro April - May 2025", WS_CHILD | WS_VISIBLE,
        0, 0, 0, 0, hwnd, (HMENU)IDC_LABEL_DEVELOPERS_INFO, NULL, NULL);

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
        CheckRadioButton(hwnd, IDC_RADIO_SERVER, IDC_RADIO_CLIENT, IDC_RADIO_CLIENT); // за замовчуванням — клієнт
        break;

    case WM_COMMAND: {
        if (LOWORD(wp) == IDC_BUTTON_REGISTER || LOWORD(wp) == IDC_BUTTON_LOGIN) {
            char username[256], password[256];
            GetWindowTextA(hwndEditUsername, username, sizeof(username));
            GetWindowTextA(hwndEditPassword, password, sizeof(password));

            std::string strUsername(username);
            std::string strPassword(password);

            if (LOWORD(wp) == IDC_BUTTON_LOGIN) {  // Логін кнопка
                if (auth.Authenticate(strUsername, strPassword)) {
                    loginSuccessful = true;
                    currentUser.login = strUsername;
                    currentUser.password = strPassword;
                    currentUser.ip = GetLocalIPAddress();
                    currentClient.ip = currentUser.ip;

                    MessageBox(hwnd, L"Login successful!", L"Success", MB_OK);
                }
                else {
                    MessageBox(hwnd, L"Invalid credentials!", L"Error", MB_OK | MB_ICONERROR);
                }
            }
            else if (LOWORD(wp) == IDC_BUTTON_REGISTER) {
                if (auth.RegisterUser(strUsername, strPassword)) {
                    MessageBox(hwnd, L"Registration successful!", L"Success", MB_OK);
                }
                else {
                    MessageBox(hwnd, L"Registration failed or user already exists!", L"Error", MB_OK | MB_ICONERROR);
                }
            }
        }
        else if(LOWORD(wp) == IDC_RADIO_SERVER)
        {
            serverBool = true;
        }
        else if(LOWORD(wp) == IDC_RADIO_CLIENT)
        {
            serverBool = false;
        }
        break;
    }

    case WM_SIZE:
    {
        int width = LOWORD(lp);
        int height = HIWORD(lp);

        // Обчислення масштабу
        float scaleX = (float)width / UI_BASE_WIDTH;
        float scaleY = (float)height / UI_BASE_HEIGHT;

        auto sx = [&](int val) { return (int)(val * scaleX); };
        auto sy = [&](int val) { return (int)(val * scaleY); };

        MoveWindow(hwndLabelUsername, sx(UI_X_LABEL), sy(UI_Y_USER),
            sx(UI_WIDTH_LABEL), sy(UI_HEIGHT_LINE), TRUE);

        MoveWindow(hwndEditUsername, sx(UI_X_LABEL + UI_WIDTH_LABEL + 10), sy(UI_Y_USER),
            sx(UI_WIDTH_EDIT), sy(UI_HEIGHT_LINE), TRUE);

        MoveWindow(hwndLabelPassword, sx(UI_X_LABEL), sy(UI_Y_PASS),
            sx(UI_WIDTH_LABEL), sy(UI_HEIGHT_LINE), TRUE);

        MoveWindow(hwndEditPassword, sx(UI_X_LABEL + UI_WIDTH_LABEL + 10), sy(UI_Y_PASS),
            sx(UI_WIDTH_EDIT), sy(UI_HEIGHT_LINE), TRUE);

        MoveWindow(hwndButtonLogin, sx(UI_X_LABEL), sy(UI_Y_BUTTONS),
            sx(UI_WIDTH_BUTTON + 10), sy(UI_HEIGHT_BUTTON), TRUE);

        MoveWindow(hwndButtonRegister, sx(UI_X_LABEL + UI_WIDTH_BUTTON + 30), sy(UI_Y_BUTTONS),
            sx(UI_WIDTH_BUTTON + 10), sy(UI_HEIGHT_BUTTON), TRUE);
        
        MoveWindow(hwndLabelHelpInfo, sx(20), sy(UI_BASE_HEIGHT - UI_HEIGHT_BUTTON -125), sx(UI_BASE_WIDTH - 40), sy(UI_HEIGHT_BUTTON+120), TRUE);

        MoveWindow(hwndLabelDevelopersInfo, sx(20), sy(UI_BASE_HEIGHT - UI_HEIGHT_BUTTON +10 ), sx(UI_BASE_WIDTH - 40), sy(UI_HEIGHT_BUTTON - 10), TRUE);


        MoveWindow(GetDlgItem(hwnd, IDC_LABEL_ROLE), sx(UI_X_LABEL), sy(5),
            sx(UI_WIDTH_LABEL), sy(UI_HEIGHT_LINE), TRUE);

        MoveWindow(GetDlgItem(hwnd, IDC_RADIO_SERVER), sx(UI_X_LABEL + UI_WIDTH_LABEL + 10), sy(5),
            sx(80), sy(UI_HEIGHT_LINE), TRUE);

        MoveWindow(GetDlgItem(hwnd, IDC_RADIO_CLIENT), sx(UI_X_LABEL + UI_WIDTH_LABEL + 100), sy(5),
            sx(80), sy(UI_HEIGHT_LINE), TRUE);

        if (hFont) DeleteObject(hFont);
        int fontSize = min(sx(16), sy(16));
        hFont = CreateFont(fontSize, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
            DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");

        HWND controls[] = {
            hwndLabelUsername, hwndEditUsername,
            hwndLabelPassword, hwndEditPassword,
            hwndButtonLogin, hwndButtonRegister,
            hwndLabelHelpInfo, hwndLabelDevelopersInfo,
            GetDlgItem(hwnd, IDC_LABEL_ROLE),
            GetDlgItem(hwnd, IDC_RADIO_SERVER),
            GetDlgItem(hwnd, IDC_RADIO_CLIENT),

        };

        for (HWND ctrl : controls) {
            SendMessage(ctrl, WM_SETFONT, (WPARAM)hFont, TRUE);
        }
        break;
    }
    case WM_CTLCOLORSTATIC:
    case WM_CTLCOLORBTN:
    {
        HDC hdcStatic = (HDC)wp;
        SetBkMode(hdcStatic, TRANSPARENT); // фон прозорий
        SetBkColor(hdcStatic, RGB(255, 255, 255)); // білий фон
        return (INT_PTR)hWhiteBrush;
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
