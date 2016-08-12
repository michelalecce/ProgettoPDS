#pragma once

#include <windows.h>
#include <string>

#define ERRBUFF 80

class AppInfo{
private:
	TCHAR name[MAX_PATH];
	DWORD namesize;
	HWND wnd;
	DWORD pid;
	LONG iconFileSize;
	TCHAR iconFile[MAX_PATH];
public:
	AppInfo();
	AppInfo(HWND wnd);
	DWORD getPid();
	DWORD getNameSize();
	TCHAR* getName();
	void retrieveIcon(); 
	void cleanIcon();
	~AppInfo();
};

