#include "pch.h"
#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include <Psapi.h>

using namespace std;

int WINAPI Mythread(HMODULE hModule)
{
	AllocConsole();
	FILE* _f;
	freopen_s(&_f, "CONOUT$", "w", stdout);
	cout << "INJECT OK" << endl;
	
	while (true)
	{
		if (GetAsyncKeyState(VK_F12) & 1) break;
	}

	fclose(_f);
	FreeConsole();
	FreeLibraryAndExitThread(hModule, 0);
	return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		CloseHandle(CreateThread(0, 0, (LPTHREAD_START_ROUTINE)Mythread, hModule, 0, 0));
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

