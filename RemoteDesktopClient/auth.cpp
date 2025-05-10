
#include <iostream>
#include <curl/curl.h>
#include "auth.h"
#include "serverUserRegistration.h"
// Ініціалізація глобальної змінної
ServerConfig globalConfig;

bool Auth::RegisterUser(const std::string& username, const std::string& password) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "CURL init failed!" << std::endl;
        return false;
    }

    std::string url = globalConfig.GetBaseUrl() + "/add_user/";
    std::string json_data = "{\"login\": \"" + username + "\", \"pass\": \"" + password + "\"}";

    std::string response_string;
    struct curl_slist* headers = curl_slist_append(nullptr, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_data.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);

    CURLcode res = curl_easy_perform(curl);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        std::cerr << "CURL request failed: " << curl_easy_strerror(res) << std::endl;
        return false;
    }

    if (response_string.find("error") != std::string::npos) {
        std::cerr << "Server error: " << response_string << std::endl;
        return false;
    }

    return true;
}

bool Auth::Authenticate(const std::string& username, const std::string& password) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        //std::cerr << "CURL init failed!" << std::endl;
        return false;
    }

    std::string url = globalConfig.GetBaseUrl() + "/get_user/?login=" + username;

    std::string response_string;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);

    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        //std::cerr << "CURL request failed: " << curl_easy_strerror(res) << std::endl;
        return false;
    }

    // Очікується, що сервер повертає JSON з паролем, напр.: {"pass": "1234"}
    size_t pos = response_string.find("\"pass\":");
    if (pos == std::string::npos) {
        std::cerr << "Invalid response or user not found: " << response_string << std::endl;
        return false;
    }

    std::string returned_pass = response_string.substr(pos + 8);
    returned_pass = returned_pass.substr(0, returned_pass.find('"'));

    return returned_pass == password;
}

bool Auth::ChangeName(const std::string& oldUsername, const std::string& password, const std::string& newUsername) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "CURL init failed!" << std::endl;
        return false;
    }

    std::string url = globalConfig.GetBaseUrl() + "/change_user_name/";
    std::string json_data = "{\"oldUsername\": \"" + oldUsername + "\", \"newUsername\": \"" + newUsername + "\"}";

    struct curl_slist* headers = curl_slist_append(nullptr, "Content-Type: application/json");

    std::string response;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_data.c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);

    // Колбек для запису відповіді
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,
        +[](void* contents, size_t size, size_t nmemb, std::string* out) -> size_t {
            size_t totalSize = size * nmemb;
            out->append((char*)contents, totalSize);
            return totalSize;
        });
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(curl);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        std::cerr << "CURL error: " << curl_easy_strerror(res) << std::endl;
        return false;
    }

    // Перевіряємо JSON відповідь
    if (response.find("\"error\"") != std::string::npos) {
        std::cerr << "Server error: " << response << std::endl;
        return false;
    }

    return response.find("username updated") != std::string::npos;
}


bool Auth::ChangePassword(const std::string& username, const std::string& oldPassword, const std::string& newPassword) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "CURL init failed!" << std::endl;
        return false;
    }

    std::string url = globalConfig.GetBaseUrl() + "/change_password/";
    std::string json_data = "{\"username\": \"" + username + "\", \"oldPassword\": \"" + oldPassword + "\", \"newPassword\": \"" + newPassword + "\"}";

    struct curl_slist* headers = curl_slist_append(nullptr, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_data.c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);

    CURLcode res = curl_easy_perform(curl);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    return (res == CURLE_OK);
}
