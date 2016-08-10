#pragma once

#include <windows.h>
#include <string>

#define MAXPATHSIZE 100

class AppInfo{
private:
	TCHAR name[MAXPATHSIZE];
	DWORD namesize;
	HWND wnd;
	DWORD pid;
	DWORD iconsize;
public:
	AppInfo();
	AppInfo(HWND wnd);
	DWORD getPid();
	DWORD getNameSize();
	TCHAR* getName();
	~AppInfo();
};

