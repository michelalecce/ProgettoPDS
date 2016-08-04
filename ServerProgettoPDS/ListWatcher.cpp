#include "ListWatcher.h"

ListWatcher::ListWatcher()
{
	if(!EnumWindows(addApp, 0)){
		//eccezioni
	}
	focus = GetForegroundWindow();
	if (focus ==NULL){
		//eccezioni
	}
}

std::list<AppInfo> ListWatcher::getList()
{
	return list;
}

BOOL CALLBACK addApp(HWND wnd, LPARAM param)
{
	ListWatcher::list.push_back(std::move(AppInfo(wnd)));
	return true;
}
