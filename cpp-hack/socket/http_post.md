```cpp
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <WinSock2.h>
#include <Windows.h>
#include <string>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

const char* ServerIP = "127.0.0.1";
const DWORD ServerPort = 3000;


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

int main()
{
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		std::cout << "WSAStartup failed: " << iResult << std::endl;
		return 0;
	}

	// 创建socket
	SOCKET ConnectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (ConnectSocket == INVALID_SOCKET)
	{
		printf("创建socket失败\n");
		return 0;
	}

	sockaddr_in   service;
	service.sin_family = AF_INET;
	service.sin_port = htons(ServerPort);
	service.sin_addr.s_addr = inet_addr(ServerIP);

	if (connect(ConnectSocket, (SOCKADDR*)&service, sizeof(service)) == SOCKET_ERROR)
	{
		closesocket(ConnectSocket);
		printf("连接失败\n");
		return 0;
	}
	
	string postData = "name=ajanuw&age=14";
	string msg = "POST /api/post HTTP/1.1\r\n";
  msg += "Accept: */*\r\n";
	msg += "Accept-Encoding: gzip\r\n";
  msg += "Content-Type: application/x-www-form-urlencoded\r\n";
  msg += "Content-Length: " + to_string(postData.length()) + "\r\n";
	msg += "Connection: Keep-Alive\r\n";
	msg += "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/84.0.4147.135 Safari/537.36\r\n";
	msg += "\r\n";
	msg += postData;

	// printf("%s\n", msg.c_str());

	if (send(ConnectSocket, msg.c_str(), msg.size(), 0) == SOCKET_ERROR)
	{
		closesocket(ConnectSocket);
		printf("发送消息失败\n");
		return 0;
	}

	while (!GetAsyncKeyState(VK_F4))
	{
		if (SOCKET_Select(ConnectSocket, 100, TRUE))
		{
			char resStr[1024];
			int resDataLength = recv(ConnectSocket, resStr, sizeof(resStr), 0);
			if (resDataLength > 0)
			{
				printf("Res Data: %s\n", resStr);
			}
			Sleep(20);
		}
	}

	closesocket(ConnectSocket);
	return 0;
}
```