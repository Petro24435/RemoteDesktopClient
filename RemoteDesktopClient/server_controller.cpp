#include <fstream>
#include <sstream>
#include <cstdio>  // ��� std::remove � std::rename
#include <string>
#include "server_tab.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#pragma comment(lib, "ws2_32.lib")  // ˳����� �������� Winsock

SOCKET serverSocket;
bool serverRunning = false;  // ���� ��� ��������, �� ������ ��� ���������

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

    // ��������, �� ���� ��������
    if (!infile.is_open()) {
        std::cerr << "�� ������� ������� ����!" << std::endl;
        return "das";
    }

    // ���������� ���������
    std::getline(infile, line);

    // ������� ������� �����
    while (std::getline(infile, line)) {
        std::stringstream ss(line);
        std::string loginServer, portStr, key, serverIp, loginClientFromFile, clientIp;

        // ��������� ����� �� �������, �������������� ���� �� ���������
        std::getline(ss, loginServer, ',');
        std::getline(ss, portStr, ',');
        std::getline(ss, key, ',');
        std::getline(ss, serverIp, ',');
        std::getline(ss, loginClientFromFile, ',');
        std::getline(ss, clientIp, ',');

        // ������������ ���� ����� �� �����
        std::string targetPortStr = std::to_string(targetPort);

        // ����������, �� ����� � ����� � ���� ������� � targetPort
        if (portStr == targetPortStr) {
            loginClient = loginClientFromFile; // �������� loginClient, ���� ���� �������
            break;  // ����� ����� � �����, ������� �� ������� ��������� �����
        }
    }

    infile.close();
    return loginClient;  // ��������� loginClient ��� ������� �����, ���� ����� �� ��������
}



void handleClient(HWND hwnd, SOCKET clientSocket) {
    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);

    logMessage(hwnd, "������� �볺���...");
    logMessage(hwnd, "����� �볺�� ����������!");
    setStatusColor(hwnd, 'y');

    // �������� loginClient � ����� �� ������
    std::string loginClient = getLoginClientForPort(port);

    // �������� loginClient � ����
    SetWindowText(GetDlgItem(hwnd, 4006), std::wstring(loginClient.begin(), loginClient.end()).c_str());
    MessageBox(hwnd, std::wstring(loginClient.begin(), loginClient.end()).c_str(), L"����",MB_OK);

    int n;
    int bytesReceived = recv(clientSocket, (char*)&n, sizeof(n), 0);
    if (bytesReceived <= 0) {
        std::cout << "�� ������� �������� ����� �� �볺���!" << std::endl;
        closesocket(clientSocket);
        return;
    }

    n = ntohl(n); // <-- ����'������!!

    std::cout << "�������� �����: " << n << std::endl;

    int result = n * n;
    std::cout << "��������� ���������� n^2: " << result << std::endl;

    int netResult = htonl(result); // <-- � ��� ����� � ��������� �������
    int bytesSent = send(clientSocket, (char*)&netResult, sizeof(netResult), 0);
    if (bytesSent == SOCKET_ERROR) {
        std::cout << "������� ��� ���������� ���������� �볺���!" << std::endl;
    }
    else {
        std::cout << "��������� ������ ����������!" << std::endl;
    }

    closesocket(clientSocket);
}



// ������� ��� ����������� �������
bool initializeServer(HWND hwnd, const std::string& serverIp, int serverPort) {
    WSADATA wsaData;
    int wsaResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (wsaResult != 0) {
        logMessage(hwnd, "WSAStartup �� ������� ������������!");
        setStatusColor(hwnd, 'r'); // ��������
        return false;
    }

    serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        logMessage(hwnd, "�� ������� �������� �����!");
        setStatusColor(hwnd, 'r'); // ��������
        WSACleanup();
        return false;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(serverPort);
    int result = inet_pton(AF_INET, serverIp.c_str(), &serverAddr.sin_addr);
    if (result <= 0) {
        logMessage(hwnd, "������ IP-������!");
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
        logMessage(hwnd, "�� ������� ����'����� ����� �� IP � �����!");
        closesocket(serverSocket);
        WSACleanup();
        return false;
    }

    int listenResult = listen(serverSocket, SOMAXCONN);
    if (listenResult == SOCKET_ERROR) {
        logMessage(hwnd, "�� ������� ������ ������� ����!");
        closesocket(serverSocket);
        WSACleanup();
        return false;
    }

    return true;
}

// ������� ��� ������� �������
void serverThreadFunction(HWND hwnd, std::string serverLogin, int serverPort, std::string serverKey, std::string serverIp) {
    if (serverRunning) {
        logMessage(hwnd, "������ ��� ���������!");
        return;  // ���� ������ ��� ������, �� ��������� �����
    }

    // ����������� �������
    if (!initializeServer(hwnd, serverIp, serverPort)) {
        return; // ���� ����������� �� �������, ���������� ������
    }

    logMessage(hwnd, "������ ������ �� IP " + serverIp + " � ����� " + std::to_string(serverPort));
    setStatusColor(hwnd, 'g'); // �������
    serverRunning = true;

    SOCKET clientSocket;
    sockaddr_in clientAddr;
    int clientAddrSize = sizeof(clientAddr);

    while (serverRunning) {
        clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrSize);
        if (clientSocket == INVALID_SOCKET) {
            continue;
        }

        // ������� �볺��� � �������� ������
        std::thread clientThread(handleClient, hwnd, clientSocket);
        clientThread.detach();
    }

    // ���������� ������ �������
    closesocket(serverSocket);
    WSACleanup();
    serverRunning = false;
}

// ��������� �'������� �� ������ �������� �'������
void addConnection(HWND hwnd, const std::string& serverLogin, int serverPort, const std::string& serverKey, const std::string& serverIp) {
    // �������� �� �������� ������� � ���������� (����, IP, ����)
    if (serverIp.empty() || /*serverLogin.empty() ||*/ serverKey.empty()) {
        logMessage(hwnd, "��������� ��������� ��� �'�������!");
        return;
    }



    // ��������� �'������� � ����
    std::ofstream file("active_connections.csv", std::ios::app);
    if (file.is_open()) {
        file << serverLogin << "," << serverPort << "," << serverKey << "," << serverIp << ",-" << ",-" << std::endl;
        file.close();
    }

    // ���� ������ �� �� ���������, ��������� ����
    if (!serverRunning) {
        std::thread(serverThreadFunction, hwnd, serverLogin, serverPort, serverKey, serverIp).detach();
    }
}

// ��������� �'�������
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
            // ���� � �������� �볺��, ���� ���'������
            if (client != "-") {
                logMessage(hwnd, "�볺�� � IP " + clientIp + " ���������� �� �������.");
                // ������� ����� �볺���, ���� ���������
            }

            // ���������� ��� �����, ������� �'������� �� ���� ��������
            continue;
        }

        outfile << line << std::endl;
    }

    infile.close();
    outfile.close();

    // ���� ���� �������� �'�������, �������� ����
    if (connectionFound) {
        std::remove("active_connections.csv");
        std::rename("active_connections_tmp.csv", "active_connections.csv");
    }
    else {
        logMessage(hwnd, "�'������� �� �������� ��� ���������.");
    }
}


// ��������� �'�������: ������ �볺��� ����� �� ������
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
            // ���� ������� ����� ��� ����� �����, ��������� ����
            outfile << login << "," << portStr << "," << key << "," << serverIpInFile << ","
                << clientLogin << "," << clientIp << std::endl;
            found = true;
        }
        else {
            // ���� ����� �� ��� ����� �����, �������� ���� ��� ���
            outfile << line << std::endl;
        }
    }

    if (!found) {
        // ���� �� ������� ������ ��� �����, ������ �����
        outfile << clientLogin << "," << serverPort << ",-, " << "0.0.0.0" << ","
            << clientLogin << "," << clientIp << std::endl;
    }

    infile.close();
    outfile.close();

    // �������� ������ ���� �����
    std::remove("active_connections.csv");
    std::rename("active_connections_tmp.csv", "active_connections.csv");
}


// ����� ����� �볺��� (�������� clientLogin �� "-")
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
    // �������� �� ������ �����
    std::set<int> usedPorts = getUsedPorts();

    // �������� �������� used_ports.csv
    std::ifstream portsFileIn("used_ports.csv");
    std::ofstream portsFileOut("used_ports_tmp.csv");

    std::string line;
    bool firstLine = true;
    if (portsFileIn && portsFileOut) {
        while (std::getline(portsFileIn, line)) {
            if (firstLine) {
                portsFileOut << line << "\n"; // ������� ���������
                firstLine = false;
                continue;
            }
            if (line.empty()) continue;
            try {
                int port = std::stoi(line);
                if (usedPorts.find(port) != usedPorts.end()) {
                    portsFileOut << port << "\n"; // ���� ���� ��� �� �������� � ��������
                }
            }
            catch (...) {}
        }
    }
    portsFileIn.close();
    portsFileOut.close();
    std::remove("used_ports.csv");
    std::rename("used_ports_tmp.csv", "used_ports.csv");

    // ����� �������� keys.csv
    std::ifstream keysFileIn("keys.csv");
    std::ofstream keysFileOut("keys_tmp.csv");

    firstLine = false;
    if (keysFileIn && keysFileOut) {
        while (std::getline(keysFileIn, line)) {
            if (firstLine) {
                keysFileOut << line << "\n"; // ������� ���������
                firstLine = false;
                continue;
            }
            if (line.empty()) continue;

            // ��������� ����� (����,����)
            std::istringstream iss(line);
            std::string key, portStr;
            if (std::getline(iss, key, ',') && std::getline(iss, portStr)) {
                try {
                    int port = std::stoi(portStr);
                    if (usedPorts.find(port) != usedPorts.end()) {
                        keysFileOut << key << "," << port << "\n"; // ���� ���� �������� � ��������
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
                connectionsFileOut << line << "\n"; // ������� ���������
                firstLine = false;
                continue;
            }
            if (line.empty()) continue;

            // ��������� ����� (login,port,key)
            std::istringstream iss(line);
            std::string login, portStr, key;
            if (std::getline(iss, login, ',') && std::getline(iss, portStr, ',') && std::getline(iss, key)) {
                try {
                    int port = std::stoi(portStr);
                    if (usedPorts.find(port) != usedPorts.end()) {
                        connectionsFileOut << login << "," << port << "," << key << "\n"; // ���� ���� �������� � ��������
                    }
                }
                catch (...) {
                    // �������� ����� � ������������ ������
                }
            }
        }
    }

    connectionsFileIn.close();
    connectionsFileOut.close();
    std::remove("active_connections.csv");
    std::rename("active_connections_tmp.csv", "active_connections.csv");
}


