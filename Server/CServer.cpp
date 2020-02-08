#include "CServer.h"
#include "packet.h"

DWORD WINAPI CServer::WorkerThread(LPVOID arg) {
	// Login(m_clien
	CServer* server = (CServer*)arg;
	SOCKET sock = server->m_clientsock;
	HANDLE hcp = server->m_hcp;
	int clientIdx = -1;
	int state = -1; // 
	int retval;

	while (true) {
		DWORD recvBytes{ 0 };
		DWORD flags{ 0 };
		DWORD cbTransferred{ 0 };
		SOCKET clientsock;
		SOCKETINFO *ptr;

		retval = GetQueuedCompletionStatus(hcp, &cbTransferred, (LPDWORD)&clientsock, 
			(LPOVERLAPPED*)&ptr, INFINITE);

		SOCKADDR_IN clientaddr;
		int addrlen = sizeof(clientaddr);
		getpeername(ptr->sock, (SOCKADDR*)&clientaddr, &addrlen);

		if (retval == 0 || cbTransferred == 0) {
			if (retval == 0) {
				DWORD temp1, temp2;
				WSAGetOverlappedResult(ptr->sock, &ptr->overlapped, &temp1, FALSE, &temp2);
				printf("[WSAGetOverlappedResult()] [%d] \n", __LINE__);
			}
			closesocket(ptr->sock);
			printf("[TCP Server] Client Out : %s \n", inet_ntoa(clientaddr.sin_addr));
			delete ptr;
			continue;
		}


		retval = WSARecv(ptr->sock, &ptr->wsabuf, 1, &recvBytes, &flags, &ptr->overlapped, NULL);
		if (retval == SOCKET_ERROR) {
			if (WSAGetLastError() != WSA_IO_PENDING)
				server->err_display("WSARecv()");
			continue;
		}
		if (state == -1) {
			server->Login(sock);
			state = In_Lobby;
		}
	}
	
	
	return 0;
}
CServer::CServer() {
	m_dbc.AllocateHandle();
	m_dbc.ConnectDataSource();
}
CServer::~CServer() {
	m_dbc.DisconnectDataSource();
}
void CServer::Start() {
	if (WSAStartup(MAKEWORD(2, 2), &m_wsa) != 0) return;

	m_hcp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
	if (m_hcp == NULL) return;

	SYSTEM_INFO si;
	GetSystemInfo(&si);

	for (int i = 0; i < (int)si.dwNumberOfProcessors * 2; ++i) {
		m_hthread = CreateThread(NULL, 0, WorkerThread, (LPVOID*)this, 0, NULL);
		if (m_hthread == NULL) return;
		CloseHandle(m_hthread);
	}

	m_listensocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (m_listensocket == SOCKET_ERROR) err_quit("WSASocket");

	ZeroMemory(&m_serveraddr, sizeof(m_serveraddr));
	m_serveraddr.sin_family = AF_INET;
	m_serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	m_serveraddr.sin_port = htons(SERVERPORT);
	m_retval = bind(m_listensocket, (SOCKADDR*)&m_serveraddr, sizeof(m_serveraddr));
	if (m_retval == SOCKET_ERROR) err_quit("bind");

	m_retval = listen(m_listensocket, SOMAXCONN);
	if (m_retval == SOCKET_ERROR) err_quit("listen");

	while (true) {
		int addrlen = sizeof(m_clientaddr);
		m_clientsock = accept(m_listensocket, (SOCKADDR*)&m_clientaddr, &addrlen);
		if (m_clientsock == INVALID_SOCKET) {
			err_display("accept");
			break;
		}

		CreateIoCompletionPort((HANDLE)m_clientsock, m_hcp, m_clientsock, 0);

	//	// 접속 로그 찍기 여기서 로그인 처리
	//	Login(m_clientsock);

		// SOCKETINFO 할당
		SOCKETINFO *ptr = new SOCKETINFO;
		if (ptr == NULL) break;
		ZeroMemory(&ptr->overlapped, sizeof(ptr->overlapped));
		ptr->sock = m_clientsock;
		ptr->recvBytes = ptr->sendBytes = 0;
		ptr->wsabuf.buf = ptr->buf;
		ptr->wsabuf.len = BUFSIZE;

		// Async IO Start
		m_flags = 0;
		m_retval = WSARecv(m_clientsock, &ptr->wsabuf, 1, &m_recvBytes,
			&m_flags, &ptr->overlapped, NULL);
		if (m_retval == SOCKET_ERROR) {
			if (WSAGetLastError() != ERROR_IO_PENDING)
				err_display("WSARecv");
			continue;
		}
	}

	WSACleanup();
	return;
}
void CServer::Login(const SOCKET& sock) {
	int recvBytes = NULL;
	int state = -1;

	while (state != login_ok) {
		// recv login packet 
		m_retval = recv(sock, (char*)&recvBytes, sizeof(int), 0);
		m_retval = recv(sock, buf, recvBytes, 0);
		if (m_retval == SOCKET_ERROR) err_display("recv");

		if (buf[0] == login_packet)
			m_retval = Login(sock, buf);
		else if (buf[0] == signup_packet)
			m_retval = SignUp(sock, buf);
	}

}

int CServer::ProcessPacket(char* buf) {
	switch (buf[0]) {
	case login_packet:
		m_dbc.RetrieveResult();
		break;
	case signup_packet:
		// SignUp(buf);
		break;
	default:
		break;
	}
}

int CServer::Login(const SOCKET& sock, char* buf) {
	buf[sizeof(CS_LOGIN)] = '\0';
	CS_LOGIN *csLogin = (CS_LOGIN*)buf;
	std::string sql = "select * from Account where id  = \'" + (std::string)csLogin->id +
		"\' and \'" + (std::string)csLogin->password + "\'";

	SC_LOGIN scLogin{ login_packet, NULL };
	int sendBytes = sizeof(SC_LOGIN);
	m_retval = m_dbc.ExcuteStatementDirect((SQLCHAR*)sql.c_str());
	if (m_retval == SQL_EXUTE_OK) {
		scLogin.state = login_ok;
		printf("%s is Log-in \n", csLogin->id);
	}
	else scLogin.state = login_error;
	m_retval = send(sock, (char*)&sendBytes, sizeof(int), 0);
	m_retval = send(sock, (char*)&scLogin, sizeof(SC_LOGIN), 0);
	if (m_retval == SOCKET_ERROR) err_display("send");
	return scLogin.state;
}


int CServer::SignUp(const SOCKET& sock, char* buf) {
	buf[sizeof(CS_SIGNUP)] = '\0';
	CS_SIGNUP *csSign = (CS_SIGNUP*)buf;
	std::string sql = "INSERT INTO Account VALUES (\'" + (std::string)csSign->id +
		"\', \'" + (std::string)csSign->password+ "\')";
	
	SC_SIGNUP scSign{ signup_packet, NULL };
	int sendBytes = sizeof(SC_SIGNUP);
	m_retval = m_dbc.ExcuteStatementDirect((SQLCHAR*)sql.c_str());
	if (m_retval == SQL_EXUTE_OK) {
		scSign.state = signup_ok;
		printf("Create New Account - ID : %s \n", csSign->id);
	}
	else scSign.state = signup_error;
	m_retval = send(sock, (char*)&sendBytes, sizeof(int), 0);
	m_retval = send(sock, (char*)&scSign, sizeof(SC_SIGNUP), 0);
	if (m_retval == SOCKET_ERROR) err_display("send");
	return scSign.state;
}

int CServer::recvn(SOCKET s, char* buf, int len, int flags) {
	int received;
	char *ptr = buf;
	int left = len;

	while (left > 0) {
		received = recv(s, ptr, left, flags);
		if (received == SOCKET_ERROR)
			return SOCKET_ERROR;
		else if (received == 0)
			break;
		left -= received;
		ptr += received;
	}
	return (len - left);
}

void CServer::err_quit(const char* msg) {
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL
	);
	MessageBox(NULL, (LPCTSTR)lpMsgBuf, msg, MB_ICONERROR);
	LocalFree(lpMsgBuf);
	exit(1);
}
void CServer::err_display(const char* msg) {
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL
	);
	printf("[%s] %s", msg, (char*)lpMsgBuf);
	LocalFree(lpMsgBuf);
}
