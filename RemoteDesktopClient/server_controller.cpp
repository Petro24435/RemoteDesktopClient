#include <fstream>
#include <sstream>
#include <cstdio>  // для std::remove і std::rename
#include <string>
#include "server_tab.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#pragma comment(lib, "ws2_32.lib")  // Лінкуємо бібліотеку Winsock

SOCKET serverSocket;
bool serverRunning = false;  // Флаг для перевірки, чи сервер вже запущений

void logMessage(HWND hwnd, const std::string& message) {
    std::ofstream logFile("server_log.txt", std::ios::app);
    if (logFile.is_open()) {
        logFile << message << std::endl;
        logFile.close();
    }
    if (hwnd != NULL) {
        HWND hStatusEdit = GetDlgItem(hwnd, 4009);
        if (hStatusEdit) {
            int size_needed = MultiByteToWideChar(CP_ACP, 0, message.c_str(), -1, NULL, 0);
            std::wstring wMessage(size_needed, 0);
            MultiByteToWideChar(CP_ACP, 0, message.c_str(), -1, &wMessage[0], size_needed);
            SetWindowTextW(hStatusEdit, wMessage.c_str());
        }
    }
}


std::string getLoginClientForPort(int targetPort) {
    std::ifstream infile("active_connections.csv");
    std::string line;
    std::string loginClient;

    // Перевірка, чи файл відкритий
    if (!infile.is_open()) {
        std::cerr << "Не вдалося відкрити файл!" << std::endl;
        return "das";
    }

    // Пропускаємо заголовок
    std::getline(infile, line);

    // Читання кожного рядка
    while (std::getline(infile, line)) {
        std::stringstream ss(line);
        std::string loginServer, portStr, key, serverIp, loginClientFromFile, clientIp;

        // Розбиваємо рядок на частини, використовуючи кому як роздільник
        std::getline(ss, loginServer, ',');
        std::getline(ss, portStr, ',');
        std::getline(ss, key, ',');
        std::getline(ss, serverIp, ',');
        std::getline(ss, loginClientFromFile, ',');
        std::getline(ss, clientIp, ',');

        // Перетворюємо ціле число на рядок
        std::string targetPortStr = std::to_string(targetPort);

        // Перевіряємо, чи рядок з порту у файлі співпадає з targetPort
        if (portStr == targetPortStr) {
            loginClient = loginClientFromFile; // Зберігаємо loginClient, якщо порт співпадає
            break;  // Можна вийти з циклу, оскільки ми знайшли відповідний запис
        }
    }

    infile.close();
    return loginClient;  // Повертаємо loginClient або порожній рядок, якщо нічого не знайдено
}



void handleClient(HWND hwnd, SOCKET clientSocket) {
    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);

    logMessage(hwnd, "Обробка клієнта...");
    logMessage(hwnd, "Новий клієнт підключений!");
    setStatusColor(hwnd, 'y');

    // Отримуємо loginClient з файлу за портом
    std::string loginClient = getLoginClientForPort(port);

    // Виводимо loginClient у вікно
    SetWindowText(GetDlgItem(hwnd, 4006), std::wstring(loginClient.begin(), loginClient.end()).c_str());
    MessageBox(hwnd, std::wstring(loginClient.begin(), loginClient.end()).c_str(), L"Успіх",MB_OK);

    int n;
    int bytesReceived = recv(clientSocket, (char*)&n, sizeof(n), 0);
    if (bytesReceived <= 0) {
        std::cout << "Не вдалося прийняти число від клієнта!" << std::endl;
        closesocket(clientSocket);
        return;
    }

    n = ntohl(n); // <-- ОБОВ'ЯЗКОВО!!

    std::cout << "Отримано число: " << n << std::endl;

    int result = n * n;
    std::cout << "Результат обчислення n^2: " << result << std::endl;

    int netResult = htonl(result); // <-- І ТУТ назад у мережевий порядок
    int bytesSent = send(clientSocket, (char*)&netResult, sizeof(netResult), 0);
    if (bytesSent == SOCKET_ERROR) {
        std::cout << "Помилка при відправленні результату клієнту!" << std::endl;
    }
    else {
        std::cout << "Результат успішно відправлено!" << std::endl;
    }

    closesocket(clientSocket);
}



// Функція для ініціалізації сервера
bool initializeServer(HWND hwnd, const std::string& serverIp, int serverPort) {
    WSADATA wsaData;
    int wsaResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (wsaResult != 0) {
        logMessage(hwnd, "WSAStartup не вдалося ініціалізувати!");
        setStatusColor(hwnd, 'r'); // Червоний
        return false;
    }

    serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        logMessage(hwnd, "Не вдалося створити сокет!");
        setStatusColor(hwnd, 'r'); // Червоний
        WSACleanup();
        return false;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(serverPort);
    int result = inet_pton(AF_INET, serverIp.c_str(), &serverAddr.sin_addr);
    if (result <= 0) {
        logMessage(hwnd, "Невірна IP-адреса!");
        setStatusColor(hwnd, '0');
        closesocket(serverSocket);
        WSACleanup();
        return false;
    }
    int optVal = 1;
    int optLen = sizeof(optVal);
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&optVal, optLen);
    int bindResult = bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    if (bindResult == SOCKET_ERROR) {
        logMessage(hwnd, "Не вдалося прив'язати сокет до IP і порту!");
        closesocket(serverSocket);
        WSACleanup();
        return false;
    }

    int listenResult = listen(serverSocket, SOMAXCONN);
    if (listenResult == SOCKET_ERROR) {
        logMessage(hwnd, "Не вдалося почати слухати порт!");
        closesocket(serverSocket);
        WSACleanup();
        return false;
    }

    return true;
}

// Функція для запуску сервера
void serverThreadFunction(HWND hwnd, std::string serverLogin, int serverPort, std::string serverKey, std::string serverIp) {
    if (serverRunning) {
        logMessage(hwnd, "Сервер вже запущений!");
        return;  // Якщо сервер вже працює, не запускати знову
    }

    // Ініціалізація сервера
    if (!initializeServer(hwnd, serverIp, serverPort)) {
        return; // Якщо ініціалізація не вдалася, припиняємо запуск
    }

    logMessage(hwnd, "Сервер працює на IP " + serverIp + " і порту " + std::to_string(serverPort));
    setStatusColor(hwnd, 'g'); // Зелений
    serverRunning = true;

    SOCKET clientSocket;
    sockaddr_in clientAddr;
    int clientAddrSize = sizeof(clientAddr);

    while (serverRunning) {
        clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrSize);
        if (clientSocket == INVALID_SOCKET) {
            continue;
        }

        // Обробка клієнта в окремому потоці
        std::thread clientThread(handleClient, hwnd, clientSocket);
        clientThread.detach();
    }

    // Завершення роботи сервера
    closesocket(serverSocket);
    WSACleanup();
    serverRunning = false;
}

// Додавання з'єднання до списку активних з'єднань
void addConnection(HWND hwnd, const std::string& serverLogin, int serverPort, const std::string& serverKey, const std::string& serverIp) {
    // Перевірка на наявність помилок в параметрах (ключ, IP, порт)
    if (serverIp.empty() || /*serverLogin.empty() ||*/ serverKey.empty()) {
        logMessage(hwnd, "Некоректні параметри для з'єднання!");
        return;
    }



    // Додавання з'єднання в файл
    std::ofstream file("active_connections.csv", std::ios::app);
    if (file.is_open()) {
        file << serverLogin << "," << serverPort << "," << serverKey << "," << serverIp << ",-" << ",-" << std::endl;
        file.close();
    }

    // Якщо сервер ще не запущений, запускаємо його
    if (!serverRunning) {
        std::thread(serverThreadFunction, hwnd, serverLogin, serverPort, serverKey, serverIp).detach();
    }
}

// Видалення з'єднання
void removeConnection(HWND hwnd, const std::string& serverLogin, int serverPort) {


    std::ifstream infile("active_connections.csv");
    std::ofstream outfile("active_connections_tmp.csv");

    std::string line;
    bool connectionFound = false;

    while (std::getline(infile, line)) {
        std::istringstream iss(line);
        std::string login, portStr, key, serverIp, client, clientIp;
        std::getline(iss, login, ',');
        std::getline(iss, portStr, ',');
        std::getline(iss, key, ',');
        std::getline(iss, serverIp, ',');
        std::getline(iss, client, ',');
        std::getline(iss, clientIp, ',');

        if (login == serverLogin && std::stoi(portStr) == serverPort) {
            connectionFound = true;
            serverRunning = false;
            // Якщо є активний клієнт, його роз'єднуємо
            if (client != "-") {
                logMessage(hwnd, "Клієнт з IP " + clientIp + " відключений від сервера.");
                // Закрити сокет клієнта, якщо необхідно
            }

            // Пропускаємо цей рядок, оскільки з'єднання має бути видалене
            continue;
        }

        outfile << line << std::endl;
    }

    infile.close();
    outfile.close();

    // Якщо було знайдено з'єднання, замінюємо файл
    if (connectionFound) {
        std::remove("active_connections.csv");
        std::rename("active_connections_tmp.csv", "active_connections.csv");
    }
    else {
        logMessage(hwnd, "З'єднання не знайдено для видалення.");
    }
}


// Оновлюємо з'єднання: додаємо клієнта тільки за портом
void updateConnection(HWND hwnd, int serverPort, const std::string& clientLogin, const std::string& clientIp) {
    std::ifstream infile("active_connections.csv");
    std::ofstream outfile("active_connections_tmp.csv");

    std::string line;
    bool found = false;
    std::string serverLogin, serverIp;

    while (std::getline(infile, line)) {
        std::istringstream iss(line);
        std::string login, portStr, key, serverIpInFile, client, clientIpOld;
        std::getline(iss, login, ',');
        std::getline(iss, portStr, ',');
        std::getline(iss, key, ',');
        std::getline(iss, serverIpInFile, ',');
        std::getline(iss, client, ',');
        std::getline(iss, clientIpOld, ',');

        if (std::stoi(portStr) == serverPort) {
            // Якщо знайшли запис для цього порту, оновлюємо його
            outfile << login << "," << portStr << "," << key << "," << serverIpInFile << ","
                << clientLogin << "," << clientIp << std::endl;
            found = true;
        }
        else {
            // Якщо запис не для цього порту, зберігаємо його без змін
            outfile << line << std::endl;
        }
    }

    if (!found) {
        // Якщо не знайшли запису для порту, додаємо новий
        outfile << clientLogin << "," << serverPort << ",-, " << "0.0.0.0" << ","
            << clientLogin << "," << clientIp << std::endl;
    }

    infile.close();
    outfile.close();

    // Замінюємо старий файл новим
    std::remove("active_connections.csv");
    std::rename("active_connections_tmp.csv", "active_connections.csv");
}


// Вихід тільки клієнта (обнулити clientLogin на "-")
void disconnectClient(HWND hwnd, int serverPort) {
    std::ifstream infile("active_connections.csv");
    std::ofstream outfile("active_connections_tmp.csv");

    std::string line;
    while (std::getline(infile, line)) {
        std::istringstream iss(line);
        std::string login, portStr, key, serverIp, client, clientIp;
        std::getline(iss, login, ',');
        std::getline(iss, portStr, ',');
        std::getline(iss, key, ',');
        std::getline(iss, serverIp, ',');
        std::getline(iss, client, ',');
        std::getline(iss, clientIp, ',');

        if (std::stoi(portStr) == serverPort) {
            outfile << login << "," << portStr << "," << key << "," << serverIp << ",-" << ",-" << std::endl;
        }
        else {
            outfile << line << std::endl;
        }
    }

    infile.close();
    outfile.close();

    std::remove("active_connections.csv");
    std::rename("active_connections_tmp.csv", "active_connections.csv");
}



void cleanUnusedPortsAndKeys() {
    // Отримати всі зайняті порти
    std::set<int> usedPorts = getUsedPorts();

    // Спочатку очистимо used_ports.csv
    std::ifstream portsFileIn("used_ports.csv");
    std::ofstream portsFileOut("used_ports_tmp.csv");

    std::string line;
    bool firstLine = true;
    if (portsFileIn && portsFileOut) {
        while (std::getline(portsFileIn, line)) {
            if (firstLine) {
                portsFileOut << line << "\n"; // Копіюємо заголовок
                firstLine = false;
                continue;
            }
            if (line.empty()) continue;
            try {
                int port = std::stoi(line);
                if (usedPorts.find(port) != usedPorts.end()) {
                    portsFileOut << port << "\n"; // Якщо порт все ще зайнятий — зберігаємо
                }
            }
            catch (...) {}
        }
    }
    portsFileIn.close();
    portsFileOut.close();
    std::remove("used_ports.csv");
    std::rename("used_ports_tmp.csv", "used_ports.csv");

    // Тепер очистимо keys.csv
    std::ifstream keysFileIn("keys.csv");
    std::ofstream keysFileOut("keys_tmp.csv");

    firstLine = false;
    if (keysFileIn && keysFileOut) {
        while (std::getline(keysFileIn, line)) {
            if (firstLine) {
                keysFileOut << line << "\n"; // Копіюємо заголовок
                firstLine = false;
                continue;
            }
            if (line.empty()) continue;

            // Розбираємо рядок (ключ,порт)
            std::istringstream iss(line);
            std::string key, portStr;
            if (std::getline(iss, key, ',') && std::getline(iss, portStr)) {
                try {
                    int port = std::stoi(portStr);
                    if (usedPorts.find(port) != usedPorts.end()) {
                        keysFileOut << key << "," << port << "\n"; // Якщо порт зайнятий — зберігаємо
                    }
                }
                catch (...) {}
            }
        }
    }
    keysFileIn.close();
    keysFileOut.close();
    std::remove("keys.csv");
    std::rename("keys_tmp.csv", "keys.csv");

    std::ifstream connectionsFileIn("active_connections.csv");
    std::ofstream connectionsFileOut("active_connections_tmp.csv");

    firstLine = false;

    if (connectionsFileIn && connectionsFileOut) {
        while (std::getline(connectionsFileIn, line)) {
            if (firstLine) {
                connectionsFileOut << line << "\n"; // Копіюємо заголовок
                firstLine = false;
                continue;
            }
            if (line.empty()) continue;

            // Розбираємо рядок (login,port,key)
            std::istringstream iss(line);
            std::string login, portStr, key;
            if (std::getline(iss, login, ',') && std::getline(iss, portStr, ',') && std::getline(iss, key)) {
                try {
                    int port = std::stoi(portStr);
                    if (usedPorts.find(port) != usedPorts.end()) {
                        connectionsFileOut << login << "," << port << "," << key << "\n"; // Якщо порт зайнятий — зберігаємо
                    }
                }
                catch (...) {
                    // Ігноруємо рядки з некоректними даними
                }
            }
        }
    }

    connectionsFileIn.close();
    connectionsFileOut.close();
    std::remove("active_connections.csv");
    std::rename("active_connections_tmp.csv", "active_connections.csv");
}


