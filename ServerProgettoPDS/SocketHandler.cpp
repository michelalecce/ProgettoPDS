#include "stdafx.h"
#include "SocketHandler.h"

SocketHandler::SocketHandler(SOCKET s, sockaddr_in caddr, int caddr_len)
{
	sock=s; client= caddr; client_len= caddr_len;
}

SocketHandler::~SocketHandler()
{
}
