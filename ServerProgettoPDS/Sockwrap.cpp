#include "stdafx.h"
#include "Sockwrap.h"


int sendn(SOCKET s, char * buffer, int len, int flags)
{
	int nleft;
	int nwritten;
	const char *ptr;

	ptr = buffer;
	nleft = len;
	while (nleft > 0)
	{
		if ((nwritten = send(s, ptr, nleft, flags)) == SOCKET_ERROR)
		{
			return nwritten;
		}
		nleft -= nwritten;
		ptr += nwritten;
	}
	return len;
}

int readn(int fd, char *vptr, int n,int flags)
{
	int nleft;
	int nread;
	char *ptr;

	ptr = vptr;
	nleft = n;
	while (nleft > 0)
	{
		if ((nread = recv(fd, ptr, nleft,0)) ==SOCKET_ERROR)
		{
			return -1;
		}
		else
			if (nread == 0)
				break; /* EOF */

		nleft -= nread;
		ptr += nread;
	}
	return n - nleft;
}

void Lsendn(SOCKET s, char * buffer, int len, int flags){
	if (sendn(s, buffer, len, flags) != len)
		throw ListException(WSAGetLastError(), "Error while sending info through the socket");
}

void Isendn(SOCKET s, char * buffer, int len, int flags)
{
	if (sendn(s, buffer, len, flags) != len)
		throw IconSendException(WSAGetLastError(), "Error while sending icon through the socket");
}


void Fsendn(SOCKET s, char * buffer, int len, int flags)
{
	if (sendn(s, buffer, len, flags) != len)
		throw FocusSendException(WSAGetLastError(), "Error while sending focus through the socket");
}

void Readn(SOCKET s, char * buffer, int len, int flags){
	int res= readn(s,buffer, len, flags);
	if (res < 0){
		throw ReadException(WSAGetLastError(), "Error while reading from the socket");
	}
	else if (res!= len){
		throw ReadException(0, "Client closed connection");
	}
}