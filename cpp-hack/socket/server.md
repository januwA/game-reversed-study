```cpp
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <WinSock2.h>
#include <Windows.h>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

const char* ServerIP = "127.0.0.1";
const DWORD ServerPort = 5000;

SOCKET ListenSocket = INVALID_SOCKET;

BOOL SOCKET_Select(SOCKET hSocket, int nTimeout, BOOL bRead)
{
	fd_set fdSet;
	timeval tv;
	FD_ZERO(&fdSet);
	FD_SET(hSocket, &fdSet);
	tv.tv_sec = 0;
	tv.tv_usec = nTimeout > 1000 ? 1000 : nTimeout;

	int iRet = 0;
	if (bRead)
	{
		iRet = select(0, &fdSet, NULL, NULL, &tv);
	}
	else
	{
		iRet = select(0, NULL, &fdSet, NULL, &tv);
	}

	if (iRet <= 0)
	{
		return FALSE;
	}
	else if (FD_ISSET(hSocket, &fdSet))
	{
		return TRUE;
	}

	return FALSE;
}

DWORD WINAPI tcpThread(LPVOID pParam)
{
	int iResult;
	WSADATA wsaData;
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		std::cout << "WSAStartup failed: " << iResult << std::endl;
		return 0;
	}

	ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (ListenSocket == INVALID_SOCKET)
	{
		printf("创建socket失败: %d\n", WSAGetLastError());
		return 0;
	}

	sockaddr_in   service;
	service.sin_family = AF_INET;
	service.sin_port = htons(ServerPort);
	service.sin_addr.s_addr = inet_addr(ServerIP);

	// 绑定端口
	if (bind(ListenSocket, (SOCKADDR*)&service, sizeof(service)) == SOCKET_ERROR)
	{
		printf("绑定端口失败\n");
		return 0;
	}

	// 监听端口
	if (listen(ListenSocket, 5) == SOCKET_ERROR)
	{
		printf("监听端口失败\n");
		return 0;
	}

	printf("Server: %s:%d\n", ServerIP, ServerPort);
	while (true)
	{
		if (SOCKET_Select(ListenSocket, 100, TRUE))
		{
			// 监听客户端信息
			sockaddr_in clientAddr;
			int iLen = sizeof(clientAddr);
			SOCKET accSock = accept(ListenSocket, (SOCKADDR*)&clientAddr, &iLen);
			if (accSock == INVALID_SOCKET)
			{
				printf("接收客户端失败\n");
				continue;
			}

			printf("Client IP: %s\n", inet_ntoa(clientAddr.sin_addr));
			char reqStr[1024];
			int reqDataLength = recv(accSock, reqStr, sizeof(reqStr), 0);
			if (reqDataLength > 0)
			{
				printf("Client Data: %s\n", reqStr);
			}
			else 
			{
				printf("Client not send data\n");
			}

			const char retData[1024] = "return msg";
			send(accSock, retData, sizeof(retData), 0);
			Sleep(100);
		}
	}
	
}

int main()
{
	HANDLE hThread = CreateThread(0, 0, tcpThread, 0, 0, 0);

	getchar();
	printf("close server\n");
	if (hThread) CloseHandle(hThread);
	if (ListenSocket != INVALID_SOCKET) closesocket(ListenSocket);
	return 0;
}
```