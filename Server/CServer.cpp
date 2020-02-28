#include "CServer.h"
#include "packet.h"
#define NOT_JOIN_ANY_ROOM -1

SC_LOBBY g_scLobby[MAX_LOBBY];
int g_numofRooms{ 0 };

DWORD WINAPI CServer::WorkerThread(LPVOID arg) {
	CServer* server = (CServer*)arg;
	SOCKET sock = server->GetSock();
	SOCKADDR_IN clientaddr;
	CPlayer player;
	int my_room = NOT_JOIN_ANY_ROOM;
	int my_idx;

	int retval;
	char buf[BUFSIZE];
	int sendBytes = 0;
	int recvBytes = 0;

	int addrlen = sizeof(clientaddr);
	getpeername(sock, (SOCKADDR*)&clientaddr, &addrlen);
	
	while (true) {
		// Receive Packet from Client
		retval = recv(sock, (char*)&recvBytes, sizeof(int), 0);
		retval = recv(sock, buf, recvBytes, 0);
		if (retval == SOCKET_ERROR) {
			server->err_display("recv");
			break;
		}
		if (retval == 0) break;

		// Casting & Process Packet
		if (player.GetState() == not_login) {
			// Connect DB
			server->m_dbc.AllocateHandle();
			server->m_dbc.ConnectDataSource();

			if (buf[0] == login_packet)
				retval = server->Login(sock, player, buf);
			else if (buf[0] == signup_packet)
				retval = server->SignUp(sock, buf);
			if (retval == login_ok) player.SetState(In_Lobby);
			sendBytes = sizeof(SC_LOGIN);

			// Disconnect DB
			server->m_dbc.DisconnectDataSource();
		}
		else if (player.GetState() == In_Lobby) {
			if (buf[0] != lobby_packet) break;
			buf[recvBytes] = '\0';
			CS_LOBBY *csLobby = (CS_LOBBY*)buf;

			// 2 - 1 방 생성 
			if (csLobby->state == create_room) {
				std::cout << "받긴하냐" << std::endl;
				int rnum = server->CreateRoom(player);
				player.SetState(host);
				g_scLobby[rnum].state = create_room;
				g_scLobby[rnum].type = lobby_packet;
				my_room = rnum;
				Sleep(1000/60);	// Stay 1 frame. for Update user count

				// Create Struct To Buf
				sendBytes = sizeof(SC_LOBBY);
				ZeroMemory(buf, BUFSIZE);
				memcpy(buf, &g_scLobby[rnum], sendBytes);
			}
			// 2 - 2 방 참여
			if (csLobby->state == join_room) {
				SC_LOBBY scLobby;
				if (0 < server->m_lobby[csLobby->num].GetUsers() && server->m_lobby[csLobby->num].GetUsers() < MAX_PLAYER) {
					my_idx = server->m_lobby[csLobby->num].AddUser(player);
					player.SetState(none_ready);
					my_room = csLobby->num;
					scLobby = g_scLobby[my_room];
				}
				else {
					scLobby.type = lobby_packet;
					scLobby.idx = NOT_JOIN_ANY_ROOM;
					scLobby.users = 0;
					scLobby.state = join_room;
				}
				Sleep(1000 / 60);	// Stay 1 frame. for Update user count

				// Create Struct To Buf
				sendBytes = sizeof(SC_LOBBY);
				ZeroMemory(buf, BUFSIZE);
				memcpy(buf, &scLobby, sendBytes);
			}
			// 2 - 3 방 업데이트받기
			if (csLobby->state == update_room) {
				g_scLobby[0].state = update_room;	// 지금 0만 가능한데 방 idx를 받아와서 하는거로 수정을 해야지?
				sendBytes = g_numofRooms * sizeof(SC_LOBBY);
				ZeroMemory(buf, BUFSIZE);
				memcpy(buf, &g_scLobby[csLobby->num], sendBytes);
			}
		}
		else if (player.GetState() == host || player.GetState() == none_ready || player.GetState() == ready) {
		//	if (buf[0] != lobby_packet) break;
			buf[recvBytes] = '\0';
			CS_LOBBY *csLobby = (CS_LOBBY*)buf;
			SC_LOBBY scLobby;
			// 1 방 안에서 대기하기
			// 1 - 2 방에서 나가기
			if (csLobby->state == out_room) {
				server->m_lobby[csLobby->num].SubUsers(player);
				scLobby.type = lobby_packet;
				scLobby.users = -1;
				// Create struct to buf
				sendBytes = sizeof(SC_LOBBY);
				ZeroMemory(buf, BUFSIZE);
				memcpy(buf, &scLobby, sendBytes);
			}

			// 1 - 3 들어온 플레이어 정보 보내기
			// 플레이어 패킷 구조체를 로비 안의 플레이어의 수만큼 생성하여 보낸다.
			else if (csLobby->state == update_player) {
				SC_PLAYER scPlayer[MAX_PLAYER];
				for (int i = 0; i < MAX_PLAYER; ++i)
					scPlayer[i] = server->m_lobby[csLobby->num].GetPlayer2Pack(i);
					// 지금 여기서 로비 번호를 제대로 못받아오고있음
					// 여윽시 여윽시
				// Create struct to buf
				sendBytes = sizeof(scPlayer);
				ZeroMemory(buf, BUFSIZE);
				memcpy(buf, &scPlayer, sendBytes);
			}
			/*
			레디한 상태에서 정보 업데이트를 요청하면 패킷 맨 앞에
			player패킷을 나타내는 값이 아니라 lobby 패킷을 나타내는 값이 들어간다
			이건 또 왜 이러는데 뭐가 문제야 진짜 팍씨

			*/
			// 2 게임 시작
			
			// 2 - 1 레디박기
			else if (csLobby->state == ready) {
				server->m_lobby[csLobby->num].ReadyPlayer(my_idx);	
				SC_PLAYER scPlayer;
				scPlayer.type = player_packet;
				scPlayer.state = ready;
				player.SetState(ready);
				sendBytes = sizeof(SC_PLAYER);
				ZeroMemory(buf, BUFSIZE);
				memcpy(buf, &scPlayer, sendBytes);
				printf("%s is Ready\n", player.GetID().c_str());
			}
			// 2 - 2 게임시작 (방장)
			else if (csLobby->state = start) {
				
				// 1 플레이어가 호스트인가
				if (server->m_lobby[csLobby->num].IsHost(player)) {
					// 2 모든 플레이어가 ready 상태인가
						// 여기까지 들어오면 start가 확정되므로 전역으로 선언한 로비배열 건드려도 ㅇㅈ이다.
						// 아 어차피 멤버변수 건드리면 자동적으로 바꿔주겠구나
						// 이걸 또 아닐때 처리해주는 것도 복잡한데
					if (server->m_lobby[csLobby->num].Start())
						scLobby.state = start_ok;
					else scLobby.state = start_fail;
					scLobby.type = lobby_packet;
					scLobby.idx = my_room;
					scLobby.users = g_scLobby[my_room].users;
					// scLobby.state = start_ok;
				}
				ZeroMemory(buf, BUFSIZE);
				sendBytes = sizeof(SC_LOBBY);
				memcpy(buf, &scLobby, sendBytes);
						/*
						이게 찐 게임시작이니까					
						플레이어들의 상태를 모두 게임중으로 바꾸고
						방의 상태도 게임중으로 바꾸자
						아 그러면 로비 출력하때 대기중인 방인지 게임중인 방인지 그것도 판별 가능해야겠다
						아
						생각해보면 할거 많은데 하기싫어서 그런게 맞는거같다 결국
						씨발앰생 ㅈ같은 새끼 ㄹㅇ개폐급은 나였구요 ㅋㅋㅋㅋㅋ
						*/
				
				/*
				사실 저걸 둘 다 만족해야지 ㅇㅇ
				호스트 판별은 사실 클라에서 처리하지만 서버에서 한번 더 판별해주자
	
				*/
			}
		}
		else if (player.GetState() == In_Game) {

		}

		// Send Packet to Client
		retval = send(sock, (char*)&sendBytes, sizeof(int), 0);
		retval = send(sock, (char*)&buf, sendBytes, 0);
		if (retval == SOCKET_ERROR) { 
			server->err_display("send"); 
			break;
		}
		if (retval == 0) break;

		printf("end of cycle of While\n");
	}
	
	printf("%s is Log-Out\n", player.GetID().c_str());
	if (my_room != NOT_JOIN_ANY_ROOM) 
		server->m_lobby[my_room].SubUsers(player);
	closesocket(sock);	
	return 0;
}
DWORD WINAPI CServer::UpdateLobby(LPVOID arg) {
	CLobby* lobby = (CLobby*)arg;
	int cnt{0};
	while (true) {
		cnt = 0;
		for (int i = 0; i < MAX_LOBBY; ++i)
			if (lobby[i].GetUsers() > 0) {
				int lobbyIdx = lobby[i].GetIdx();
				int lobbyUsers = lobby[i].GetUsers();
				g_scLobby[cnt].idx = lobbyIdx;
				g_scLobby[cnt].users = lobbyUsers;
				g_scLobby[cnt].type = lobby_packet;
				cnt++;
			}
		g_numofRooms = cnt;
		Sleep(1000/30);
	}

	return 0;
}

void CServer::Start() {
	if (WSAStartup(MAKEWORD(2, 2), &m_wsa) != 0) return;

	m_listensocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (m_listensocket == SOCKET_ERROR) err_quit("WSASocket");

	// Set nagle algorithm off
	BOOL option = TRUE;
	setsockopt(m_listensocket, IPPROTO_TCP, TCP_NODELAY, (char*)& option, sizeof(option));

	ZeroMemory(&m_serveraddr, sizeof(m_serveraddr));
	m_serveraddr.sin_family = AF_INET;
	m_serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	m_serveraddr.sin_port = htons(SERVERPORT);
	m_retval = bind(m_listensocket, (SOCKADDR*)&m_serveraddr, sizeof(m_serveraddr));
	if (m_retval == SOCKET_ERROR) err_quit("bind");

	m_retval = listen(m_listensocket, SOMAXCONN);
	if (m_retval == SOCKET_ERROR) err_quit("listen");

	// Create Lobby Thread for Update Lobby & Rooms
	m_lobbythread = CreateThread(NULL, 0, UpdateLobby, (LPVOID)m_lobby, 0, NULL);

	while (true) {
		int addrlen = sizeof(m_clientaddr);
		m_clientsock = accept(m_listensocket, (SOCKADDR*)&m_clientaddr, &addrlen);
		if (m_clientsock == INVALID_SOCKET) {
			err_display("accept");
			break;
		}
		
		m_hthread = CreateThread(NULL, 0, WorkerThread, (LPVOID)this, 0, NULL);
		if (m_hthread == NULL) closesocket(m_clientsock);
		else CloseHandle(m_hthread);
	}

	WSACleanup();
	return;
}
int CServer::Login(const SOCKET& sock, CPlayer& player, char* buf) {
	buf[sizeof(CS_LOGIN)] = '\0';
	CS_LOGIN *csLogin = (CS_LOGIN*)buf;
	std::string sql = "select * from Account where id  = \'" + (std::string)csLogin->id +
		"\' and password = \'" + (std::string)csLogin->password + "\'";
	printf("%s \n", sql);
	SC_LOGIN scLogin{ login_packet, NULL };
	m_mu.lock();
	m_retval = m_dbc.ExcuteStatementDirect((SQLCHAR*)sql.c_str());
	if (m_dbc.RetrieveResult(csLogin->id, csLogin->password) == false)
		scLogin.state = login_error;
	m_mu.unlock();
	if (m_retval == SQL_EXUTE_OK && scLogin.state != login_error) {
		scLogin.state = login_ok;
		player.Initialize(CPlayer(std::string(csLogin->id), std::string(csLogin->password))); // DB에 레벨 등 다른 정보들 추가시 수정
		printf("%s is Log-in \n", csLogin->id);
	}
	else scLogin.state = login_error;
	
	ZeroMemory(buf, BUFSIZE);
	memcpy(buf, &scLogin, sizeof(SC_LOGIN));	
	return scLogin.state;
}

int CServer::SignUp(const SOCKET& sock, char* buf) {
	buf[sizeof(CS_LOGIN)] = '\0';
	CS_LOGIN *csSign = (CS_LOGIN*)buf;
	std::string sql = "INSERT INTO Account VALUES (\'" + (std::string)csSign->id +
		"\', \'" + (std::string)csSign->password+ "\')";
	
	SC_LOGIN scSign{ signup_packet, NULL };
	m_retval = m_dbc.ExcuteStatementDirect((SQLCHAR*)sql.c_str());
	if (m_retval == SQL_EXUTE_OK) {
		scSign.state = signup_ok;
		printf("Create New Account - ID : %s \n", csSign->id);
	}
	else scSign.state = signup_error;
	ZeroMemory(buf, BUFSIZE);
	memcpy(buf, &scSign, sizeof(SC_LOGIN));
	return scSign.state;
}

int CServer::CreateRoom(const CPlayer& master) {
	for (int i = 0; i < MAX_LOBBY; ++i)
		if (m_lobby[i].GetUsers() == 0) {
			m_lobby[i].SetIdx(i);
			m_lobby[i].AddUser(master);
			m_lobby[i].SetHost(master);
			
			printf("Create Room - %d\t host : %s \n", i, m_lobby[i].GetHostID().c_str());
			return i;
		}
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