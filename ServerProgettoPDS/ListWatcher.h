#pragma once
#include <list>
#include "AppInfo.h"
#include <windows.h>
#include "Errors.cpp"

#define ICONBUFF 4096
#define BUFF 4096
#define COMMSIZE 3

extern BOOL CALLBACK enumProc(HWND wnd, LPARAM param);
extern void Lsendn(SOCKET, char*, int, int);
extern void Isendn(SOCKET, char*, int, int);
extern void Fsendn(SOCKET, char*, int ,int);

class ListWatcher {
private:
	std::list<AppInfo> applist;
	//DWORD focus;
	HWND focus;
public:
	ListWatcher ();
	void init();
	std::list<AppInfo> getList();
	BOOL addApp(HWND wnd, LPARAM param);
	void clearList();
	void sendList(SOCKET);
	void sendFocus(SOCKET);
};

void pushHandle(char * buffer, HWND h);
unsigned long long htonll(unsigned long long src);
void sendIcon(SOCKET, TCHAR *, LONG);