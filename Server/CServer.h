#pragma once

#pragma comment (lib, "ws2_32.lib")
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#include <iostream>
#include <vector>
#include <mutex>
#include "globals.h"
#include "CDBConnector.h"
#include "CLobby.h"
#include "CPlayer.h"

class CServer {
private:
	int m_retval;
	WSADATA m_wsa;
	HANDLE m_hthread;
	HANDLE m_lobbythread;

	SOCKET m_listensocket;
	SOCKADDR_IN m_serveraddr;

	SOCKET m_clientsock;
	SOCKADDR_IN m_clientaddr;
	char buf[BUFSIZE];
	std::mutex m_mu;

	CDBConnector m_dbc;
	CLobby m_lobby[MAX_LOBBY];
	int m_numofRooms;
public:
	CServer() = default;
	~CServer() = default;

	void Start();

	void Login(const SOCKET& sock, CPlayer& player);
	int Login(const SOCKET& sock, CPlayer& player, char* buf);

	void err_quit(const char* msg);
	void err_display(const char* msg);

	// Thread Function
	static DWORD WINAPI WorkerThread(LPVOID arg);
	static DWORD WINAPI UpdateLobby(LPVOID arg);
	static DWORD WINAPI RecvThread(LPVOID arg);

	// Supervise Account with MS-SQL
	int SignUp(const SOCKET& sock, char* buf);

	// Supervise Lobby
	int CreateRoom(const CPlayer& master);

	// for Test
	SOCKET GetSock() { return m_clientsock; }
};

