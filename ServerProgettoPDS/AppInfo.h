#pragma once
#include "windows.h"
#include <string>

#define ERRBUFF 80
extern bool SaveIcon(HICON hIcon,DWORD& szSize, int nColorBits, const TCHAR* szPath);
class AppInfo{
private:
	TCHAR name[MAX_PATH];
	char nameA[MAX_PATH];
	DWORD namesize;
	HWND wnd;
	DWORD pid;
	DWORD iconFileSize;
	TCHAR iconFile[MAX_PATH];
	bool still_open; //while updating the list it is true if it's found open again with a successive call of enumWindows
public:
	AppInfo();
	AppInfo(HWND wnd);
	DWORD getPid();
	DWORD getNameSize();
	TCHAR* getName();
	char* getNameA();
	HWND getWindow();
	void retrieveIcon(); 
	void deleteIcon();
	TCHAR *getIconFile();
	LONG getIconFileSize();
	void setStillOpen();
	void clearStillOpen();
	bool getStillOpen();
	~AppInfo();
};

HICON GetHighResolutionIcon(LPTSTR );

