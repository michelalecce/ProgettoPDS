#include "stdafx.h"
#include "ListWatcher.h"
#include <iostream>
#include <fstream>
#include "string.h"
#define TYP_INIT 0 
#define TYP_SMLE 1 
#define TYP_BIGE 2


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
	//GetWindowThreadProcessId(wnd, &focus);
	focus=wnd;
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
	//for (std::list<AppInfo>::iterator ai = applist.begin(); ai != applist.end(); ai++) ai->cleanIcon(); //eliminates the iconfile for the apps in the list
	//OOOOOOOOOOOOOOOOOOOOOOCCCCCCCCCCCCCCCCCCCCCHHHHHHHHHHHHHHHHHHHHHHHHHIIIIIIIIIIIIIIOOOOOOOOOOOOOOOOO QUAAAAAAAAAAAAAAAAAAAAAA
	applist.clear();
}

void ListWatcher::sendList(SOCKET sock) {
	char buffer[BUFF];
	char *ptr;
	sprintf_s(buffer, BUFF, "lis");
	*((uint32_t *)(buffer + COMMSIZE)) = htonl(applist.size());
	int n = COMMSIZE * sizeof(char) + sizeof(uint32_t);
	std::cout << "Application list: (n=" << applist.size()<< ")" << std::endl;
	for (std::list<AppInfo>::iterator ai = applist.begin(); ai!= applist.end(); ai++) {
		
		ptr = ((char *)buffer + n);	
		pushHandle(ptr, ai->getWindow()); n += sizeof(uint64_t);
		std::cout <<"pid: "<< ai->getPid() << " handle: "<< ai->getWindow();
		
		ptr = ((char *)buffer + n);	*((DWORD *)ptr) = htonl(ai->getNameSize()); n += sizeof(DWORD);
		strcpy_s(buffer + n, BUFF - n, ai->getNameA());
		n += ai->getNameSize() * sizeof(char);
		//_tprintf(_TEXT("name: %s"), ai->getName());
		printf(" nameA:%s size:%d iconsize:%d\n", ai->getNameA(), ai->getNameSize(), ai->getIconFileSize());
		Lsendn(sock, (char *)buffer, n, 0);
		try{
			sendIcon(sock,ai->getIconFile(), ai->getIconFileSize());
		}
		catch(IconSendException e){
			throw ListException(e.getErr(), e.what());
		}
		n=0;
	}

	//send focus for the first time
	sprintf_s(buffer, BUFF, "foc");
	n = COMMSIZE * sizeof(char);
	pushHandle(buffer + n, focus); n += sizeof(uint64_t);
	Lsendn(sock, (char *)buffer, n, 0);
	std::cout <<"Focus:" << focus << std::endl;
}


unsigned long long htonll(unsigned long long src) { // HTONL 64 BIT VERSION
	static int typ = TYP_INIT;
	unsigned char c;
	union {
		unsigned long long ull;
		unsigned char c[8];
	} x;
	if (typ == TYP_INIT) {
		x.ull = 0x01;
		typ = (x.c[7] == 0x01ULL) ? TYP_BIGE : TYP_SMLE;
	}
	if (typ == TYP_BIGE)
		return src;
	x.ull = src;
	c = x.c[0]; x.c[0] = x.c[7]; x.c[7] = c;
	c = x.c[1]; x.c[1] = x.c[6]; x.c[6] = c;
	c = x.c[2]; x.c[2] = x.c[5]; x.c[5] = c;
	c = x.c[3]; x.c[3] = x.c[4]; x.c[4] = c;
	return x.ull;
}

void pushHandle(char * buffer, HWND h){
	uint64_t longnum;
	//I PUT THE HANDLE ON 64 BIT BECAUSE IT'S A POINTER IN TRUTH. TO MAKE THE CODE PORTABLE 64 BIT IS THE SAFEST CHOICE FOR INTEGER
	if (sizeof(h) == 4) {
		longnum = (uint32_t)h;
	}
	else if (sizeof(h) == 8) {
		longnum = (uint64_t)h;
	}
	*((uint64_t *)buffer) = htonll(longnum);
}

void ListWatcher::sendFocus(SOCKET sock){
	char buffer[BUFF];
	uint64_t longnum;
	sprintf_s(buffer, BUFF, "foc");
	int n = COMMSIZE * sizeof(char);
	pushHandle(buffer + n, focus); n+= sizeof(uint64_t);
	Fsendn(sock, buffer, n, 0);
}

void sendIcon(SOCKET sock, TCHAR *file, LONG size)
{
	char buffer[BUFF];
	void *ptr;
	int n=0;
	if(size==0){
		*((LONG *) buffer) = htonl(0);
		Isendn(sock, buffer, sizeof(LONG),0);
	}
	else{
		*((LONG *)buffer) = htonl(size); n= sizeof(LONG);
		std::ifstream fin(file);

		fin.read((buffer + n), size); n+=size;
		Isendn(sock,buffer,n,0);
	}
}
