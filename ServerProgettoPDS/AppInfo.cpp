#include "stdafx.h"
#include "AppInfo.h"
#include "Errors.cpp"
#include "Psapi.h"
#include "Shellapi.h"

AppInfo::AppInfo(HWND wnd):wnd(wnd){
	HANDLE hproc;
	GetWindowThreadProcessId(wnd,&pid);
	if((hproc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid))==NULL){
		throw WindowInfoException(GetLastError(), "Error while opening the handle to a process");
	}
	namesize = GetModuleFileNameEx(hproc, NULL, name, MAXPATHSIZE - 1);
	if(namesize==0){
		throw WindowInfoException(GetLastError(), "Problems with process name");
	}
	else{
		name[namesize] = (TCHAR)'\0';
	}
	//ExtractIconEx(name, 0, )
}

AppInfo::~AppInfo(){}
