#pragma once

#pragma comment (lib, "ws2_32.lib")
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#include <iostream>
#include "globals.h"
#include "CDBConnector.h"
class CServer {
private:
	int m_retval;
	WSADATA m_wsa;
	HANDLE m_hcp;
	HANDLE m_hthread;

	SOCKET m_listensocket;
	SOCKADDR_IN m_serveraddr;

	SOCKET m_clientsock;
	SOCKADDR_IN m_clientaddr;
	DWORD m_recvBytes, m_flags;
	char buf[BUFSIZE];

	CDBConnector m_dbc;

public:
	CServer();
	~CServer();

	void Start();
	// void WorkerThread();
	static DWORD WINAPI WorkerThread(LPVOID arg);

	void Login(const SOCKET& sock);
	int Login(const SOCKET& sock, char* buf);

	int ProcessPacket(char* buf);

	int recvn(SOCKET s, char* buf, int len, int flags);

	void err_quit(const char* msg);
	void err_display(const char* msg);


	int SignUp(const SOCKET& sock, char* buf);
};

