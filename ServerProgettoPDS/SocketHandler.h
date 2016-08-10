#pragma once

class SocketHandler
{
private:
	SOCKET sock;
	sockaddr_in client;
	int client_len;

public:
	SocketHandler(SOCKET s, sockaddr_in caddr, int caddr_len);
	~SocketHandler();
};

