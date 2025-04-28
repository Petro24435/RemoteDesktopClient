#ifndef AUTH_H
#define AUTH_H

#include <string>
#include <windows.h>
LRESULT CALLBACK WndProcLogin(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
extern bool loginSuccessful;
class Auth {
public:
    static bool Authenticate(const std::string& username, const std::string& password);
    static bool RegisterUser(const std::string& username, const std::string& password);
    static bool ChangeName(const std::string& oldUsername, const std::string& password, const std::string& newUsername);
    static bool ChangePassword(const std::string& username, const std::string& oldPassword, const std::string& newPassword);
};


#endif // AUTH_H
