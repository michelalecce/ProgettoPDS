#include "stdafx.h"
#include "ListWatcher.h"


ListWatcher::ListWatcher()
{
	
	if(!EnumWindows(addApp, 0)){
		throw ListException(GetLastError(), "Enum failed");
	}
	HWND wnd = GetForegroundWindow();
	if (wnd ==NULL){
		throw ListException(GetLastError(), "Error while detecting focus owner");
	}
	GetWindowThreadProcessId(wnd, &focus);
}

BOOL CALLBACK addApp(HWND wnd, LPARAM param)
{
	try{
		applist.push_back(std::move(AppInfo(wnd)));
		return true;
	}
	catch(WindowInfoException e){
			SetLastError(e.getErr());
			return false;
	}
}
