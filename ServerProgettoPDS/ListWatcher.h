#pragma once
#include <list>
#include "AppInfo.h"
#include <windows.h>
#include "Errors.cpp"

extern std::list<AppInfo> applist;

class ListWatcher {
private:
	DWORD focus;
public:
	ListWatcher ();
	std::list<AppInfo> getList();
friend	BOOL CALLBACK addApp(HWND wnd, LPARAM param);
};