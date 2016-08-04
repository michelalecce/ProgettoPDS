#pragma once
#include <list>
#include "AppInfo.h"
#include <windows.h>

class ListWatcher {
private:
	static std::list<AppInfo> list;
	HWND focus;
public:
	ListWatcher ();
	std::list<AppInfo> getList();
friend	BOOL CALLBACK addApp(HWND wnd, LPARAM param);
};