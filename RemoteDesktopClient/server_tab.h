#ifndef SERVER_TAB_H
#define SERVER_TAB_H
#include <winsock2.h>  // �������� ��������� winsock2.h
#include <ws2tcpip.h>  // ��� InetPton
#include <windows.h>
#include <thread>  // ��� ������ � ��������
#include <string>
#include "user.h"
#include <cstdio>
#include <set>

// UI ��������� ��� ����������� �������������
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


void DrawOpenConnectTab(HWND hwnd);  											// ������� �������
void DrawConnectManagingTab(HWND hwnd);  											// ������� �������
void InitOpenConnectTab(HWND hwnd); 												// ����������� DrawST � ��������� ���� 
void InitConnectManagingTab(HWND hwnd); 												// ����������� DrawST � ��������� ���� 
void FillPort(HWND hwnd);  													// ���� ������ � �������� ����
void FillKey(HWND hwnd);													// �������� ���� �����
std::string generateKey(const std::string& ip, const std::string& login, int port);	// ������ ���� ������� ����������
void downloadCSVFromDropbox(const std::string& dropboxFilePath);			// ϳ���� �������� ������ �������
void updatePortsTable(const std::string& newPort);							// ��������� �����
void updateKeysTable(const std::string& newKey);							// ��������� ������
void cleanUnusedPortsAndKeys();												// �������� ���������������� ������
std::set<int> getUsedPorts();												// ����������� �������� ���������� ��� ������ �����
																									
void addConnection(   HWND hwnd, const std::string& serverLogin, int serverPort, const std::string& serverKey, const std::string& serverIp);		// ���� ������
void removeConnection(HWND hwnd, const std::string& serverLogin, int serverPort);													// ����� ������
void updateConnection(HWND hwnd, int serverPort, const std::string& clientLogin, const std::string& clientIp);		// ������� ������ ��� ������� �볺���
void disconnectClient(HWND hwnd, int serverPort);									// ������� ������ ��� ��'������ �볺���
#endif // SERVER_TAB_H
