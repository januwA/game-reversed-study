// 线程劫持 https://www.youtube.com/watch?v=PEMkcbY8U9o&t=25s

#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include <string.h>

using namespace std;

// 获取process ID
DWORD getPID(string name)
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
				if (!_wcsicmp(pe.szExeFile, wstring(name.begin(), name.end()).c_str())) {
					pid = pe.th32ProcessID;
					break;
				}
			} while (Process32Next(hSnap, &pe));
		}
	}
	CloseHandle(hSnap);
	return pid;
}

// 获取线程id
DWORD getTID(DWORD pid)
{
	DWORD tid = 0;
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, pid);
	if (hSnap == INVALID_HANDLE_VALUE) return tid;

	THREADENTRY32 te{ 0 };
	te.dwSize = sizeof(te);
	if (Thread32First(hSnap, &te))
	{
		do
		{
			if (te.th32OwnerProcessID == pid)
			{
				tid = te.th32ThreadID;
				break;
			}
		} while (Thread32Next(hSnap, &te));
	}
	CloseHandle(hSnap);
	return tid;
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

// 读取字节集中的ASCII
void ReadASCII(BYTE* addr, size_t offset, char r[])
{
	size_t i = 0;
	char c;
	while (true)
	{
		c = *(addr + offset + i);
		r[i] = c; // 需要把最后一个0写进去
		if (!c) break;
		i++;
	}
}

// 在指定模块的导出表中，找到指定函数名的函数地址
void* MyGetProcAddress(string processName, string modName, string exportFunName)
{
	DWORD pid = getPID(processName);
	if (!pid) return nullptr;

	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	if (!hProcess) return nullptr;

	uintptr_t kernel32BaseAddr = getModuleBaseAddress(pid,
		wstring(modName.begin(), modName.end()).c_str());

	//printf("kernel32BaseAddr: %x\n", kernel32BaseAddr);

	auto RVA2VA = [&](uintptr_t rva) -> uintptr_t
	{
		return kernel32BaseAddr + rva;
	};

	// 储存headers+节表 对齐后
	BYTE peHeader[0x1000];
	ReadProcessMemory(hProcess, (LPVOID)kernel32BaseAddr, peHeader, sizeof(peHeader), 0);

	IMAGE_DOS_HEADER* dosHeader = (IMAGE_DOS_HEADER*)peHeader;

	// nt头可以分为PE头，标准头，可选头
	IMAGE_NT_HEADERS* ntHeader = (IMAGE_NT_HEADERS*)(kernel32BaseAddr + dosHeader->e_lfanew);

	IMAGE_DATA_DIRECTORY* exportEntry = (IMAGE_DATA_DIRECTORY*)&ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
	if (!exportEntry || !exportEntry->Size)
	{
		printf("没有导出表\n");
		return nullptr;
	}

	BYTE* exportDirData = new BYTE[exportEntry->Size];
	ReadProcessMemory(hProcess, (LPVOID)RVA2VA(exportEntry->VirtualAddress),
		exportDirData, exportEntry->Size, 0);

	IMAGE_EXPORT_DIRECTORY* exportDir = (IMAGE_EXPORT_DIRECTORY*)exportDirData;

	// 函数数量
	//printf("NumberOfFunctions: %d\n", exportDir->NumberOfFunctions);

	// 以函数名导出的数量
	//printf("NumberOfNames:     %d\n", exportDir->NumberOfNames);

	// 获取fun name表的VA地址
	// 每个大小是DWORD，一共有NumberOfNames个，并且表里面存的都是RVA值
	DWORD* AddressOfNamesVA = (DWORD*)RVA2VA(exportDir->AddressOfNames);


	uintptr_t itRVA = 0;
	uintptr_t itVA = 0;
	char funName[1024];
	size_t funNameIndex = 0;
	for (; funNameIndex < exportDir->NumberOfNames; funNameIndex++)
	{
		itRVA = *(AddressOfNamesVA + funNameIndex);
		itVA = kernel32BaseAddr + itRVA;
		ReadASCII((BYTE*)itVA, 0, funName);
		// printf("%s\n", funName);
		if (!strcmp(funName, exportFunName.c_str()))
		{
			break;
		}
	}
	//printf("funName: %s\n", funName);
	//printf("funNameIndex: %d\n", funNameIndex);

	// 拿到索引后去AddressOfNameOrdinals表，找到对应的索引中的值，取出来的值是
	// AddressOfFunctions的索引，里面存的就是地址


	// 获取AddressOfNameOrdinals表的VA地址，每个大小是WORD
	WORD* AddressOfNameOrdinalsVA = (WORD*)RVA2VA(exportDir->AddressOfNameOrdinals);
	WORD AddressOfFunctionsIndex = *(AddressOfNameOrdinalsVA + funNameIndex);


	// 获取AddressOfFunctions表的VA地址，每个大小是DWORD
	DWORD* AddressOfFunctionsVA = (DWORD*)RVA2VA(exportDir->AddressOfFunctions);

	// 每个函数地址存的是RVA地址，需要转为VA使用
	DWORD funAddrVA = *(AddressOfFunctionsVA + AddressOfFunctionsIndex);
	//printf("funName Addr RVA: %x\n", RVA2VA(funAddrVA));

	delete[] exportDirData;
	CloseHandle(hProcess);

	return (VOID*)RVA2VA(funAddrVA);
}

bool HijackThreadInject(string processName, const char* dllpath)
{
	DWORD pid = getPID(processName);
	if (!pid) return false;

	DWORD tid = getTID(pid);
	if (!tid) return false;
	
	// 打开这个线程
	HANDLE hThread = OpenThread(THREAD_SET_CONTEXT | THREAD_GET_CONTEXT | THREAD_SUSPEND_RESUME,
		FALSE, tid);
	if (!hThread) return false;

	
	// 挂起线程
	if (SuspendThread(hThread) == (DWORD)-1)
	{
		CloseHandle(hThread);
		return false;
	}
	
	// 获取线程上下文
	// https://docs.microsoft.com/en-us/windows/win32/api/winnt/ns-winnt-context 
	CONTEXT ctx{ 0 };
	ctx.ContextFlags = CONTEXT_CONTROL;
	GetThreadContext(hThread, &ctx);
	
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	if (!hProcess) return false;

	void* loadlibrary_a = MyGetProcAddress(processName, string("kernel32.dll"), string("LoadLibraryA"));
	// 将参数放在虚拟内存中
	size_t len = strlen(dllpath) + sizeof(char);
	BYTE* argMem = (BYTE*)VirtualAllocEx(hProcess, 0, len, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	WriteProcessMemory(hProcess, argMem, dllpath, len, 0);

	void* newmem = VirtualAllocEx(hProcess, 0, 0x1000,
		MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

#ifdef _WIN64
	BYTE Shellcode[] =
	{
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,							// - 0x08			-> returned value

		0x48, 0x83, 0xEC, 0x08,													// + 0x00			-> sub rsp, 0x08

		0xC7, 0x04, 0x24, 0x00, 0x00, 0x00, 0x00,								// + 0x04 (+ 0x07)	-> mov [rsp], RipLowPart
		0xC7, 0x44, 0x24, 0x04, 0x00, 0x00, 0x00, 0x00,							// + 0x0B (+ 0x0F)	-> mov [rsp + 0x04], RipHighPart

		0x50, 0x51, 0x52, 0x41, 0x50, 0x41, 0x51, 0x41, 0x52, 0x41, 0x53,		// + 0x13			-> push r(a/c/d)x / r(8 - 11)
		0x9C,																	// + 0x1E			-> pushfq

		0x48, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,				// + 0x1F (+ 0x21)	-> mov rax, pRoutine
		0x48, 0xB9, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,				// + 0x29 (+ 0x2B)	-> mov rcx, pArg

		0x48, 0x83, 0xEC, 0x20,													// + 0x33			-> sub rsp, 0x20
		0xFF, 0xD0,																// + 0x37			-> call rax
		0x48, 0x83, 0xC4, 0x20,													// + 0x39			-> add rsp, 0x20

		0x48, 0x8D, 0x0D, 0xB4, 0xFF, 0xFF, 0xFF,								// + 0x3D			-> lea rcx, [pCodecave]
		0x48, 0x89, 0x01,														// + 0x44			-> mov [rcx], rax

		0x9D,																	// + 0x47			-> popfq
		0x41, 0x5B, 0x41, 0x5A, 0x41, 0x59, 0x41, 0x58, 0x5A, 0x59, 0x58,		// + 0x48			-> pop r(11-8) / r(d/c/a)x

		0xC6, 0x05, 0xA9, 0xFF, 0xFF, 0xFF, 0x00,								// + 0x53			-> mov byte ptr[$ - 0x57], 0

		0xC3																	// + 0x5A			-> ret
	}; // SIZE = 0x5B (+ 0x08)

	DWORD FuncOffset = 0x08;
	DWORD CheckByteOffset = 0x03 + FuncOffset;

	DWORD dwLoRIP = (DWORD)(ctx.Rip & 0xFFFFFFFF);
	DWORD dwHiRIP = (DWORD((ctx.Rip) >> 0x20) & 0xFFFFFFFF);

	*reinterpret_cast<DWORD*>(Shellcode + 0x07 + FuncOffset) = dwLoRIP;
	*reinterpret_cast<DWORD*>(Shellcode + 0x0F + FuncOffset) = dwHiRIP;

	*reinterpret_cast<void**>(Shellcode + 0x21 + FuncOffset) = loadlibrary_a;
	*reinterpret_cast<void**>(Shellcode + 0x2B + FuncOffset) = argMem;
	ctx.Rip = (uintptr_t)(newmem)+FuncOffset;

#else
	BYTE Shellcode[] =
	{
		0x00, 0x00, 0x00, 0x00,						// - 0x04 (newmem)	-> 留着存loadLibrary返回值

		0x83, 0xEC, 0x04,							// + 0x00				-> sub esp, 0x04		; 给后面的ret的返回地址								
		0xC7, 0x04, 0x24, 0x00, 0x00, 0x00, 0x00,	// + 0x03 (+ 0x06)		-> mov [esp], ctx.Eip	

		0x50, 0x51, 0x52,							// + 0x0A				-> psuh e(a/c/d)							;save e(a/c/d)x
		0x9C,										// + 0x0D				-> pushfd

		0xB9, 0x00, 0x00, 0x00, 0x00,				// + 0x0E (+ 0x0F)		-> mov ecx, dllpath	
		0xB8, 0x00, 0x00, 0x00, 0x00,				// + 0x13 (+ 0x14)		-> mov eax, loadLibrary
		0x51,										// + 0x18				-> push ecx
		0xFF, 0xD0,									// + 0x19				-> call eax	 ; call loadLibrary
		0xA3, 0x00, 0x00, 0x00, 0x00,				// + 0x1B (+ 0x1C)		-> mov dword ptr [newmem], eax

		0x9D,										// + 0x20				-> popfd
		0x5A, 0x59, 0x58,							// + 0x21				-> pop e(d/c/a)								;restore e(d/c/a)x

		0xC6, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00,	// + 0x24 (+ 0x26)		-> mov byte ptr [newmem + 0x06], 0x00		;set checkbyte to 0

		0xC3										// + 0x2B				-> ret
	}; // SIZE = 0x2C (+ 0x04)

	DWORD FuncOffset = 0x04;
	DWORD CheckByteOffset = FuncOffset + 0x02;

	*(DWORD*)(Shellcode + FuncOffset + 0x06) = ctx.Eip; // 存挂钩后的返回地址
	*(void**)(Shellcode + FuncOffset + 0x0F) = argMem; // 设置dllpath地址
	*(void**)(Shellcode + FuncOffset + 0x14) = loadlibrary_a; // 设置loadLibraryA地址

	*(void**)(Shellcode + FuncOffset + 0x1C) = newmem;
	*(uintptr_t*)(Shellcode + FuncOffset + 0x26) = (uintptr_t)(newmem)+CheckByteOffset;

	// 拦截线程
	ctx.Eip = (DWORD)(newmem) + FuncOffset;

#endif // _WIN64
	WriteProcessMemory(hProcess, newmem, Shellcode, sizeof(Shellcode), 0);
	SetThreadContext(hThread, &ctx);

	ResumeThread(hThread);
	CloseHandle(hThread);
	
	// 检测loadLibraryA被执行
	BYTE CheckByte = 1;
	while (true)
	{
		ReadProcessMemory(hProcess, (BYTE*)(newmem)+CheckByteOffset, &CheckByte, sizeof(BYTE), 0);
		if (CheckByte == 0) break;
		Sleep(20);
	}
	printf("CheckByte %d\n", CheckByte);
	VirtualFreeEx(hProcess, argMem, 0, MEM_RELEASE);
	VirtualFreeEx(hProcess, newmem, 0, MEM_RELEASE);
	CloseHandle(hProcess);
	return true;
}

int main()
{
	//const string processName = "game2.exe";
	//const string processName = "ida.exe";
	//const string processName = "Tutorial-i386.exe";
	//const string dllpath = "C:\\Users\\ajanuw\\Desktop\\EmptyDll\\Release\\EmptyDll.dll";

	const string processName = "ida64.exe";
	const string dllpath = "C:\\Users\\ajanuw\\Desktop\\EmptyDll\\x64\\Release\\EmptyDll.dll";

	HijackThreadInject(processName, dllpath.c_str());
	return 0;
}