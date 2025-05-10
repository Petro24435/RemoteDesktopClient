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
HWND hTabPages[4]; // 4 �������

// ��������� ������� �� TabControl
void AddTabsClient(HWND hwnd) {
    hTab = CreateWindowEx(0, WC_TABCONTROL, L"",
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
        0, 0, 600, 40,
        hwnd, NULL, NULL, NULL);

    TCITEM tie = { 0 };
    tie.mask = TCIF_TEXT;

    wchar_t* tabNames[4] = { (LPWSTR)L"�� ���", (LPWSTR)L"������ ����������", (LPWSTR)L"ϳ���������", (LPWSTR)L"������� �'�������" };

    // ��������� ���� �������
    for (int i = 0; i < 4; i++) {
        tie.pszText = tabNames[i];
        TabCtrl_InsertItem(hTab, i, &tie);
    }
}

// ����������� �������
void InitTabsClient(HWND hwnd) {
    // ��������� ������� �������
    for (int i = 0; i < 4; ++i) {
        hTabPages[i] = CreateWindowEx(0, L"STATIC", L"",
            WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
            0, 60, 600, 400, hwnd, NULL, NULL, NULL);

        // ������� �� ������� �� �������
        ShowWindow(hTabPages[i], (i == 0) ? SW_SHOW : SW_HIDE);
    }

    // ����������� �������
    InitProfileTab(hTabPages[0]);
    InitLastConnectionsTab(hTabPages[1]);
    //InitConnectToTab(hTabPages[2]);
    //InitActiveConnetionTab(hTabPages[3]);
}

// ��������� ������� �� TabControl
void AddTabsServer(HWND hwnd) {
    hTab = CreateWindowEx(0, WC_TABCONTROL, L"",
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
        0, 0, 600, 40,
        hwnd, NULL, NULL, NULL);

    TCITEM tie = { 0 };
    tie.mask = TCIF_TEXT;

    wchar_t* tabNames[4] = { (LPWSTR)L"̳� �������", (LPWSTR)L"������ ����������", (LPWSTR)L"³������� �'������� ", (LPWSTR)L"��������� �'���������" };

    // ��������� ���� �������
    for (int i = 0; i < 4; i++) {
        tie.pszText = tabNames[i];
        TabCtrl_InsertItem(hTab, i, &tie);
    }
}

// ����������� �������
void InitTabsServer(HWND hwnd) {
    // ��������� ������� �������
    for (int i = 0; i < 4; ++i) {
        hTabPages[i] = CreateWindowEx(0, L"STATIC", L"",
            WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
            0, 60, 600, 400, hwnd, NULL, NULL, NULL);

        // ������� �� ������� �� �������
        ShowWindow(hTabPages[i], (i == 0) ? SW_SHOW : SW_HIDE);
    }

    // ����������� �������
    InitProfileTab(hTabPages[0]);
    InitLastConnectionsTab(hTabPages[1]);
    InitOpenConnectTab(hTabPages[2]);
    //InitServerTab(hTabPages[3]);
    InitConnectManagingTab(hTabPages[3]);
}

// ����������� �������
void ShowTab(int index) {
    for (int i = 0; i < 4; ++i) {
        ShowWindow(hTabPages[i], (i == index) ? SW_SHOW : SW_HIDE);
    }
}

// �������� �������� ���� ��� �볺���
LRESULT CALLBACK ClientWndProcMenu(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_COMMAND:
        // ��� ���� ���� ������� ���� ��� ������ �� ��������
        break;
    case WM_NOTIFY:
        // ������� ���� �������
        if (((LPNMHDR)lp)->hwndFrom == hTab && ((LPNMHDR)lp)->code == TCN_SELCHANGE) {
            int iSel = TabCtrl_GetCurSel(hTab);
            ShowTab(iSel);
        }
        break;
    case WM_CREATE:
        AddTabsClient(hwnd);      // ��������� ������� �� ����
        InitTabsClient(hwnd);     // ����������� �������
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    }

    return DefWindowProc(hwnd, msg, wp, lp);
}

// �������� �������� ���� ��� �������
LRESULT CALLBACK ServerWndProcMenu(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_COMMAND:
        // ��� ���� ���� ������� ���� ��� ������ �� ��������
        break;
    case WM_NOTIFY:
        // ������� ���� �������
        if (((LPNMHDR)lp)->hwndFrom == hTab && ((LPNMHDR)lp)->code == TCN_SELCHANGE) {
            int iSel = TabCtrl_GetCurSel(hTab);
            ShowTab(iSel);
        }
        break;
    case WM_CREATE:
        AddTabsServer(hwnd);      // ��������� ������� �� ����
        InitTabsServer(hwnd);     // ����������� �������
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_SIZE: {
        // ��������� ����� ������ ����
        int width = LOWORD(lp);
        int height = HIWORD(lp);

        // ���������� ��������
        float scaleX = (float)width / UI_BASE_WIDTH;
        float scaleY = (float)height / UI_BASE_HEIGHT;

        auto sx = [&](int val) { return (int)(val * scaleX); };
        auto sy = [&](int val) { return (int)(val * scaleY); };

        // ������������� �������
        MoveWindow(hTab, 0, 0, sx(600), 40, TRUE);

        // ������������� �������
        for (int i = 0; i < 4; ++i) {
            MoveWindow(hTabPages[i], 0, 40, sx(600), sy(400), TRUE);
        }

        return 0;
    }
    }

    return DefWindowProc(hwnd, msg, wp, lp);
}
