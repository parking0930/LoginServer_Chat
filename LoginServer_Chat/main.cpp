#include "LoginServer.h"

int main()
{
	LoginServer server;
	bool svStatus = server.Start(NULL, 20001, 2, 10, false, 20000);
	// Logic
	while (svStatus)
	{
		//int totalCnt = packetpool.totalCnt;
		//int useCnt = packetpool.useCnt;
		wprintf(L"Now connect session: %u\n", server.GetSessionCount());
		wprintf(L"AcceptTotal: %d\n", server.GetAcceptTotal());
		//wprintf(L"MemoryPool(CPacket) Total: %d / Use: %d / Remain: %d\n", totalCnt, useCnt, totalCnt - useCnt);
		wprintf(L"Accept TPS: %d\n", server.getAcceptTPS());
		wprintf(L"SendPacket TPS: %d\n", server.getSendMessageTPS());
		wprintf(L"RecvPacket TPS: %d\n", server.getRecvMessageTPS());
		wprintf(L"LoginSuccess TPS: %d\n", server.GetLoginSuccessTPS());
		printf("\n");
		Sleep(1000);
	}

	server.Stop();
	return 0;
}