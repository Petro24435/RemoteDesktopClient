#include "libraries.h"
#include "profile_tab.h"
#include "user.h"
#include "auth.h"


extern UserInfo currentUser;
HWND hLoginEdit; // ������ �� ������� �����

// ������� ��� ��������� �������� ������� Profile
void DrawProfileTab(HWND hwnd) {
    std::wstring wip(currentUser.login.begin(), currentUser.login.end());

    // ��������� ����� ��� ���� �����������
    CreateWindowEx(0, L"STATIC", L"��� �����������:", WS_CHILD | WS_VISIBLE,
        20, 20, 120, 20, hwnd, NULL, NULL, NULL);

    // ���� ��� ����������� ���� �����������
    hLoginEdit = CreateWindowEx(0, L"EDIT", wip.c_str(), WS_CHILD | WS_VISIBLE | WS_BORDER,
        150, 20, 200, 25, hwnd, NULL, NULL, NULL);


    // ���� ��� ����������� ���� �����������
    CreateWindowEx(0, L"BUTTON", L"������ ����", WS_CHILD | WS_VISIBLE | WS_BORDER,
        370, 20, 150, 25, hwnd, (HMENU)1001, NULL, NULL);

    // ������ ��� ���� ������
    CreateWindowEx(0, L"BUTTON", L"������ ������", WS_CHILD | WS_VISIBLE,
        150, 60, 150, 30, hwnd, (HMENU)1002, NULL, NULL);
}

// ������� ��� ������� ���� �� ������� Profile
LRESULT CALLBACK ProfileTabWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_COMMAND: {
        // �������� �� ���������� ������ "������ ������"
        if (LOWORD(wParam) == 1001) {
            wchar_t buffer[100];
            GetWindowText(hLoginEdit, buffer, 100);

            std::wstring wnewLogin(buffer);
            std::string newLogin(wnewLogin.begin(), wnewLogin.end());

            if (newLogin.empty()) {
                MessageBox(hwnd, L"����� ���� �� ���� ���� �������!", L"�������", MB_OK | MB_ICONERROR);
                break;
            }

            // ������ ������ ����
            if (Auth::ChangeName(currentUser.login, currentUser.password, newLogin)) {
                currentUser.login = newLogin;

                MessageBox(hwnd, L"��� ����������� ������ ������!", L"����", MB_OK | MB_ICONINFORMATION);
            }
            else {
                MessageBox(hwnd, L"�� ������� ������ ��� �����������. �������, ���� ��� �������.", L"�������", MB_OK | MB_ICONERROR);
            }
        }

        if (LOWORD(wParam) == 1002) {
            ShowPasswordChangeDialog(hwnd);
        }
        break;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

// ����������� ������� Profile
void InitProfileTab(HWND hwnd) {
    DrawProfileTab(hwnd);

    // ������������ ��������� ������� ����
    SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)ProfileTabWndProc);
}
void ShowPasswordChangeDialog(HWND parentHwnd) {
    const wchar_t* className = L"PasswordChangeWindow";

    WNDCLASSW wc = { 0 };
    wc.lpfnWndProc = PasswordWndProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = className;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    RegisterClassW(&wc);

    HWND hwnd = CreateWindowExW(
        0, className, L"���� ������",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, 340, 200,
        parentHwnd, NULL, GetModuleHandle(NULL), NULL
    );
}
