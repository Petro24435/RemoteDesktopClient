#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <winsock2.h>
#include <ws2tcpip.h> // Для inet_pton
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#pragma comment(lib, "ws2_32.lib")

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 12345
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

int main() {
    // WinSock ініціалізація
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed\n");
        return -1;
    }

    // SDL ініціалізація
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SDL_Init failed: %s\n", SDL_GetError());
        WSACleanup();
        return -1;
    }

    // Створення вікна
    SDL_Window* window = SDL_CreateWindow("Screen Viewer", SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_RESIZABLE);
    if (!window) {
        printf("SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        WSACleanup();
        return -1;
    }

    // Створення рендерера
    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
    if (!renderer) {
        printf("SDL_CreateRenderer failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        WSACleanup();
        return -1;
    }

    // Створення TCP сокету
    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        printf("Socket creation failed\n");
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        WSACleanup();
        return -1;
    }

    // Налаштування адреси сервера
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    // Підключення до сервера
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        printf("Connection failed\n");
        closesocket(sock);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        WSACleanup();
        return -1;
    }

    // Основний цикл
    int running = 1;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) { // Правильний варіант для SDL3
                running = 0;
            }
        }

        // Отримання розміру кадру
        uint32_t frame_size_net;
        int bytes_received = recv(sock, (char*)&frame_size_net, sizeof(frame_size_net), 0);
        if (bytes_received <= 0) {
            printf("Connection closed or error\n");
            break;
        }

        uint32_t frame_size = ntohl(frame_size_net);
        uint8_t* jpeg_data = (uint8_t*)malloc(frame_size);
        if (!jpeg_data) {
            printf("Memory allocation failed\n");
            break;
        }

        size_t total_received = 0;
        while (total_received < frame_size) {
            bytes_received = recv(sock, (char*)jpeg_data + total_received, frame_size - total_received, 0);
            if (bytes_received <= 0) {
                printf("Error receiving JPEG data\n");
                free(jpeg_data);
                running = 0;
                break;
            }
            total_received += bytes_received;
        }

        if (!running) break;

        // Завантаження JPEG у SDL_Surface
        SDL_RWops* rw = SDL_RWFromMem(jpeg_data, frame_size); // використання SDL_RWFromMem

        if (!rw) {
            printf("SDL_RWFromMem failed: %s\n", SDL_GetError());
            free(jpeg_data);
            continue;
        }

        SDL_Surface* surface = IMG_Load_RW(rw, 1);
        SDL_CloseIO(rw);  // використовуємо SDL_CloseIO замість SDL_RWclose
        if (!surface) {
            printf("IMG_Load_RW failed: %s\n", IMG_GetError());
            free(jpeg_data);
            continue;
        }

        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_DestroySurface(surface);
        free(jpeg_data);

        if (!texture) {
            printf("SDL_CreateTextureFromSurface failed: %s\n", SDL_GetError());
            continue;
        }

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
        SDL_DestroyTexture(texture);
    }

    // Завершення
    closesocket(sock);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    WSACleanup();
    return 0;
}



// === SERVER (Windows) ===
// server.c
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#pragma comment(lib, "ws2_32.lib")

void send_screen(SOCKET client) {
    u_long mode = 1;
    ioctlsocket(client, FIONBIO, &mode);  // Enable non-blocking mode

    HDC hScreen = GetDC(NULL);
    HDC hDC = CreateCompatibleDC(hScreen);

    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    send(client, (char*)&screenWidth, sizeof(int), 0);
    send(client, (char*)&screenHeight, sizeof(int), 0);

    HBITMAP hBitmap = CreateCompatibleBitmap(hScreen, screenWidth, screenHeight);
    SelectObject(hDC, hBitmap);

    BITMAPINFOHEADER bi = { 0 };
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = screenWidth;
    bi.biHeight = -screenHeight; // top-down
    bi.biPlanes = 1;
    bi.biBitCount = 24;
    bi.biCompression = BI_RGB;

    char* buffer = (char*)malloc(screenWidth * screenHeight * 3);


    while (1) {
        BitBlt(hDC, 0, 0, screenWidth, screenHeight, hScreen, 0, 0, SRCCOPY);
        GetDIBits(hDC, hBitmap, 0, screenHeight, buffer, (BITMAPINFO*)&bi, DIB_RGB_COLORS);
        int size = screenWidth * screenHeight * 3;
        send(client, (char*)&size, sizeof(int), 0);
        send(client, buffer, size, 0);

        char cmd[128];
        int len = recv(client, cmd, sizeof(cmd), 0);
        if (len > 0) {
            cmd[len] = '\0';
            int x, y;
            char key[32];
            if (sscanf_s(cmd, "click %d %d", &x, &y) == 2) {
                SetCursorPos(x, y);
                mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
                mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
            }
            else if (sscanf_s(cmd, "key %s", key) == 1) {
                INPUT ip;
                ip.type = INPUT_KEYBOARD;
                ip.ki.wScan = 0;
                ip.ki.time = 0;
                ip.ki.dwExtraInfo = 0;
                ip.ki.wVk = VkKeyScan(key[0]);
                ip.ki.dwFlags = 0;
                SendInput(1, &ip, sizeof(INPUT));
                ip.ki.dwFlags = KEYEVENTF_KEYUP;
                SendInput(1, &ip, sizeof(INPUT));
            }
        }
        Sleep(30);
    }
    free(buffer);
}

int main() {
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
    SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server = { 0 };
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(9000);

    bind(s, (struct sockaddr*)&server, sizeof(server));
    listen(s, 1);
    SOCKET client = accept(s, NULL, NULL);
    send_screen(client);
    closesocket(s);
    WSACleanup();
    return 0;
}