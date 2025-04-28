#ifndef PROFILE_TAB_H
#define PROFILE_TAB_H

#include <windows.h>
#include <string>
#include "user.h"

extern UserInfo currentUser;

void DrawProfileTab(HWND hwnd);
void InitProfileTab(HWND hwnd);
void ShowPasswordChangeDialog(HWND parentHwnd);
INT_PTR CALLBACK PasswordWndProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
#endif // PROFILE_TAB_H
