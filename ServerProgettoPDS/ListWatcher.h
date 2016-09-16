#pragma once
#include "windows.h"
#include <list>
#include <map>
#include "AppInfo.h"
#include "Errors.cpp"

#define ICONBUFF 65536
#define BUFF 4096
#define COMMSIZE 3 //size of message type sent through the socket
#define MODMAX 3 //maximum number of modifiers present in a keyboard combination

extern BOOL CALLBACK enumProc(HWND wnd, LPARAM param);
extern BOOL CALLBACK updateProc(HWND wnd, LPARAM param);
extern void Lsendn(SOCKET, char*, int, int);
extern void Isendn(SOCKET, char*, int, int);
extern void Fsendn(SOCKET, char*, int ,int);
extern void Readn(SOCKET, char*, int , int);

class ListWatcher {
private:
	std::map<std::pair<HWND, DWORD>,AppInfo> applist;
	//DWORD focus;
	HWND focus;
	HWND newFocus;
	HWND desktopwnd;
	std::list<AppInfo> addList;
	std::list<AppInfo> removeList;
	void sendUpdate(SOCKET);
	bool newFocusGood(HWND newfocus);
public:
	ListWatcher ();
	void init();
	std::list<AppInfo> getList();
	BOOL addApp(HWND wnd, LPARAM param);
	BOOL checkApp(HWND wnd, LPARAM param);
	bool sendCommand(SOCKET);
	void updateList(SOCKET);
	void clearList();
	void sendList(SOCKET);
	void sendFocus(SOCKET);
};

void pushHandle(char * buffer, HWND h);
unsigned long long htonll(unsigned long long src);
void sendIcon(SOCKET, TCHAR *, LONG);