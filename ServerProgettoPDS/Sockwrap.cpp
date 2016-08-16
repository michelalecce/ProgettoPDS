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