#pragma once
#include <list>
#include <map>
#include "AppInfo.h"
#include <windows.h>
#include "Errors.cpp"

#define ICONBUFF 65536
#define BUFF 4096
#define COMMSIZE 3

extern BOOL CALLBACK enumProc(HWND wnd, LPARAM param);
extern BOOL CALLBACK updateProc(HWND wnd, LPARAM param);
extern void Lsendn(SOCKET, char*, int, int);
extern void Isendn(SOCKET, char*, int, int);
extern void Fsendn(SOCKET, char*, int ,int);

class ListWatcher {
private:
	std::map<std::pair<HWND, DWORD>,AppInfo> applist;
	//DWORD focus;
	HWND focus;
	HWND newFocus;
	std::list<AppInfo> addList;
	std::list<AppInfo> removeList;
	void sendUpdate(SOCKET);
public:
	ListWatcher ();
	void init();
	std::list<AppInfo> getList();
	BOOL addApp(HWND wnd, LPARAM param);
	BOOL checkApp(HWND wnd, LPARAM param);
	void updateList(SOCKET);
	void clearList();
	void sendList(SOCKET);
	void sendFocus(SOCKET);
};

void pushHandle(char * buffer, HWND h);
unsigned long long htonll(unsigned long long src);
void sendIcon(SOCKET, TCHAR *, LONG);