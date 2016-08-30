#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include "windows.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>

#pragma comment(lib, "Ws2_32.lib")
#include "Errors.cpp"

void Lsendn(SOCKET , char*, int, int);
void Isendn(SOCKET, char*, int, int);
void Fsendn(SOCKET, char*, int, int);
void Readn(SOCKET, char*, int, int);