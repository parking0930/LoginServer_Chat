#pragma once
#include <unordered_map>
#include "CNetServer.h"
#define dfNETWORK_PACKET_RECV_TIMEOUT	40000

using namespace std;

class LoginServer :public CNetServer
{
private:
	class Character
	{
	public:
		static Character* Alloc();
		static void	Free(Character* pCharacter);
	private:
		Character();
	public:
		unsigned __int64		_sessionID;
		INT64					_accountNo;
		char					_sessionKey[65];
		ULONGLONG				_lastRecvTime;
	private:
		friend class MemoryPoolTLS<Character>;
		static MemoryPoolTLS<Character> poolManager;
	};
public:
	LoginServer();
	~LoginServer();
	bool OnConnectionRequest(wchar_t* ip, unsigned short port);
	void OnClientJoin(unsigned __int64 sessionID);
	void OnClientLeave(unsigned __int64 sessionID);
	void OnRecv(unsigned __int64 sessionID, CNetPacket* pPacket);
	void OnError(int errorcode, wchar_t* str);
	void OnStart();
	void OnStop();
	int GetAcceptTotal();
	int GetLoginSuccessTPS();
private:
	static unsigned WINAPI stContentsMonitorThread(LPVOID lpParam);
	void ContentsMonitorThread();
	void JobHandler(Character* pCharacter, CNetPacket* pPacket);
	void Login(Character* pCharacter, CNetPacket* pPacket);
	Character* FindCharacter(unsigned __int64 sessionID);
private:
	INT _acceptTotal;
	unordered_map<UINT64, Character*> characterMap;
	SRWLOCK charMapLock;
	HANDLE _hMonitorThread;
	HANDLE _hContentsEvent;
	UINT	_loginSuccessCnt;
	UINT	_loginSuccessTPS;
	DWORD	_tlsIdxDB;
	DWORD	_tlsIdxRedis;
	volatile bool _isServerOff;
};