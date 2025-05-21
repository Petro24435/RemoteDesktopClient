#ifndef SERVER_TAB_H
#define SERVER_TAB_H
#include <winsock2.h>  // Спочатку підключаємо winsock2.h
#include <ws2tcpip.h>  // Для InetPton
#include <windows.h>
#include <thread>  // Для роботи з потоками
#include <string>
#include "user.h"
#include <cstdio>
#include <set>

// UI параметри для адаптивного масштабування
#define UI_BASE_WIDTH 600
#define UI_BASE_HEIGHT 320 
#define UI_X_MARGIN 20
#define UI_Y_LINE(n) (20 + 40 * (n))
#define UI_HEIGHT_LINE 25
#define UI_WIDTH_LABEL 100
#define UI_WIDTH_EDIT 200
#define UI_WIDTH_BUTTON 150

extern HWND hLabelIp, hIpEdit, hStartBtn, hCloseBtn, hDisconnectBtn, hStatusIcon;
extern HWND hGroupBoxAccess, hMouseAccess, hKeyboardAccess;
extern HWND hLabelPort, hPortEdit, hLabelKey, hKeyEdit;
extern HWND hLabelClient, hClientEdit, hLogEdit;
extern bool mouseAccess, keyboardAccess;
extern int port;
static WCHAR bufferPort[256];
static WCHAR bufferKey[256];
extern UserInfo currentUser;
static COLORREF currentStatusColor;
void setStatusColor(HWND hwnd, char c);


void DrawOpenConnectTab(HWND hwnd);  											// Створює вкладку
void DrawConnectManagingTab(HWND hwnd);  											// Створює вкладку
void InitOpenConnectTab(HWND hwnd); 												// Ініціалізація DrawST і обробника подій 
void InitConnectManagingTab(HWND hwnd); 												// Ініціалізація DrawST і обробника подій 
void FillPort(HWND hwnd);  													// Шукає вільний і заповнює порт
void FillKey(HWND hwnd);													// Заповнює поле ключа
std::string generateKey(const std::string& ip, const std::string& login, int port);	// Генерує ключ методом шифрування
void downloadCSVFromDropbox(const std::string& dropboxFilePath);			// Пізніше реалізуємо хмарне сховище
void updatePortsTable(const std::string& newPort);							// Оновлення портів
void updateKeysTable(const std::string& newKey);							// Оновлення ключів
void cleanUnusedPortsAndKeys();												// Очищення використовуваних ключів
std::set<int> getUsedPorts();												// Отримування системної інформації про поточні порти
																									
void addConnection(   HWND hwnd, const std::string& serverLogin, int serverPort, const std::string& serverKey, const std::string& serverIp);		// Додає Сервер
void removeConnection(HWND hwnd, const std::string& serverLogin, int serverPort);													// Знищує Сервер
void updateConnection(HWND hwnd, int serverPort, const std::string& clientLogin, const std::string& clientIp);		// Оновлює Сервер при доєднанні клієнта
void disconnectClient(HWND hwnd, int serverPort);									// Оновлює Сервер при від'єднанні клієнта
#endif // SERVER_TAB_H
