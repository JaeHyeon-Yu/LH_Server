#pragma once

// define 
#define SERVERPORT 9000
#define BUFSIZE 512

// struct
struct SOCKETINFO {
	OVERLAPPED overlapped;
	SOCKET sock;
	char buf[BUFSIZE];
	int recvBytes;
	int sendBytes;
	WSABUF wsabuf;
};