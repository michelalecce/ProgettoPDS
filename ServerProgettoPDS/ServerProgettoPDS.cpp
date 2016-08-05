// ServerProgettoPDS.cpp : definisce il punto di ingresso dell'applicazione console.
//
#include "stdafx.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>

#pragma comment(lib, "Ws2_32.lib")

#include <iostream>
#include <list>
#include "Errors.cpp"
#include "ListWatcher.h"
#include "AppInfo.h"

#define MAXREQUESTS 10

std::list<AppInfo> applist;

int main(int argc, char** argv)
{
	WORD err;
	LPTSTR lpMsg=NULL;
	WSADATA wsaData;
	int port=atoi(argv[2]);
	SOCKET s, conn_sock;
	
	WORD wVersionRequested = MAKEWORD(2, 2);
	WSAStartup(wVersionRequested, &wsaData);
	struct sockaddr_in local, client;
	int client_len;

	s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (s == INVALID_SOCKET) {
		err = WSAGetLastError();
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, err,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), lpMsg, 0, NULL);
		std::cout << "Error while opening socket\nerrno = " << err << "\t" << lpMsg;
		LocalFree(lpMsg);
		ExitProcess(1);
	}
	local.sin_family = AF_INET;
	local.sin_port = htons(port);
	local.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(s, (struct sockaddr *)&local, sizeof(local)) == SOCKET_ERROR) {
		err = WSAGetLastError();
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, err,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), lpMsg, 0, NULL);
		std::cout << "Error while opening socket\nerrno = " << err << "\t" << lpMsg;
		LocalFree(lpMsg);
		closesocket(s);
		ExitProcess(1);
	}
	if (listen(s, MAXREQUESTS) == SOCKET_ERROR) {
		err = WSAGetLastError();
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, err,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), lpMsg, 0, NULL);
		std::cout << "Error while opening socket\nerrno = " << err << "\t" << lpMsg;
		LocalFree(lpMsg);
		closesocket(s);
		ExitProcess(1);
	}
	client_len = sizeof(struct sockaddr_in);
	while (1) {
		try{
			if((conn_sock = accept(s, (struct sockaddr *)&client, &client_len)) == INVALID_SOCKET){
				err = WSAGetLastError();
				FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, err,
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), lpMsg, 0, NULL);
				std::cout << "Error while opening socket\nerrno = " << err << "\t" << lpMsg;
				LocalFree(lpMsg);
				closesocket(conn_sock);
				continue;
			}
			ListWatcher lw;
		}
		catch(ListException){
			
		}
	}
    return 0;
}

