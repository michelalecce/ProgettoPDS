#include "stdafx.h"
#include "ListWatcher.h"
#include <iostream>


ListWatcher::ListWatcher()
{
}

void ListWatcher::init()
{
	int attempts=4;
	HWND wnd=NULL;
	if(!EnumWindows(enumProc, 0)){ //for every top-level window it calls enumProc
		throw ListException(GetLastError(), "Enum failed");
	}
	while (attempts!=0){
		wnd = GetForegroundWindow();
		if (wnd!=NULL){
			break;
		}
		attempts--; //we do some attempts because sometimes if focus is changing GetForegroundWindow() can fail
	}
	if (wnd ==NULL){
		throw ListException(GetLastError(), "Error while detecting focus owner");
	}
	GetWindowThreadProcessId(wnd, &focus);
}

BOOL ListWatcher::addApp(HWND wnd, LPARAM param)
{
		try{
			applist.push_back(std::move(AppInfo(wnd)));
			return true;
		}
		catch(WindowInfoException e){
			if (e.getErr() == ERROR_ACCESS_DENIED) {
				return true;
			}
			else
				throw e;
		}
}

void ListWatcher::clearList()
{
	for (std::list<AppInfo>::iterator ai = applist.begin(); ai != applist.end(); ai++) ai->cleanIcon(); //eliminates the iconfile for the apps in the list
	applist.clear();
}

void ListWatcher::sendList(SOCKET sock) {
	TCHAR buffer[MAXBUFF];
	void *ptr;
	_stprintf_s(buffer, BUFF, TEXT("lis"));
	*((uint32_t *)(buffer + COMMSIZE)) = htonl(applist.size());
	int n = COMMSIZE * sizeof(TCHAR) + sizeof(uint32_t);
	std::cout << "Application list: (n=" << applist.size()<< ")" << std::endl;
	for (std::list<AppInfo>::iterator ai = applist.begin(); ai!= applist.end(); ai++) {
		//before we push something into the buffer we check if it fits, if not we send before pushing
		if (n + sizeof(DWORD) > BUFF) {
			Lsendn(sock, (char *)buffer, n, 0);
			n = 0;
		}
		ptr = (void *)((char *)buffer + n);	*((DWORD *)ptr) = htonl(ai->getPid()); n += sizeof(DWORD);
		if (n + sizeof(DWORD) > BUFF) {
			Lsendn(sock, (char *)buffer, n, 0);
			n = 0;
		}
		std::cout <<"pid: "<< ai->getPid();
		ptr = (void *)((char *)buffer + n);	*((DWORD *)ptr) = htonl(ai->getNameSize()); n += sizeof(DWORD);
		if (n + ai->getNameSize() * sizeof(TCHAR) > BUFF) {
			Lsendn(sock, (char *)buffer, n, 0);
			n = 0;
		}
		ptr = (void *)( (char *)buffer + n);	_tcscpy_s((TCHAR *)ptr, ai->getNameSize() + 1, ai->getName());
		n += ai->getNameSize() * sizeof(TCHAR);
		_tprintf(_T("name: %s size:%d\n"), ai->getName(), ai->getNameSize());
		//ICOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOON MISSSSSSSSSSSSSSSSSSSSSSSSSSSSIIIIIIIIIIIIIIIIIIIIIIIINNNNNNNNNNNGGGGGGGGG
	}
	if (n!=0)
		Lsendn(sock, (char *)buffer, n, 0);
	//send focus for the first time
	_stprintf_s(buffer, BUFF, TEXT("foc"));
	n = COMMSIZE * sizeof(TCHAR);
	*((DWORD *)(buffer + COMMSIZE)) = htonl(focus); n += sizeof(DWORD);
	Lsendn(sock, (char *)buffer, n, 0);
	std::cout <<"Focus:" << focus << std::endl;
}

void ListWatcher::sendFocus(SOCKET sock){
	TCHAR buffer[BUFF];

	_stprintf_s(buffer, BUFF, TEXT("foc"));
	int n = COMMSIZE * sizeof(TCHAR) + sizeof(uint32_t);
	*((DWORD *)(buffer + n)) = htonl(focus); n += sizeof(DWORD);
	//send MIIIIIIIIIIIIIIIIIIIIIIIIISSSSSSSSSSSSSSSSSSSSSSSSIIIIIIIIIIIIIIIIIIIIIIIIIIIIINNNNNNNNNNNNNNNNNGGGGGGGGGGGGGGGGGG
}