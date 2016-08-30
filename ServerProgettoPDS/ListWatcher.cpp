#include "stdafx.h"
#include "ListWatcher.h"
#include <iostream>
#include <fstream>
#include "string.h"
#define TYP_INIT 0 
#define TYP_SMLE 1 
#define TYP_BIGE 2

void ListWatcher::sendUpdate(SOCKET sock)
{
	char buffer[BUFF];
	char *ptr;
	if(removeList.size()!=0){
		//we send the communication for removed windows
		sprintf_s(buffer, BUFF, "rem");
		*((uint32_t *)(buffer + COMMSIZE)) = htonl(removeList.size());
		int n = COMMSIZE * sizeof(char) + sizeof(uint32_t);
		std::cout << "Removed application list: (n=" << removeList.size() << ")" << std::endl;
		for (auto ai = removeList.begin(); ai != removeList.end(); ai++) {
			ai->deleteIcon();
			ptr = ((char *)buffer + n);
			pushHandle(ptr, ai->getWindow()); n += sizeof(uint64_t);
			std::cout << "pid: " << ai->getPid() << " handle: " << ai->getWindow();

			ptr = ((char *)buffer + n);	*((DWORD *)ptr) = htonl(ai->getNameSize()); n += sizeof(DWORD);
			strcpy_s(buffer + n, BUFF - n, ai->getNameA());
			n += ai->getNameSize() * sizeof(char);
			//_tprintf(_TEXT("name: %s"), ai->getName());
			printf(" nameA:%s size:%d iconsize:%d\n", ai->getNameA(), ai->getNameSize(), ai->getIconFileSize());
			Lsendn(sock, (char *)buffer, n, 0);
			n = 0;
		}
	}
	if (addList.size()!=0){
		//we send the communication for added windows
		sprintf_s(buffer, BUFF, "add");
		*((uint32_t *)(buffer + COMMSIZE)) = htonl(addList.size());
		int n = COMMSIZE * sizeof(char) + sizeof(uint32_t);
		std::cout << "Added application list: (n=" << addList.size() << ")" << std::endl;
		for (auto ai = addList.begin(); ai != addList.end(); ai++) {

			ptr = ((char *)buffer + n);
			pushHandle(ptr, ai->getWindow()); n += sizeof(uint64_t);
			std::cout << "pid: " << ai->getPid() << " handle: " << ai->getWindow();

			ptr = ((char *)buffer + n);	*((DWORD *)ptr) = htonl(ai->getNameSize()); n += sizeof(DWORD);
			strcpy_s(buffer + n, BUFF - n, ai->getNameA());
			n += ai->getNameSize() * sizeof(char);
			//_tprintf(_TEXT("name: %s"), ai->getName());
			printf(" nameA:%s size:%d iconsize:%d\n", ai->getNameA(), ai->getNameSize(), ai->getIconFileSize());
			Lsendn(sock, (char *)buffer, n, 0);
			try {
				sendIcon(sock, ai->getIconFile(), ai->getIconFileSize());
			}
			catch (IconSendException e) {
				throw ListException(e.getErr(), e.what());
			}
			n = 0;
		}
	}
}

ListWatcher::ListWatcher()
{
}

void ListWatcher::init()
{
	int attempts=100;
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
			AppInfo ai(wnd);
			std::pair<HWND, DWORD> pair(ai.getWindow(), ai.getPid());
			applist.insert(std::pair<std::pair<HWND, DWORD>, AppInfo>(std::move(pair),std::move(ai)));
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

BOOL ListWatcher::checkApp(HWND wnd, LPARAM param)
{
	DWORD pid;
	GetWindowThreadProcessId(wnd, &pid);
	std::pair<HWND, DWORD> pair(wnd, pid);
	auto found = applist.find(pair);
	if (found == applist.end()){
		//the window is new to the map
		AppInfo ai(wnd);
		ai.setStillOpen();
		addList.push_back(ai);
		applist.insert(std::pair<std::pair<HWND, DWORD>, AppInfo>(std::move(pair), std::move(ai)));
		return true;
	}
	else{
		//the window was already present in the map
		found->second.setStillOpen();
		return true;
	}
}

bool ListWatcher::newFocusGood(HWND newfocus){
	DWORD focuspid;
	if (newfocus==NULL)
		return false;
	GetWindowThreadProcessId(newfocus, &focuspid);
	std::pair<HWND, DWORD> pair(newfocus, focuspid);
	auto iter = applist.find(pair);
	return iter != applist.end();
}

static unsigned long long ntoh64(unsigned long long src) {
	static int typ = TYP_INIT;
	unsigned char c;
	union {
		unsigned long long ull;
		unsigned char c[8];
	} x;

	if (typ == TYP_INIT) {
		x.ull = 0x01;
		typ = (x.c[7] == 0x01) ? TYP_BIGE : TYP_SMLE;
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

void ListWatcher::sendCommand(SOCKET sock)
{
	char buffer[BUFF], vmodifier[MODMAX][COMMSIZE +1]= {"ALT","SHI","CTR"}, key;
	bool presentmod[MODMAX]= {false, false, false};
	BYTE virtualkeys[MODMAX] = {VK_MENU ,VK_SHIFT, VK_CONTROL }; //codes of the keys to send
	INPUT vsend[MODMAX + 1], current; // vsend is the vector of input struct to send
	uint64_t clientfoc, serverfocus;
	uint32_t nmod;
	unsigned int i,j, n=0;
	LPARAM messagelparam;DWORD scan;
	//I bring the handle to the focused application on 64 bit
	if (sizeof(focus) == 4) {
		serverfocus = (uint32_t)focus;
	}
	else if (sizeof(focus) == 8) {
		serverfocus = (uint64_t)focus;
	}
	Readn(sock, buffer, sizeof(uint64_t), 0);
	clientfoc= ntoh64(*((uint64_t *) buffer));
	//After reading the handle of the application that the client considers ad the one with the focus, I check it with the current value of focus
	if (clientfoc != serverfocus){
		//the focus has changed
		std::cout<< "Error receiving command from client: wrong focused application handle"<< std::endl;
		sprintf_s(buffer, "err"); Lsendn(sock, buffer, COMMSIZE, 0);
		sendFocus(sock);
		return;
	}
	Readn(sock, buffer, sizeof(uint32_t), 0);
	nmod = ntohl(*((uint32_t *)buffer)); // I read the number of modificators
	Readn(sock, buffer, nmod*3 + 1 , 0);
	//We read 3 byte for each modificator, plus 1 B for the key sent
	for(i=0; i<nmod; i++){
		for(j=0;j<MODMAX; j++){
			if(strncmp(vmodifier[j], buffer + i*COMMSIZE,COMMSIZE)==0){
				//if a modifier is recognized I set the corrispondent flag
				presentmod[j]= true;
			}
		}
	}
	key = buffer[nmod*COMMSIZE];

	// I send the message to the app after preparing the input vector
	current.type = INPUT_KEYBOARD;
	current.ki.time = 0;
	current.ki.dwExtraInfo = NULL;
	current.ki.dwFlags =0;
	for (i=0, n=0;i<MODMAX; i++){
		if(presentmod[i]){
			current.ki.wVk = virtualkeys[i];
			vsend[n] = current;
			n++;
		}
	}
	current.ki.wVk = key;
	vsend[n] = current;
	n++;
	SendInput(n, vsend , sizeof(INPUT));

	for(i=0;i<n;i++){
		vsend[i].ki.dwFlags= KEYEVENTF_KEYUP;
	}
	SendInput(n, vsend, sizeof(INPUT));

	std::cout << "Sent ";
	for (j=0; j<MODMAX; j++){
		if(presentmod[j])
			printf("%s+", vmodifier[j]);
	}
	std::cout << key << " to: " << focus << std::endl;
}

void ListWatcher::updateList(SOCKET sock)
{
	static int consecutive_errors=10;
	char errbuf[ERRBUFF];
	int err;
	int attempts=100;
	if (!EnumWindows(updateProc, 0)) { //for every top-level window it calls updateProc
		err = GetLastError();
		consecutive_errors--;
		if(consecutive_errors ==0){
			throw ListException(err, "EnumWindows failed while updating");
		}
		std::cout << "EnumWindows failed while updating\nerrno = " << err << "\t";
		strerror_s(errbuf, ERRBUFF, err);
		printf("%s\n", errbuf);
	}
	//we fill in the list of windows to remove and removes them from the map
	auto elim_iter = applist.begin();
	for (auto ai = applist.begin(); ai != applist.end();) {
		if (!ai->second.getStillOpen()) {
			removeList.push_back(ai->second);
			elim_iter = ai;
			ai++;
			applist.erase(elim_iter);
		}
		else{
			ai++;
		}
	}
	if(addList.size()!=0 || removeList.size()!=0){
		sendUpdate(sock);
	}
	//we take the new focused windows and update
	newFocus = GetForegroundWindow();
	if (newFocusGood(newFocus) && newFocus!=focus){
		focus=newFocus;
		sendFocus(sock);
	}
	//Now we get ready for the successive update
	addList.clear();
	removeList.clear();
	for(auto ai = applist.begin(); ai!= applist.end(); ai++){
		ai->second.clearStillOpen();
	}
}

void ListWatcher::clearList()
{
	for (auto ai = applist.begin(); ai != applist.end(); ai++) ai->second.deleteIcon(); //eliminates the iconfile for the apps in the list
	applist.clear();
}

void ListWatcher::sendList(SOCKET sock) {
	char buffer[BUFF];
	char *ptr;
	sprintf_s(buffer, BUFF, "lis");
	*((uint32_t *)(buffer + COMMSIZE)) = htonl(applist.size());
	int n = COMMSIZE * sizeof(char) + sizeof(uint32_t);
	std::cout << "Application list: (n=" << applist.size()<< ")" << std::endl;
	for (auto ai = applist.begin(); ai!= applist.end(); ai++) {
		
		ptr = ((char *)buffer + n);	
		pushHandle(ptr, ai->second.getWindow()); n += sizeof(uint64_t);
		std::cout <<"pid: "<< ai->second.getPid() << " handle: "<< ai->second.getWindow();
		
		ptr = ((char *)buffer + n);	*((DWORD *)ptr) = htonl(ai->second.getNameSize()); n += sizeof(DWORD);
		strcpy_s(buffer + n, BUFF - n, ai->second.getNameA());
		n += ai->second.getNameSize() * sizeof(char);
		//_tprintf(_TEXT("name: %s"), ai->getName());
		printf(" nameA:%s size:%d iconsize:%d\n", ai->second.getNameA(), ai->second.getNameSize(), ai->second.getIconFileSize());
		Lsendn(sock, (char *)buffer, n, 0);
		try{
			sendIcon(sock,ai->second.getIconFile(), ai->second.getIconFileSize());
		}
		catch(IconSendException e){
			throw ListException(e.getErr(), e.what());
		}
		n=0;
	}

	//send focus for the first time
	sendFocus(sock);
}


unsigned long long hton64(unsigned long long src) { // HTONL 64 BIT VERSION
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
	*((uint64_t *)buffer) = hton64(longnum);
}

void ListWatcher::sendFocus(SOCKET sock){
	char buffer[BUFF], *ptr;
	uint64_t longnum;
	DWORD focuspid; 
	GetWindowThreadProcessId(focus, &focuspid);
	std::pair<HWND, DWORD> pair(focus, focuspid);
	auto iter= applist.find(pair);
	sprintf_s(buffer, BUFF, "foc");
	int n = COMMSIZE * sizeof(char);
	ptr = buffer + n; pushHandle(ptr, focus); n+= sizeof(uint64_t);
	std::cout << "Focus: handle: " << focus << " pid: " << focuspid << std::endl; 
	ptr = ((char *)buffer + n);	*((DWORD *)ptr) = htonl(iter->second.getNameSize()); n += sizeof(DWORD);
	strcpy_s(buffer + n, BUFF - n, iter->second.getNameA());
	n += iter->second.getNameSize() * sizeof(char);
	//_tprintf(_TEXT("name: %s"), ai->getName());
	//printf(" nameA:%s size:%d iconsize:%d\n", iter->second.getNameA(), iter->second.getNameSize(), iter->second.getIconFileSize());
	Fsendn(sock, buffer, n, 0);
}

void sendIcon(SOCKET sock, TCHAR *file, LONG size)
{
	char buffer[ICONBUFF];
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
