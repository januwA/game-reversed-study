- https://www.youtube.com/watch?v=wiX5LmdD5yk&list=PL2C03D3BB7FAF2EA0&index=2


## 读取值，和写入值
```
#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include <vector>


// 获取进程名的pid
DWORD getPID(const wchar_t* name)
{
	DWORD pid = 0;
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnap != INVALID_HANDLE_VALUE)
	{
		PROCESSENTRY32 pe;
		pe.dwSize = sizeof(pe);
		if (Process32First(hSnap, &pe))
		{
			do {
				if (!_wcsicmp(pe.szExeFile, name)) {
					pid = pe.th32ProcessID;
					break;
				}
			} while (Process32Next(hSnap, &pe));
		}
	}
	CloseHandle(hSnap);
	return pid;
}

// 获取模块基址
uintptr_t getModuleBaseAddress(DWORD pid, const wchar_t* modName)
{
	uintptr_t modBaseAddr = 0;
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);

	if (hSnap != INVALID_HANDLE_VALUE)
	{
		MODULEENTRY32 me;
		me.dwSize = sizeof(me);
		if (Module32First(hSnap, &me))
		{
			do {
				if (!_wcsicmp(me.szModule, modName)) {
					modBaseAddr = (uintptr_t)me.modBaseAddr;
					break;
				}
			} while (Module32Next(hSnap, &me));
		}
	}
	CloseHandle(hSnap);
	return modBaseAddr;
}

uintptr_t readIntger(HANDLE hProc, uintptr_t ptr, std::vector<unsigned int> offsets)
{
	uintptr_t addr = ptr;
	for (unsigned int i = 0; i < offsets.size(); ++i)
	{
		// 进程句柄, 内存地址(从这里读), 值引用(存读取得值)，值的字节大小(读多大), null
		ReadProcessMemory(hProc, (BYTE*)addr, &addr, sizeof(addr), 0);
		addr += offsets[i];
	}
	return addr;
}

int main()
{

	// 地址: [game.exe+009E820C]+338

	 // 1 获取进程pid
	DWORD pid = getPID(L"game.exe");
	printf_s("pid: %d\n", pid);

	// 2 打开进程
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	if (hProcess == NULL) return 0; // 打开进程失败

	// 3 获取模块地址
	uintptr_t moduleBaseAddress = getModuleBaseAddress(pid, L"game.exe");
	// 00905A4D
	printf_s("module address: %x\n", moduleBaseAddress);

	uintptr_t r = 0;
	ReadProcessMemory(hProcess, (LPCVOID)(moduleBaseAddress + 0x9E820C), &r, sizeof(uintptr_t), 0);
	ReadProcessMemory(hProcess, (LPCVOID)(r + 0x338), &r, 4, 0);
	printf_s("value: %d\n", r);


	// 写入新的值
	uintptr_t newValue = 20;
	ReadProcessMemory(hProcess, (LPCVOID)(moduleBaseAddress + 0x9E820C), &r, sizeof(r), 0);
	WriteProcessMemory(hProcess, (LPVOID)(r + 0x338), (LPCVOID)&newValue, sizeof(newValue), 0);

	CloseHandle(hProcess);
	return 0;
}
```

## 类似CE的read/writeIntger函数
```
#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include <vector>
#include <regex>
#include <sstream>
#include <string>

// global
DWORD pid = 0;
HANDLE hProcess = 0;

// 获取进程名的pid
DWORD getPID(const wchar_t* name)
{
	DWORD pid = 0;
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnap != INVALID_HANDLE_VALUE)
	{
		PROCESSENTRY32 pe;
		pe.dwSize = sizeof(pe);
		if (Process32First(hSnap, &pe))
		{
			do {
				if (!_wcsicmp(pe.szExeFile, name)) {
					pid = pe.th32ProcessID;
					break;
				}
			} while (Process32Next(hSnap, &pe));
		}
	}
	CloseHandle(hSnap);
	return pid;
}

// 获取模块基址
uintptr_t getModuleBaseAddress(DWORD pid, const wchar_t* modName)
{
	uintptr_t modBaseAddr = 0;
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);

	if (hSnap != INVALID_HANDLE_VALUE)
	{
		MODULEENTRY32 me;
		me.dwSize = sizeof(me);
		if (Module32First(hSnap, &me))
		{
			do {
				if (!_wcsicmp(me.szModule, modName)) {
					modBaseAddr = (uintptr_t)me.modBaseAddr;
					break;
				}
			} while (Module32Next(hSnap, &me));
		}
	}
	CloseHandle(hSnap);
	return modBaseAddr;
}

std::string replaceString(std::string origenString, std::string replaceString, std::string newValue)
{
	int startIndex = origenString.find(replaceString);
	int endIndex = replaceString.size();
	return origenString.replace(startIndex - 1, endIndex + 2, newValue);
}

uintptr_t hexStr2Hex(std::string hexStr)
{
	uintptr_t r;
	std::stringstream(hexStr) >> std::hex >> r;
	return r;
}

struct SplitListItem
{
	std::string key;
	std::string value;
};

std::vector<SplitListItem> splitString(std::string origenString, std::regex pattern)
{
	std::smatch result;
	std::string::const_iterator iterStart = origenString.begin();
	std::string::const_iterator iterEnd = origenString.end();


	std::vector<std::string> splitList = {};
	std::vector<std::string> splitKeys = {};
	std::vector<SplitListItem> resultSplitList = {};

	while (regex_search(iterStart, iterEnd, result, pattern))
	{
		splitList.emplace_back(iterStart, result[0].first);
		splitKeys.push_back(result[0].str());
		iterStart = result[0].second;
	}
	splitList.emplace_back(iterStart, iterEnd);


	for (size_t i = 0; i < splitList.size(); i++)
	{
		resultSplitList.push_back(SplitListItem{ i > 0 ? splitKeys[i - 1] : "",  splitList[i] });
	}
	return resultSplitList;
}

uintptr_t getOffsetsAddress(std::string address, uintptr_t nextValue = 0)
{
	
	std::string str = std::regex_replace(address, (std::regex)"\\s", "") ;
	std::smatch result;
	std::regex pattern(".*\\[([^\\[\\]]+)\\].*");
	std::regex_match(str, result, pattern);
	if (result.size() == 0)
	{
		if (str.size() == 0) {
			return nextValue;
		}

		std::vector<SplitListItem>  r = splitString(str, (std::regex)"[+-]");

		uintptr_t a = hexStr2Hex(r[0].value);
		if (a == 0 && r[0].value != "0")
		{
			// 符号
			a = getModuleBaseAddress(
				pid,
				std::wstring(r[0].value.begin(), r[0].value.end()).c_str()
			);
		}
		uintptr_t b = hexStr2Hex(r[1].value);

		if (r[1].key == "+") a += b;
		if (r[1].key == "-") a -= b;
		return a;
	}



	std::vector<SplitListItem>  r = splitString(result[1], (std::regex)"[+-]");
	uintptr_t data = 0;
	for (size_t i = 0; i < r.size(); i++)
	{

		uintptr_t v = hexStr2Hex(r[i].value);

		if (v == 0 && r[i].value != "0")
		{
			// 符号
			data += getModuleBaseAddress(
				pid,
				std::wstring(r[i].value.begin(), r[i].value.end()).c_str()
			);
		}
		else
		{
			if (r[i].key == "+") data += v;
			if (r[i].key == "-") data -= v;
			ReadProcessMemory(hProcess, (LPCVOID)data, &data, 4, 0);
		}
	}

	std::stringstream hexData;
	hexData << std::hex << data;
	std::string newOrigenString = replaceString(str, result[1], hexData.str());
	return getOffsetsAddress(newOrigenString, data);
}

uintptr_t readIntger(std::string address)
{
	uintptr_t r = getOffsetsAddress(address);
	if (r == 0) return 0;
	ReadProcessMemory(hProcess, (LPCVOID)r, &r, 4, 0);
	return r;
}

uintptr_t writeIntger(std::string address, uintptr_t newInt)
{
	uintptr_t r = getOffsetsAddress(address);
	if (r == 0) return 0;
	WriteProcessMemory(hProcess, (LPVOID)r, (LPCVOID)&newInt, 4, 0);
	return r;
}

int main()
{

	// 地址: [game.exe+009E820C]+338

	std::string mainname = "game.exe";

	pid = getPID(std::wstring(mainname.begin(), mainname.end()).c_str());


	hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	if (hProcess == NULL) return 0;


	std::cout << readIntger("game.exe+009E820C") << std::endl;
	std::cout << readIntger("[game.exe + 009E820C] + 338") << std::endl;

	writeIntger("[game.exe+ 009E820C] + 338", 20);
	

	CloseHandle(hProcess);
	return 0;
}
```


## DLL
使用dll你可以向开发人员一样编写程序

```
// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"
#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include <vector>
#include <regex>
#include <sstream>
#include <string>

// 获取模块基址
uintptr_t getModuleBaseAddress(const wchar_t* modName)
{
	uintptr_t modBaseAddr = 0;
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, 0);

	if (hSnap != INVALID_HANDLE_VALUE)
	{
		MODULEENTRY32 me;
		me.dwSize = sizeof(me);
		if (Module32First(hSnap, &me))
		{
			do {
				if (!_wcsicmp(me.szModule, modName)) {
					modBaseAddr = (uintptr_t)me.modBaseAddr;
					break;
				}
			} while (Module32Next(hSnap, &me));
		}
	}
	CloseHandle(hSnap);
	return modBaseAddr;
}

int Mythread(HMODULE hModule)
{
	// 创建控制台
	AllocConsole();
	FILE* f;
	freopen_s(&f, "CONOUT$", "w", stdout);
	std::cout << "run...";

	uintptr_t miduleAddress = getModuleBaseAddress(L"game.exe");
	std::cout << "miduleAddress: " << std::hex << miduleAddress << std::endl;

	// hack loop
	while (true)
	{
		// 按下f2，修改值
		if (GetAsyncKeyState(VK_F2) & 1)
		{
			// [game.exe+009E820C]+338
			// addr = *(uintptr_t*)addr => mov addr [addr]

			// lea a, [miduleAddress + 0x009E820C]
			uintptr_t* a = (uintptr_t*)(miduleAddress + 0x009E820C);

			// mov eax, [miduleAddress + 0x009E820C]
			std::cout << "a value: " << std::hex << *a << std::endl;

			if (a)
			{
				std::cout << "old value: " << std::hex << *(int*)(*a + 0x338) << std::endl;
				// mov [eax+0x338], #10
				*(int*)(*a + 0x338) = 10; // 设置新值
			}
			break;
		}
		
		// 修改字节集
		if (GetAsyncKeyState(VK_F3) & 1)
		{
			BYTE* address = (BYTE*)(miduleAddress + 0x3B79F7);

			// code: F3 0F 11 84 90 B8 02 00 00
			BYTE* bytes = (BYTE*)"\x90\x90\x90\x90\x90\x90\x90\x90\x90";
			int size = 9;

			// 保存旧的保护值
			DWORD oldprotect;

			// 要更改任何进程的访问保护
			VirtualProtect(address, size, PAGE_EXECUTE_READWRITE, &oldprotect);
			
			// 从缓冲区复制字符
			// 设置新的，不同的字节集
			_memccpy(address, bytes, 0, size);

			// 将缓冲区设置为指定字符
			// 如果要设置相同的字节用这个方便点
			// memset(address, 0x90, size);

			VirtualProtect(address, size, oldprotect, &oldprotect);
			break;
		}

		if (GetAsyncKeyState(VK_F12) & 1)
		{
			// 释放资源
			fclose(f);
			FreeConsole();
			FreeLibraryAndExitThread(hModule, 0);
			break;
		}

		Sleep(5);
	}
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
```


## AAScript to c++

- https://www.youtube.com/watch?v=jTl3MFVKSUM


ce的aa脚本是一种绕行/挂钩技术

源地址为`00433F86: mov [edi+00005578],esi`，将这里绕行到我们定义的函数，在最后jmp回去

AAScript:
```
[ENABLE]
aobscanmodule(INJECT,PlantsVsZombies.exe,89 B7 78 55 00 00)
alloc(newmem,$1000)

label(return)

newmem:
  add esi,#100
  mov [edi+00005578],esi
  jmp return

INJECT:
  jmp newmem
  nop
return:
registersymbol(INJECT)

[DISABLE]

INJECT:
  db 89 B7 78 55 00 00

unregistersymbol(INJECT)
dealloc(newmem)

{
00433F84: 2B F3                          -  sub esi,ebx
// ---------- INJECTING HERE ----------
00433F86: 89 B7 78 55 00 00              -  mov [edi+00005578],esi
// ---------- DONE INJECTING  ----------
00433F8C: B0 01                          -  mov al,01
}
```

cpp DLL:
```
#include "pch.h"
#include <iostream>
#include <Windows.h>


bool handleHook(void * oldHook, void * newFunc, int len)
{
	if (len < 5) return false;

	// 更改访问保护
	DWORD oldProc;
	VirtualProtect(oldHook, len, PAGE_EXECUTE_READWRITE, &oldProc);

	// 先将旧的字节集设置为nop
	memset(oldHook, 0x90, len);

	// 计算新的字节集
	// 跳转目标地址 - 当前指令地址 - 5 = 字节集
	DWORD relativeAddress = ((DWORD)newFunc - (DWORD)oldHook - 5);
	
	// 设置jmp指令
	*(BYTE*)oldHook = 0xE9;
	*(DWORD*)((DWORD)oldHook + 1) = relativeAddress;
	
	// 修改后还原访问保护
	VirtualProtect(oldHook, len, oldProc, &oldProc);

	return true;
}

DWORD returnAddress;

// 定义新的处理函数
void __declspec(naked) myNewFunc() {

	//newmem:
	__asm {
		add esi, 0x64
		mov[edi + 0x00005578], esi
		jmp [returnAddress]
	}
}

int Mythread(HMODULE hModule)
{
	DWORD oldHookAddress = 0x00433F86;
	int oldHookAddressLen = 6;

	// 0x433F8C
	returnAddress = oldHookAddress + oldHookAddressLen;
	handleHook((void*)oldHookAddress, myNewFunc, oldHookAddressLen);

	while (true)
	{
		if (GetAsyncKeyState((VK_F2))) break;
		Sleep(10);
	}

	// 脱钩
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
```


## module scan

- https://www.youtube.com/watch?v=5M2rjjdX6DQ
- https://www.youtube.com/watch?v=S_SR5l_hquw

aa脚本：
```
[ENABLE]

aobscanmodule(INJECT,game2.exe,A3 24 37 4B 00)
alloc(newmem,$1000)

label(code)
label(return)

newmem:
code:
  mov [004B3724],eax
  jmp return

INJECT:
  jmp newmem
return:
registersymbol(INJECT)

[DISABLE]

INJECT:
  db A3 24 37 4B 00

unregistersymbol(INJECT)
dealloc(newmem)

{
00401570: E8 97 FA FF FF        -  call game2.exe+100C
// ---------- INJECTING HERE ----------
00401575: A3 24 37 4B 00        -  mov [game2.exe+B3724],eax
// ---------- DONE INJECTING  ----------
0040157A: 68 01 03 00 80        -  push 80000301
}
```

c++
```
#include "pch.h"
#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include <Psapi.h>


MODULEINFO GetModuleInfo(const wchar_t* mName)
{
	MODULEINFO mInfo = { 0 };
	HMODULE hModule = GetModuleHandleW(mName);
	if (hModule == 0) return mInfo;

	// 在MODULEINFO结构中检索有关指定模块的信息
	GetModuleInformation(GetCurrentProcess(), hModule, &mInfo, sizeof(MODULEINFO));
	return mInfo;
}

DWORD ModuleScan(const wchar_t* moduleName, BYTE* bytes, const wchar_t* mask)
{
	// https://docs.microsoft.com/en-us/windows/win32/api/psapi/ns-psapi-moduleinfo
	MODULEINFO mInfo = GetModuleInfo(moduleName);

	// 起始位置
	uintptr_t base = (uintptr_t)mInfo.lpBaseOfDll;

	// 模块大小
	uintptr_t size = (uintptr_t)mInfo.SizeOfImage;

	int patternLen = wcslen(mask);
	wchar_t anyByte{ L'?' };

	for (size_t i = 0; i < size - patternLen; i++)
	{
		bool found = true;
		for (size_t j = 0; j < patternLen; j++)
		{
			found &= mask[j] == anyByte || bytes[j] == *(BYTE*)(base + i + j);
		}
		
		// return find address start
		if (found) return base + i;
	}

	return 0;
}

int Mythread(HMODULE hModule)
{
	AllocConsole();
	FILE* f;
	freopen_s(&f, "CONOUT$", "w", stdout);


	const wchar_t* moduleName = L"game2.exe";
	BYTE* bytes = (BYTE*)"\xA3\x24\x37\x4B\x00";
	// std::cout << (bytes[0] == 0xA3) << std::endl;
	const wchar_t* mask = L"xxxxx"; // 每一个代表一个字节?代表any

	DWORD address = ModuleScan(moduleName, bytes, mask);

	// 00401575
	std::cout << std::hex << address << std::endl;

	while (true)
	{
		if (GetAsyncKeyState(VK_F12) & 1)
		{
			break;
		}

		Sleep(10);
	}


	fclose(f);
	FreeConsole();
	// 脱钩
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
```

## C++调用游戏函数
- https://www.youtube.com/watch?v=jmgwFpVnRmU
- https://www.youtube.com/watch?v=gZN2damgYHg
- https://www.youtube.com/watch?v=-hwUPT5Gyvc


## 绕行挂钩技术 x86
```
#include "pch.h"
#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>

using namespace std;

uintptr_t GetModuleBaseAddr(const wchar_t* name)
{
	uintptr_t addr = 0;
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, 0);

	if (hSnap != INVALID_HANDLE_VALUE)
	{
		MODULEENTRY32 me;
		me.dwSize = sizeof(me);
		if (Module32First(hSnap, &me))
		{
			do
			{
				if (!_wcsicmp(me.szModule, name))
				{
					addr = (uintptr_t)me.modBaseAddr;
					break;
				}
			} while (Module32Next(hSnap, &me));
		}
	}

	CloseHandle(hSnap);
	return addr;
}

uintptr_t returnAddr = 0;

void __declspec(naked) mymemnew()
{
	__asm {
		inc [esi]
		push edi
		mov edi, [esp + 0x14]
		jmp [returnAddr]
	}
}

int MyThread(HMODULE hModule)
{
	AllocConsole();
	FILE* f;
	freopen_s(&f, "CONOUT$", "w", stdout);
	cout << "INJECT OK" << endl;

	// 1.获取模块地址
	// 2.创建钩子
	// 3.将源地址跳到钩子

	uintptr_t addr = GetModuleBaseAddr(L"ac_client.exe");
	BYTE* address = (BYTE*)(addr + 0x637E9);
	BYTE bytes[] = { 0xFF, 0x0E, 0x57, 0x8B, 0x7C, 0x24, 0x14 };
	unsigned int size = sizeof(bytes);
	returnAddr = (uintptr_t)address + size;
	DWORD jmpBytes = (DWORD)mymemnew - (DWORD)address - 5;

	bool bEnable = false;
	while (true)
	{

		if (GetAsyncKeyState(VK_F2) & 1)
		{
			bEnable = !bEnable;
			DWORD oldProc;
			VirtualProtect(address, size, PAGE_EXECUTE_READWRITE, &oldProc);
			if (bEnable)
			{
				memset(address, 0x90, size);
				*(BYTE*)address = 0xE9;
				*(DWORD*)(address + 1) = jmpBytes;
			}
			else
			{
				_memccpy(address, bytes, 0, size);
			}
			VirtualProtect(address, size, oldProc, 0);
		}

		if( GetAsyncKeyState(VK_F12)&1 )
		{
			break;
		}

		Sleep(20);
	}

	fclose(f);
	FreeConsole();
	FreeLibraryAndExitThread(hModule, 0);
	return 0;
}
```

## 绕行挂钩技术 x64
```
#include "pch.h"
#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include <Psapi.h>

using namespace std;

MODULEINFO GetModuleInfo(const wchar_t* modname)
{
	MODULEINFO mi{ 0 };
	HMODULE hMod = GetModuleHandle(modname);
	GetModuleInformation(GetCurrentProcess(), hMod, &mi, sizeof(mi));
	return mi;
}

int Mythread(HMODULE hModule)
{
	AllocConsole();
	FILE* f;
	freopen_s(&f, "CONOUT$", "w", stdout);
	cout << "INJECT OK" << endl;

	// 获取模块信息
	MODULEINFO mi = GetModuleInfo(L"MonsterHunterWorld.exe");

	// "MonsterHunterWorld.exe"+CE7462A
	BYTE* address = (BYTE*)((uintptr_t)mi.lpBaseOfDll + 0xCE7462A);
	BYTE bytes[] = {
		0xF3, 0x0F, 0x11, 0x41, 0x6C, // movss [rcx+6C],xmm0
	};
	int bytesSize = sizeof(bytes);

	// "MonsterHunterWorld.exe"+CE7462F
	uintptr_t returnAddr = (uintptr_t)address + bytesSize;


	/* newmem */
	BYTE codeBytes[] = {
		0xC7, 0x41, 0x6C, 0x00, 0x00, 0xC8, 0x42, // mov [rcx+6C],42C80000
	};
	int codeSize = sizeof(codeBytes);

	// 申请500字节虚拟空间存代码
	BYTE* newmem = (BYTE*)VirtualAlloc((BYTE*)((uintptr_t)mi.lpBaseOfDll - 0x10000)/* 2-4GB */, 500,
		MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

	/*
	BYTE* newmem2 = (BYTE*)VirtualAlloc((BYTE*)((uintptr_t)newmem - 0x10000), 4,
		MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	*/


	// 将代码复制到newmem
	memcpy_s(newmem, codeSize, codeBytes, codeSize);

	// jmp return
	uintptr_t newmemJmpReturnAddr = ((uintptr_t)newmem + codeSize);

	// 计算jmp字节集，丢弃高位
	DWORD returnBytes = (DWORD)(returnAddr - newmemJmpReturnAddr - 5);

	// jmp "MonsterHunterWorld.exe"+CE7462F
	*(BYTE*)(newmemJmpReturnAddr) = 0xE9; // jmp
	*(DWORD*)(newmemJmpReturnAddr + 1) = returnBytes; // 
	/* newmem */

	DWORD jmpNewmemBytes = (DWORD)((uintptr_t)newmem - (uintptr_t)address - 5);
	bool bEnable = false;
	while (true)
	{
		if (GetAsyncKeyState(VK_F2) & 1)
		{
			bEnable = !bEnable;
			DWORD oldProc;
			VirtualProtect(address, bytesSize, PAGE_EXECUTE_READWRITE, &oldProc);
			if (bEnable)
			{
				cout << "[ENABLE]" << endl;
				memset(address, 0x90, bytesSize);

				// jmp newmem
				*(BYTE*)(address) = 0xE9; // jmp
				*(DWORD*)(address + 1) = jmpNewmemBytes;
			}
			else
			{
				cout << "[DISABLE]" << endl;
				memcpy_s(address, bytesSize, bytes, bytesSize);
			}
			VirtualProtect(address, bytesSize, oldProc, 0);
		}

		if (GetAsyncKeyState(VK_F12) & 1)
		{
			break;
		}
		Sleep(20);
	}
	
	VirtualFree(newmem, 0, MEM_RELEASE);
	fclose(f);
	FreeConsole();
	FreeLibraryAndExitThread(hModule, 0);
	return 0;
}
```


## 钩住opengl32.dll->wglSwapBuffers函数头
- https://guidedhacking.com/threads/how-to-hook-functions-code-detouring-guide.14185/

```
#include "pch.h"
#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>

using namespace std;

typedef BOOL(__stdcall* mywglSwapBuffers)(HDC hDc);
mywglSwapBuffers  owglSwapBuffers;

HMODULE _hModule = 0;
FILE* _f;


// 定义自己的钩子函数
BOOL __stdcall mywglSwapBuffersHook(HDC hDc)
{
		if (GetAsyncKeyState(VK_F2) & 1)
		{
			cout << "F2" << endl;
		}

		/* 脱钩失败
		if (GetAsyncKeyState(VK_F12) & 1)
		{
			fclose(_f);
			FreeConsole();
			FreeLibraryAndExitThread(_hModule, 0);
		}
		*/
	return owglSwapBuffers(hDc);
}

uintptr_t GetModuleBaseAddress(const wchar_t* modName)
{
	uintptr_t modBaseAddr = 0;
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, 0);

	if (hSnap != INVALID_HANDLE_VALUE)
	{
		MODULEENTRY32 me;
		me.dwSize = sizeof(me);
		if (Module32First(hSnap, &me))
		{
			do {
				if (!_wcsicmp(me.szModule, modName)) {
					modBaseAddr = (uintptr_t)me.modBaseAddr;
					break;
				}
			} while (Module32Next(hSnap, &me));
		}
	}
	CloseHandle(hSnap);
	return modBaseAddr;
}


/* 钩住wglSwapBuffers */
bool Detour32(BYTE* origen, BYTE* hook, int len)
{
	if (len < 5) return false;
	DWORD oldProc;
	VirtualProtect(origen, len, PAGE_EXECUTE_READWRITE, &oldProc);
	
	// 将wglSwapBuffers函数开始处设置为
	// jmp mywglSwapBuffersHook
	uintptr_t jmpHookBytes =  hook - origen - 5;
	*origen = 0xE9;
	*(uintptr_t*)(origen + 1) = jmpHookBytes;

	VirtualProtect(origen, len, oldProc, 0);
}

BYTE* TrampHook32(BYTE* origen, BYTE* hook, int len)
{
	if (len < 5) return 0;

	// 申请一块虚拟内存存代码
	BYTE* gateway = (BYTE*)VirtualAlloc(0, len, 
		MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

	// 从origen中拷贝len大小的字节到gateway
	_memccpy(gateway, origen, 0, len);

	// jmp wglSwapBuffers+5
	// uintptr_t returnBytes = (origen+len) - (gateway+len) - 5;
	uintptr_t returnBytes = origen - gateway - 5;
	*(gateway + len) = 0xE9;
	*(uintptr_t*)((uintptr_t)gateway + len + 1) = returnBytes;

	Detour32(origen, hook, len);

	return gateway;
}

int Mythread()
{
	AllocConsole();
	freopen_s(&_f, "CONOUT$", "w", stdout);
	cout << "INJECT OK" << endl;

	// 显示连接DLL函数
	// opengl32.dll导出的wglSwapBuffers函数
	owglSwapBuffers = (mywglSwapBuffers)GetProcAddress(
		GetModuleHandleW(L"opengl32.dll"), "wglSwapBuffers");

	// 将返回的虚拟内存地址给owglSwapBuffers
	// 在mywglSwapBuffersHook中的owglSwapBuffers将执行
	owglSwapBuffers = (mywglSwapBuffers)TrampHook32(
		(BYTE*)owglSwapBuffers, (BYTE*)mywglSwapBuffersHook, 5);
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
		_hModule = hModule;
		CloseHandle(CreateThread(0, 0, (LPTHREAD_START_ROUTINE)Mythread, 0, 0, 0));
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}
```