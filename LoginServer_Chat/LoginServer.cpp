#pragma comment (lib, "cpp_redis.lib")
#pragma comment (lib, "tacopie.lib")
#pragma comment (lib, "ws2_32.lib")
#include <cpp_redis/cpp_redis>
#include "LoginServer.h"
#include "CommonProtocol.h"
#include "DBConnector.h"

MemoryPoolTLS<LoginServer::Character> LoginServer::Character::poolManager(10000, POOL_MAX_ALLOC, true);

LoginServer::Character::Character() : _accountNo(0), _sessionKey{ 0, }, _sessionID(0), _lastRecvTime(0){}

LoginServer::Character* LoginServer::Character::Alloc()
{
	return poolManager.Alloc();
}

void LoginServer::Character::Free(Character* pCharacter)
{
	poolManager.Free(pCharacter);
}


LoginServer::LoginServer():_acceptTotal(0),_loginSuccessCnt(0), _loginSuccessTPS(0), _hContentsEvent(NULL),
						_hMonitorThread(NULL), _isServerOff(false)
{
	InitializeSRWLock(&charMapLock);
	_acceptTotal = 0;
	_loginSuccessCnt = 0;
	_loginSuccessTPS = 0;
	_hContentsEvent = NULL;
	_hMonitorThread = NULL;
	_tlsIdxDB = 0;
	_tlsIdxRedis = 0;
	//_hTimeoutThread = (HANDLE)_beginthreadex(NULL, 0, stTimeoutThread, (LPVOID)this, NULL, NULL);
}

LoginServer::~LoginServer()
{
	_isServerOff = true;
	if (_hMonitorThread != NULL)
	{
		if (WaitForSingleObject(_hMonitorThread, 5000) == WAIT_TIMEOUT)
			TerminateThread(_hMonitorThread, 1);
		CloseHandle(_hMonitorThread);

	}
	if (_hContentsEvent != NULL)
	{
		CloseHandle(_hContentsEvent);
	}
}

bool LoginServer::OnConnectionRequest(wchar_t* ip, unsigned short port) { return true; }
void LoginServer::OnError(int errorcode, wchar_t* str) {}

void LoginServer::OnStart()
{
	InitializeSRWLock(&charMapLock);
	_acceptTotal = 0;
	_loginSuccessCnt = 0;
	_loginSuccessTPS = 0;
	_hContentsEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	_hMonitorThread = (HANDLE)_beginthreadex(NULL, 0, stContentsMonitorThread, (LPVOID)this, NULL, NULL);
	_tlsIdxDB = TlsAlloc();
	_tlsIdxRedis = TlsAlloc();
}

void LoginServer::OnStop()
{
	_isServerOff = true;
	if (_hMonitorThread != NULL)
	{
		if (WaitForSingleObject(_hMonitorThread, 5000) == WAIT_TIMEOUT)
			TerminateThread(_hMonitorThread, 1);
		CloseHandle(_hMonitorThread);
		_hMonitorThread = NULL;

	}
	if (_hContentsEvent != NULL)
	{
		CloseHandle(_hContentsEvent);
		_hContentsEvent = NULL;
	}
}

void LoginServer::OnClientJoin(unsigned __int64 sessionID)
{
	++_acceptTotal;
	// Create character
	Character* newCharacter = Character::Alloc();
	newCharacter->_accountNo = -1;
	newCharacter->_sessionID = sessionID;
	newCharacter->_lastRecvTime = GetTickCount64();

	AcquireSRWLockExclusive(&charMapLock);
	characterMap.insert(pair<UINT64, Character*>(sessionID, newCharacter));
	ReleaseSRWLockExclusive(&charMapLock);
}

void LoginServer::OnClientLeave(unsigned __int64 sessionID)
{
	AcquireSRWLockExclusive(&charMapLock);
	auto it = characterMap.find(sessionID);
	if (it != characterMap.end())
	{
		Character::Free(it->second);
		characterMap.erase(it);
	}
	ReleaseSRWLockExclusive(&charMapLock);
}

void LoginServer::OnRecv(unsigned __int64 sessionID, CNetPacket* pPacket)
{
	Character* pCharacter = FindCharacter(sessionID);
	if (pCharacter == nullptr)
	{
		Disconnect(sessionID);
		return;
	}
	pCharacter->_lastRecvTime = GetTickCount64();
	JobHandler(pCharacter, pPacket);
}

void LoginServer::JobHandler(Character* pCharacter, CNetPacket* pPacket)
{
	WORD type;
	*pPacket >> type;
	switch (type)
	{
	case en_PACKET_CS_LOGIN_REQ_LOGIN:
		Login(pCharacter, pPacket);
		break;
	default:
		break;
	}
}

void LoginServer::Login(Character* pCharacter, CNetPacket* pPacket)
{
	*pPacket >> pCharacter->_accountNo;
	pPacket->GetData(pCharacter->_sessionKey, 64);
	pCharacter->_sessionKey[64] = '\0';

	DBConnector* conn = reinterpret_cast<DBConnector*>(TlsGetValue(_tlsIdxDB));
	if (conn == nullptr)
	{
		conn = new DBConnector("127.0.0.1", 3306, "root", "wogus@4735", "accountdb");
		TlsSetValue(_tlsIdxDB, conn);
	}

	BYTE isSuccess = 0;
	do
	{
		if (!conn->ReqQuery("select count(*) from account where accountno=%lld", pCharacter->_accountNo))
			break;

		MYSQL_ROW sql_row = conn->FetchRow();
		if (sql_row[0][0] == '0')
			break;
		conn->FreeQueryResult();
		isSuccess = 1;
	} while (0);

	// Redis
	if (isSuccess)
	{
		char strAccountNo[10];
		sprintf_s(strAccountNo, "%lld", pCharacter->_accountNo);
		cpp_redis::client* client = reinterpret_cast<cpp_redis::client*>(TlsGetValue(_tlsIdxRedis));
		if (client == nullptr)
		{
			client = new cpp_redis::client;
			client->connect();
			TlsSetValue(_tlsIdxRedis, client);
		}
		client->set_advanced(strAccountNo, pCharacter->_sessionKey, true, 10000);
		client->sync_commit();
	}

	WCHAR dummyID[20] = L"0";
	WCHAR dummyNick[20] = L"0";
	WCHAR GameServerIP[16] = L"127.0.0.1";
	USHORT GameServerPort = 20002;
	WCHAR ChatServerIP[16] = L"127.0.0.1";
	USHORT ChatServerPort = 20000;

	CNetPacket* pSndPkt = CNetPacket::Alloc();
	*pSndPkt << (WORD)en_PACKET_CS_LOGIN_RES_LOGIN;
	*pSndPkt << pCharacter->_accountNo;
	*pSndPkt << isSuccess;
	pSndPkt->SetData((char*)dummyID, sizeof(dummyID));
	pSndPkt->SetData((char*)dummyNick, sizeof(dummyNick));
	pSndPkt->SetData((char*)GameServerIP, sizeof(GameServerIP));
	pSndPkt->SetData((char*)&GameServerPort, sizeof(GameServerPort));
	pSndPkt->SetData((char*)ChatServerIP, sizeof(ChatServerIP));
	pSndPkt->SetData((char*)&ChatServerPort, sizeof(ChatServerPort));
	SendPacket(pCharacter->_sessionID, pSndPkt);
	CNetPacket::Free(pSndPkt);
	InterlockedIncrement((LONG*)&_loginSuccessCnt);
}

LoginServer::Character* LoginServer::FindCharacter(unsigned __int64 sessionID)
{
	AcquireSRWLockShared(&charMapLock);
	unordered_map<UINT64, Character*>::iterator it = characterMap.find(sessionID);
	if (it == characterMap.end())
	{
		ReleaseSRWLockShared(&charMapLock);
		return nullptr;
	}
	else
	{
		ReleaseSRWLockShared(&charMapLock);
		return it->second;
	}
}

unsigned WINAPI LoginServer::stContentsMonitorThread(LPVOID lpParam)
{
	LoginServer* pContents = (LoginServer*)lpParam;
	pContents->ContentsMonitorThread();
	return 0;
}

void LoginServer::ContentsMonitorThread()
{
	while (!_isServerOff)
	{
		_loginSuccessTPS = InterlockedExchange((LONG*)&_loginSuccessCnt, 0);
		Sleep(1000);
	}
}

int LoginServer::GetAcceptTotal()
{
	return _acceptTotal;
}

int LoginServer::GetLoginSuccessTPS()
{
	return _loginSuccessTPS;
}