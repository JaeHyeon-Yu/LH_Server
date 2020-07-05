#include "CDBConnector.h"
#include <iostream>
#include <queue>
#include <mutex>
#include <map>
#include "CPlayer.h"

using namespace std;

enum ENUMOP { OP_RECV, OP_SEND, OP_ACCEPT, EV_MONSTER, EV_BOSS, EV_LOGIN, EV_SIGN, EV_UPDATE };

queue<DB_EVENT> quaryQueue;
mutex quaryLock;

extern HANDLE g_iocp;
extern map<int, CPlayer*> g_player;
void CDBConnector::AllocateHandle() {
	// Allocate ODBC Handle
	retcode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);

	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		// OBCD Driver Version 명시
		retcode = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
			// Allocate Connect Handler
			retcode = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);

			if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
				printf("DBConnector ODBC Handle Allocate Success \n");
			else {
				SQLGetDiagRec(SQL_HANDLE_DBC, hdbc, ++rec, state, 
					&native, message, sizeof(message), &length);
				printf("%s : %ld : %ld : %s \n", state, rec, native, message);
			}
		}

		else {
			SQLGetDiagRec(SQL_HANDLE_ENV, henv, ++rec, state,
				&native, message, sizeof(message), &length);
			printf("%s : %ld : %ld : %s \n", state, rec, native, message);
		}
	}
	else {
		SQLGetDiagRec(SQL_HANDLE_ENV, henv, rec, state,
			&native, message, sizeof(message), &length);
		printf("%s : %ld : %ld : %s \n", state, rec, native, message);
	}
}

void CDBConnector::ConnectDataSource() {
	// Connect MsSQL with ODBC
	retcode = SQLConnect(hdbc, (SQLCHAR*)ODBC_NAME, SQL_NTS,
		(SQLCHAR*)DB_ID, SQL_NTS, (SQLCHAR*)DB_PW, SQL_NTS);
	printf("Complete Connect DB \n");
}

int CDBConnector::ExcuteStatementDirect(SQLCHAR* sql) {
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
		printf("DB Connect Success \n");
	}
	else {
		SQLGetDiagRec(SQL_HANDLE_DBC, hdbc, ++rec, state,
			&native, message, sizeof(message), &length);
		printf("%s : %ld : %ld : %s \n", state, rec, native, message);
	}

	retcode = SQLExecDirect(hstmt, sql, SQL_NTS);	// Excute SQL

	if (retcode == SQL_SUCCESS)
		printf("Query Suceess \n");
	else {
		SQLGetDiagRec(SQL_HANDLE_STMT, hstmt, ++rec, state,
			&native, message, sizeof(message), &length);
		printf("%s : %ld : %ld : %s \n", state, rec, native, message);
		return SQL_EXUTE_FAIL;
	}

	return SQL_EXUTE_OK;
}

void CDBConnector::PrepareStatement(SQLCHAR* sql) {
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
		printf("DB Connect Success \n");
	}
	else {
		SQLGetDiagRec(SQL_HANDLE_DBC, hdbc, ++rec, state,
			&native, message, sizeof(message), &length);
		printf("%s : %ld : %ld : %s \n", state, rec, native, message);
	}

	retcode = SQLPrepare(hstmt, sql, SQL_NTS);

	if (retcode == SQL_SUCCESS)
		printf("Query Prepate Success\n");
	else {
		SQLGetDiagRec(SQL_HANDLE_DBC, hdbc, ++rec, state,
			&native, message, sizeof(message), &length);
		printf("%s : %ld : %ld : %s \n", state, rec, native, message);
	}
}

void CDBConnector::ExcuteStatement() {
	retcode = SQLExecute(hstmt);

	if (retcode == SQL_SUCCESS)
		printf("Query Exute Success \n");
	else {
		SQLGetDiagRec(SQL_HANDLE_DBC, hdbc, ++rec, state,
			&native, message, sizeof(message), &length);
		printf("%s : %ld : %ld : %s \n", state, rec, native, message);
	}
}

bool CDBConnector::RetrieveResult(std::string Cid, std::string Cpass) {
	char AccountId[10], AccouuntPass[10];
	SQLLEN id, pass;
	SQLINTEGER u_hp;
	// 체력, 오브젝트 타입, 경험치, 레벨, 마나, 공격력, 좌표

	SQLBindCol(hstmt, 1, SQL_C_CHAR, &AccountId, sizeof(AccountId), &id);
	SQLBindCol(hstmt, 2, SQL_C_CHAR, &AccouuntPass, sizeof(AccouuntPass), &pass);
	
	retcode = SQLFetch(hstmt);

	SQLFreeStmt(hstmt, SQL_UNBIND);
	
	// Delete garbage value - Do Bind ID and pass, value is stored garbage value
	for (int i = Cid.length(); i < 10; ++i)
		AccountId[i] = NULL;
	for (int i = Cpass.length(); i < 10; ++i)
		AccouuntPass[i] = NULL;

	if (string(AccountId) == Cid && string(AccouuntPass) == Cpass) return true;
	return false;
}

bool CDBConnector::RetrieveResultLogin(int uid) {
	SQLINTEGER u_hp, u_oType, u_exp, u_level, u_mp, u_atk, u_x, u_y;
	SQLLEN l_hp, l_oType, l_exp, l_level, l_mp, l_atk, l_x, l_y;

	SQLBindCol(hstmt, 1, SQL_C_LONG, &u_atk, 10, &l_atk);
	SQLBindCol(hstmt, 2, SQL_C_LONG, &u_hp, 10, &l_hp);
	SQLBindCol(hstmt, 3, SQL_C_LONG, &u_mp, 10, &l_mp);
	SQLBindCol(hstmt, 4, SQL_C_LONG, &u_level, 10, &l_level);
	SQLBindCol(hstmt, 5, SQL_C_LONG, &u_exp, 10, &l_exp);
	SQLBindCol(hstmt, 6, SQL_C_LONG, &u_x, 10, &l_x);
	SQLBindCol(hstmt, 7, SQL_C_LONG, &u_y, 10, &l_y);
	SQLBindCol(hstmt, 8, SQL_C_LONG, &u_oType, 10, &l_oType);

	retcode = SQLFetch(hstmt);

	SQLFreeStmt(hstmt, SQL_UNBIND);

	if (retcode == SQL_SUCCESS) {
		if (g_player[uid] == NULL) g_player[uid] = new CPlayer;
		g_player[uid]->Initialize(u_hp, u_oType, u_exp, u_level, u_mp, u_atk, u_x, u_y);
		cout << "Load Player's Data Success!" << endl;
		return true;
	}
	cout << "Login Player's Data Fail!" << endl;
	return false;
}

void CDBConnector::DisconnectDataSource() {
	if (hstmt) {
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
		hstmt = NULL;
	}

	SQLDisconnect(hdbc);

	if (hdbc) {
		SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
		hdbc = NULL;
	}

	if (henv) {
		SQLFreeHandle(SQL_HANDLE_ENV, henv);
		henv = NULL;
	}
}

void DB_Thread() {
	CDBConnector* dbc = new CDBConnector;
	dbc->AllocateHandle();
	dbc->ConnectDataSource();
	int retcode;

	while (true) {
		this_thread::sleep_for(1ms);
		while (true) {
			quaryLock.lock();
			if (quaryQueue.empty()) {
				quaryLock.unlock();
				break;
			}

			DB_EVENT ev = quaryQueue.front();
			quaryQueue.pop();
			quaryLock.unlock();

			switch (ev.ev_id) {
			case EV_LOGIN: {
				string sql = "EXEC Login_User " + (string)ev.name + ", " + (string)ev.pass;
				dbc->ExcuteStatementDirect((SQLCHAR*)sql.c_str());
				dbc->RetrieveResultLogin(ev.user_id);
			}break;
			case EV_SIGN: {
				string sql = "EXEC Sign_User " + (string)ev.name + ", " + (string)ev.pass;
				retcode = dbc->ExcuteStatementDirect((SQLCHAR*)sql.c_str());
				if (retcode == SQL_SUCCESS) {
					cout << "Sign Up Success!";
					DB_EVENT new_ev{ev.user_id, EV_LOGIN};
					strcpy_s(new_ev.name, ev.name);
					quaryQueue.push(new_ev);
				}
			}break;
			case EV_UPDATE: {
				string sql = "EXEC Update_User " + (string)ev.name;
				dbc->ExcuteStatementDirect((SQLCHAR*)sql.c_str());
			}break;
			}
		}

	}
	dbc->DisconnectDataSource();
	delete dbc;
	dbc = NULL;
}

void AddQuary(int uid, int eid, const char name[], const char pass[]) {
	DB_EVENT ev{ uid, eid};
	strcpy_s(ev.name, name);
	strcpy_s(ev.pass, pass);
	quaryLock.lock();
	quaryQueue.push(ev);
	quaryLock.unlock();
}
