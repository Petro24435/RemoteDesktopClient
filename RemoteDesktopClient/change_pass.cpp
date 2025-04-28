#include <windows.h>
#include <string>
#include "auth.h"
#include "user.h"

extern UserInfo currentUser;

HWND hOldPassEdit, hNewPassEdit, hConfirmPassEdit;

std::string WideToUTF8(const std::wstring& wstr) {
    if (wstr.empty()) return {};
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
}

LRESULT CALLBACK PasswordWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE:
        CreateWindowW(L"STATIC", L"Старий пароль:", WS_CHILD | WS_VISIBLE,
            20, 20, 100, 20, hwnd, NULL, NULL, NULL);
        hOldPassEdit = CreateWindowW(L"EDIT", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_PASSWORD,
            140, 20, 150, 20, hwnd, NULL, NULL, NULL);

        CreateWindowW(L"STATIC", L"Новий пароль:", WS_CHILD | WS_VISIBLE,
            20, 50, 100, 20, hwnd, NULL, NULL, NULL);
        hNewPassEdit = CreateWindowW(L"EDIT", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_PASSWORD,
            140, 50, 150, 20, hwnd, NULL, NULL, NULL);

        CreateWindowW(L"STATIC", L"Підтвердити пароль:", WS_CHILD | WS_VISIBLE,
            20, 80, 120, 20, hwnd, NULL, NULL, NULL);
        hConfirmPassEdit = CreateWindowW(L"EDIT", NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | ES_PASSWORD,
            140, 80, 150, 20, hwnd, NULL, NULL, NULL);

        CreateWindowW(L"BUTTON", L"Змінити", WS_CHILD | WS_VISIBLE,
            110, 120, 100, 25, hwnd, (HMENU)1, NULL, NULL);
        break;

    case WM_COMMAND:
        if (LOWORD(wParam) == 1) {
            wchar_t oldPass[100], newPass[100], confirmPass[100];
            GetWindowTextW(hOldPassEdit, oldPass, 100);
            GetWindowTextW(hNewPassEdit, newPass, 100);
            GetWindowTextW(hConfirmPassEdit, confirmPass, 100);

            std::string oldP = WideToUTF8(oldPass);
            std::string newP = WideToUTF8(newPass);
            std::string confirmP = WideToUTF8(confirmPass);

            if (newP != confirmP) {
                MessageBoxW(hwnd, L"Новий пароль не співпадає з підтвердженням!", L"Помилка", MB_OK | MB_ICONERROR);
                return 0;
            }

            if (!Auth::Authenticate(currentUser.login, oldP)) {
                MessageBoxW(hwnd, L"Старий пароль неправильний!", L"Помилка", MB_OK | MB_ICONERROR);
                return 0;
            }

            if (Auth::ChangePassword(currentUser.login, oldP, newP)) {
                currentUser.password = newP;
                MessageBoxW(hwnd, L"Пароль змінено успішно!", L"Успіх", MB_OK | MB_ICONINFORMATION);
                DestroyWindow(hwnd);
            }
            else {
                MessageBoxW(hwnd, L"Не вдалося змінити пароль!", L"Помилка", MB_OK | MB_ICONERROR);
            }
        }
        break;

    case WM_CLOSE:
        DestroyWindow(hwnd);
        break;

    default:
        return DefWindowProcW(hwnd, msg, wParam, lParam);
    }
    return 0;
}
