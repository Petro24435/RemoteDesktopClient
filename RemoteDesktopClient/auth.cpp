#include "auth.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>


bool Auth::Authenticate(const std::string& username, const std::string& password) {
    std::ifstream file("users.csv");
    if (!file.is_open()) {
        std::cerr << "Failed to open the CSV file!" << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string storedUsername, storedPassword;

        std::getline(ss, storedUsername, ',');
        std::getline(ss, storedPassword, ',');

        if (username == storedUsername && password == storedPassword) {
            return true;
        }
    }

    return false;
}

bool Auth::RegisterUser(const std::string& username, const std::string& password) {
    std::ifstream file("users.csv");
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            std::stringstream ss(line);
            std::string storedUsername;
            std::getline(ss, storedUsername, ',');

            if (username == storedUsername) {
                return false; // Користувач уже існує
            }
        }
        file.close();
    }

    std::ofstream outFile("users.csv", std::ios::app);
    if (!outFile.is_open()) {
        std::cerr << "Failed to open the CSV file for writing!" << std::endl;
        return false;
    }

    outFile << username << "," << password << std::endl;
    return true;
}

bool Auth::ChangeName(const std::string& oldUsername, const std::string& password, const std::string& newUsername) {
    std::ifstream file("users.csv");
    if (!file.is_open()) {
        std::cerr << "Не вдалося відкрити файл для читання!" << std::endl;
        return false;
    }

    std::vector<std::pair<std::string, std::string>> users;
    std::string line;
    bool found = false;

    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string username, pass;
        std::getline(ss, username, ',');
        std::getline(ss, pass, ',');

        if (username == oldUsername && pass == password) {
            users.emplace_back(newUsername, pass); // Змінюємо ім’я
            found = true;
        }
        else if (username == newUsername) {
            std::cerr << "Користувач із таким новим іменем вже існує!" << std::endl;
            return false;
        }
        else {
            users.emplace_back(username, pass);
        }
    }
    file.close();

    if (!found) {
        std::cerr << "Старе ім’я користувача або пароль невірні!" << std::endl;
        return false;
    }

    std::ofstream outFile("users.csv", std::ios::trunc);
    if (!outFile.is_open()) {
        std::cerr << "Не вдалося відкрити файл для запису!" << std::endl;
        return false;
    }

    for (const auto& user : users) {
        outFile << user.first << "," << user.second << "\n";
    }

    return true;
}


bool Auth::ChangePassword(const std::string& username, const std::string& oldPassword, const std::string& newPassword) {
    std::ifstream file("users.csv");
    if (!file.is_open()) return false;

    std::vector<std::pair<std::string, std::string>> users;
    bool found = false;

    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string uname, pass;
        std::getline(ss, uname, ',');
        std::getline(ss, pass);

        if (uname == username && pass == oldPassword) {
            users.emplace_back(uname, newPassword);
            found = true;
        }
        else {
            users.emplace_back(uname, pass);
        }
    }
    file.close();

    if (!found) return false;

    std::ofstream outFile("users.csv", std::ios::trunc);
    for (const auto& user : users) {
        outFile << user.first << "," << user.second << "\n";
    }
    return true;
}
