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
    std::ifstream infile("C:/opencv/active_connections.csv");
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


// ������� ��� ���������� ��������� ��� �����

bool recvAll(SOCKET socket, char* buffer, int totalBytes) {
    int bytesReceived = 0;
    while (bytesReceived < totalBytes) {
        int result = recv(socket, buffer + bytesReceived, totalBytes - bytesReceived, 0);
        if (result <= 0) {
            return false; // ������� ��� �������� �'�������
        }
        bytesReceived += result;
    }
    return true;
}

// ������� ��� ���������� ���������� ��� �����
bool sendAll(SOCKET socket, const char* data, int totalBytes) {
    int bytesSent = 0;
    while (bytesSent < totalBytes) {
        int result = send(socket, data + bytesSent, totalBytes - bytesSent, 0);
        if (result == SOCKET_ERROR) {
            return false;
        }
        bytesSent += result;
    }
    return true;
}

void handleClient(HWND hwnd, SOCKET clientSocket) {
    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);

    logMessage(hwnd, "������� �볺���...");
    setStatusColor(hwnd, 'y');

    // ������� ������ ������� (4 ����� ��� ���������� �������)
    int netLength = 0;
    if (!recvAll(clientSocket, (char*)&netLength, sizeof(netLength))) {
        closesocket(clientSocket);
        return;
    }
    int n = ntohl(netLength); // ������������ � ���������� ������� � ���������


    if (n <= 0 || n > 1000) {
        closesocket(clientSocket);
        return;
    }

    // ������� �������
    std::ostringstream matrixStream;
    srand(time(NULL)); // ���������� ��������� ���������� �����
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            int val = rand() % 100;
            matrixStream << val << " ";
        }
        matrixStream << "\n";
    }

    std::string matrixStr = matrixStream.str();

    // ³���������� ��������� �볺���: �������� �����, ���� �������
    int messageLength = matrixStr.size();
    int netMessageLength = htonl(messageLength); // ������������ ������� � ��������� ������
    if (!sendAll(clientSocket, (char*)&netMessageLength, sizeof(netMessageLength))) {
        closesocket(clientSocket);
        return;
    }

    // ��������� ���� �������
    if (!sendAll(clientSocket, matrixStr.c_str(), messageLength)) {
        //std::cout << "������� ��� ���������� ������� �볺���!" << std::endl;
    }
    else {
        //std::cout << "������� ������ ���������� �볺���!" << std::endl;
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
    std::ofstream file("C:/opencv/active_connections.csv", std::ios::app);
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


    std::ifstream infile("C:/opencv/active_connections.csv");
    std::ofstream outfile("C:/opencv/active_connections_tmp.csv");

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
        std::remove("C:/opencv/active_connections.csv");
        std::rename("C:/opencv/active_connections_tmp.csv", "C:/opencv/active_connections.csv");
    }
    else {
        logMessage(hwnd, "�'������� �� �������� ��� ���������.");
    }
}


// ��������� �'�������: ������ �볺��� ����� �� ������
void updateConnection(HWND hwnd, int serverPort, const std::string& clientLogin, const std::string& clientIp) {
    std::ifstream infile("C:/opencv/active_connections.csv");
    std::ofstream outfile("C:/opencv/active_connections_tmp.csv");

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
    std::remove("C:/opencv/active_connections.csv");
    std::rename("C:/opencv/active_connections_tmp.csv", "C:/opencv/active_connections.csv");
}


// ����� ����� �볺��� (�������� clientLogin �� "-")
void disconnectClient(HWND hwnd, int serverPort) {
    std::ifstream infile("C:/opencv/active_connections.csv");
    std::ofstream outfile("C:/opencv/active_connections_tmp.csv");

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

    std::remove("C:/opencv/active_connections.csv");
    std::rename("C:/opencv/active_connections_tmp.csv", "C:/opencv/active_connections.csv");
}



void cleanUnusedPortsAndKeys() {
    // �������� �� ������ �����
    std::set<int> usedPorts = getUsedPorts();

    // �������� �������� used_ports.csv
    std::ifstream portsFileIn("C:/opencv/used_ports.csv");
    std::ofstream portsFileOut("C:/opencv/used_ports_tmp.csv");

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
    std::remove("C:/opencv/used_ports.csv");
    std::rename("C:/opencv/used_ports_tmp.csv", "C:/opencv/used_ports.csv");

    // ����� �������� keys.csv
    std::ifstream keysFileIn("C:/opencv/keys.csv");
    std::ofstream keysFileOut("C:/opencv/keys_tmp.csv");

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
    std::remove("C:/opencv/keys.csv");
    std::rename("C:/opencv/keys_tmp.csv", "C:/opencv/keys.csv");

    std::ifstream connectionsFileIn("C:/opencv/active_connections.csv");
    std::ofstream connectionsFileOut("C:/opencv/active_connections_tmp.csv");

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
    std::remove("C:/opencv/active_connections.csv");
    std::rename("C:/opencv/active_connections_tmp.csv", "C:/opencv/active_connections.csv");
}


