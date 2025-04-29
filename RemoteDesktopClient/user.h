// user.h
#pragma once
#include <string>

struct UserInfo {
    std::string login;
    std::string password;
    std::string ip;
};

extern UserInfo currentUser;


