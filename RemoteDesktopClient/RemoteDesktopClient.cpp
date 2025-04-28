//#include <iostream>
//#include <winsock2.h>
//#include <opencv2/opencv.hpp>
//#include <ws2tcpip.h>
//#include <thread>
//#include <cmath>
//
//#pragma comment(lib, "ws2_32.lib")
//#pragma comment(lib, "User32.lib")
//#include <windows.h>
//
//#define SCALE 2
//#define WIDTH (1920 / SCALE)
//#define HEIGHT (1080 / SCALE)
//#define PORT 12345
//#define RECONNECT_DELAY 5000
//
//SOCKET clientSocket;
//sockaddr_in serverAddr;
//std::atomic<bool> isRunning(true);
//
//void ConnectToServer() {
//    while (true) {
//        clientSocket = socket(AF_INET, SOCK_STREAM, 0);
//        if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == 0) {
//            std::cout << "Підключено до сервера!" << std::endl;
//            break;
//        }
//
//        std::cerr << "Сервер недоступний, спроба підключення через "
//            << RECONNECT_DELAY / 1000 << " сек..." << std::endl;
//        closesocket(clientSocket);
//        std::this_thread::sleep_for(std::chrono::milliseconds(RECONNECT_DELAY));
//    }
//}
//
//void MouseCallback(int event, int x, int y, int flags, void* userdata) {
//    double multiplier = sqrt(SCALE);
//    int finalX = static_cast<int>(round(x * multiplier));
//    int finalY = static_cast<int>(round(y * multiplier));
//
//    if (event == cv::EVENT_LBUTTONDOWN) {
//        std::cout << "Клік у точці (" << finalX << ", " << finalY << ")" << std::endl;
//        int data[3] = { finalX, finalY, 1 }; // 1 - клік
//        send(clientSocket, (char*)data, sizeof(data), 0);
//    }
//}
//
//// Потік для відправки координат миші
//void SendMousePosition() {
//    POINT lastPos = { 0, 0 };
//
//    while (isRunning) {
//        POINT pos;
//        GetCursorPos(&pos);
//        std::this_thread::sleep_for(std::chrono::milliseconds(500));
//        // Перевіряємо, чи змінились координати
//        if (pos.x != lastPos.x || pos.y != lastPos.y) {
//            int data[3] = { pos.x, pos.y, 0 }; // 0 - просто переміщення
//            send(clientSocket, (char*)data, sizeof(data), 0);
//            lastPos = pos;
//        }
//
//        std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Невелика затримка для плавності
//    }
//}
//
//int main() {
//    SetConsoleCP(1251);
//    SetConsoleOutputCP(1251);
//    WSADATA wsa;
//    char* buffer = new char[WIDTH * HEIGHT * 3];
//
//    WSAStartup(MAKEWORD(2, 2), &wsa);
//
//    std::string serverIP;
//    std::cout << "Введіть IP сервера \n1 для 127.0.0.1\n2 для 192.168.0.128): ";
//    std::cin >> serverIP;
//    if (serverIP == "1") serverIP = "127.0.0.1";
//    else if (serverIP == "2") serverIP = "192.168.0.128";
//
//    serverAddr.sin_family = AF_INET;
//    serverAddr.sin_port = htons(PORT);
//    inet_pton(AF_INET, serverIP.c_str(), &serverAddr.sin_addr);
//
//    ConnectToServer();
//
//    cv::namedWindow("Remote Desktop");
//    cv::setMouseCallback("Remote Desktop", MouseCallback);
//
//    std::thread mouseThread(SendMousePosition);
//
//    while (true) {
//        int bytesReceived = recv(clientSocket, buffer, WIDTH * HEIGHT * 3, MSG_WAITALL);
//        if (bytesReceived <= 0) {
//            std::cerr << "Втрата з'єднання! Спроба підключення..." << std::endl;
//            closesocket(clientSocket);
//            ConnectToServer();
//            continue;
//        }
//
//        cv::Mat frame(HEIGHT, WIDTH, CV_8UC3, buffer);
//        cv::cvtColor(frame, frame, cv::COLOR_RGB2BGR);
//        cv::imshow("Remote Desktop", frame);
//
//        if (cv::waitKey(30) == 27) {
//            isRunning = false;
//            break;
//        }
//    }
//
//    closesocket(clientSocket);
//    WSACleanup();
//    delete[] buffer;
//    return 0;
//}
