// ServerProgettoPDS.cpp : definisce il punto di ingresso dell'applicazione console.
//
#include "stdafx.h"
#include "Sockwrap.h"
#include <iostream>
#include <list>
#include "Errors.cpp"
#include "ListWatcher.h"
#include "AppInfo.h"

#define MAXREQUESTS 10
#define ERRBUFF 80

ListWatcher lw;
void sendList(SOCKET);

int main(int argc, char** argv)
{
	DWORD err;
	LPTSTR lpMsg=NULL;
	WSADATA wsaData;
	int port=atoi(argv[1]);
	char errbuf[ERRBUFF];
	SOCKET s, conn_sock;
	
	WORD wVersionRequested = MAKEWORD(2, 2);
	WSAStartup(wVersionRequested, &wsaData);
	struct sockaddr_in local, client;
	int client_len;
	// socket creation and preparation
	s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s == INVALID_SOCKET) {
		err = WSAGetLastError();
		std::cout << "Error while opening socket\nerrno = " << err << "\t";
		strerror_s(errbuf, ERRBUFF, err);
		printf("%s\n", errbuf);
		ExitProcess(1);
	}
	local.sin_family = AF_INET;
	local.sin_port = htons(port);
	local.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(s, (struct sockaddr *)&local, sizeof(local)) == SOCKET_ERROR) {
		err = WSAGetLastError();
		std::cout << "Error while opening socket\nerrno = " << err << "\t";
		strerror_s(errbuf, ERRBUFF, err);
		printf("%s\n", errbuf);
		closesocket(s);
		ExitProcess(1);
	}
	if (listen(s, MAXREQUESTS) == SOCKET_ERROR) {
		err = WSAGetLastError();
		std::cout << "Error while opening socket\nerrno = " << err << "\t";
		strerror_s(errbuf, ERRBUFF, err);
		printf("%s\n", errbuf);
		closesocket(s);
		ExitProcess(1);
	}
	client_len = sizeof(struct sockaddr_in);
	while (1) {
		try{
			std::cout << argv[0] << "-Waiting for connections" << std::endl;
			if((conn_sock = accept(s, (struct sockaddr *)&client, &client_len)) == INVALID_SOCKET){
				err = WSAGetLastError();
				std::cout << "Error while opening socket\nerrno = " << err << "\t" ;
				strerror_s(errbuf, ERRBUFF, err);
				printf("%s\n", errbuf);
				closesocket(conn_sock);
				continue;
			}
			//once we have a client connected we initialize the watcher to create and send the list of application
			std::cout << argv[0] << "-Connection established"<< std::endl;
			lw.init();
			lw.sendList(conn_sock);
			//at the end we clear the list, it will be created again with the next connection
			lw.clearList();
			getchar(); // solo per fermare il DEBUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG
		}
		catch(ListException e){
			//we have only the socket to release at this moment
			std::cout << e.what() <<"\nerrno = " << e.getErr() << "\t" ;
			strerror_s(errbuf, ERRBUFF, e.getErr());
			printf("%s\n", errbuf);
			closesocket(conn_sock);
			lw.clearList();
		}
		catch (WindowInfoException e) {
			//we have only the socket to release at this moment
			std::cout << e.what() << "\nerrno = " << e.getErr() << "\t";
			strerror_s(errbuf, ERRBUFF, e.getErr());
			printf("%s\n", errbuf);
			closesocket(conn_sock);
			lw.clearList();
		}
	}
    return 0;
}

BOOL IsAltTabWindow(HWND hwnd)
{
	TITLEBARINFO ti;
	HWND hwndTry, hwndWalk = NULL;

	if (!IsWindowVisible(hwnd))
		return FALSE;

	hwndTry = GetAncestor(hwnd, GA_ROOTOWNER);
	while (hwndTry != hwndWalk)
	{
		hwndWalk = hwndTry;
		hwndTry = GetLastActivePopup(hwndWalk);
		if (IsWindowVisible(hwndTry))
			break;
	}
	if (hwndWalk != hwnd)
		return FALSE;

	// the following removes some task tray programs and "Program Manager"
	ti.cbSize = sizeof(ti);
	GetTitleBarInfo(hwnd, &ti);
	if (ti.rgstate[0] & STATE_SYSTEM_INVISIBLE)
		return FALSE;

	// Tool windows should not be displayed either, these do not appear in the
	// task bar.
	if (GetWindowLong(hwnd, GWL_EXSTYLE) & WS_EX_TOOLWINDOW)
		return FALSE;

	return TRUE;
}


BOOL CALLBACK enumProc(HWND wnd, LPARAM param) {
	if (!IsAltTabWindow(wnd)) //STACKOVERFLOW FUNCTION: returns true if wnd appears when alt-tabbing
		return true; //if it's not a window visible alt-tabbing we skip the wnd, so we go on with the enumeration 
	return lw.addApp(wnd, param);
}