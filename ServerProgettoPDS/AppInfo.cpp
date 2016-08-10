#include "stdafx.h"
#include "AppInfo.h"
#include "Errors.cpp"
#include "Psapi.h"
#include "Shellapi.h"

AppInfo::AppInfo(){
	wnd=NULL;
	pid=0;
	namesize=0;
}

AppInfo::AppInfo(HWND wnd):wnd(wnd){
	HANDLE hproc;
	GetWindowThreadProcessId(wnd,&pid);
	if((hproc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid))==NULL){
		DWORD err = GetLastError();
		throw WindowInfoException(err, "Error while opening the handle to a process");
	}
	namesize = GetModuleFileNameEx(hproc, NULL, name, MAXPATHSIZE - 1);
	if(namesize==0){
		throw WindowInfoException(GetLastError(), "Problems with process name");
	}
	else{
		name[namesize] = (TCHAR)'\0';
	}
	//icona
	CloseHandle(hproc);
}

DWORD AppInfo::getPid()
{
	return pid;
}

DWORD AppInfo::getNameSize()
{
	return namesize;
}

TCHAR * AppInfo::getName()
{
	return (TCHAR *)name;
}

AppInfo::~AppInfo(){
}


