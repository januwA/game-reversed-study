// https://www.youtube.com/watch?v=FmCDVwA5kYQ

#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include <string.h>

using namespace std;

typedef  NTSTATUS(__stdcall* f_NtCreateThreadEx)(
	HANDLE* pThreadHandleOut,
	ACCESS_MASK DesiredAccess,
	void* pAttr,
	HANDLE hProc,
	void* pRoutine,
	void* pArg,
	ULONG Flags,
	SIZE_T ZeroBits,
	SIZE_T StackSize,
	SIZE_T MaxStackSize,
	void* pAttrListOut
);

/*
typedef NTSTATUS(WINAPI* LPFUN_NtCreateThreadEx)
(
	OUT PHANDLE hThread,
	IN ACCESS_MASK DesiredAccess,
	IN LPVOID ObjectAttributes,
	IN HANDLE ProcessHandle,
	IN LPTHREAD_START_ROUTINE lpStartAddress,
	IN LPVOID lpParameter,
	IN BOOL CreateSuspended,
	IN ULONG StackZeroBits,
	IN ULONG SizeOfStackCommit,
	IN ULONG SizeOfStackReserve,
	OUT LPVOID lpBytesBuffer
	);
*/


// 获取PID
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
	if (!exportEntry->Size)
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


bool inject(string processName, const char* dllpath)
{
	void* loadlibrary_a = MyGetProcAddress(processName, string("kernel32.dll"), string("LoadLibraryA"));
	//printf("loadlibrary_a: %p\n", loadlibrary_a);

	//auto nt_create_thread_ex = (f_NtCreateThreadEx)GetProcAddress(GetModuleHandle(L"ntdll.dll"), "NtCreateThreadEx");
	auto nt_create_thread_ex = (f_NtCreateThreadEx)MyGetProcAddress(processName, string("ntdll.dll"), string("NtCreateThreadEx"));
	//printf("nt_create_thread_ex: %p\n", nt_create_thread_ex);

	HANDLE hProcess =  OpenProcess(PROCESS_ALL_ACCESS, FALSE, getPID(processName.c_str()));
	if (!hProcess)
	{
		printf("OpenProcess error\n");
		return false;
	}
	
	// 将参数放在虚拟内存中
	size_t len = strlen(dllpath) + sizeof(char);
	BYTE* argMem = (BYTE*)VirtualAllocEx(hProcess, 0, len, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	WriteProcessMemory(hProcess, argMem, dllpath, len, 0);

	/*
#ifdef _WIN64
	BYTE Shellcode[] =
	{
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		// - 0x10	-> argument / returned value
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		// - 0x08	-> pRoutine

		0x48, 0x8B, 0xC1,									// + 0x00	-> mov rax, rcx
		0x48, 0x8B, 0x08,									// + 0x03	-> mov rcx, [rax]

		0x48, 0x83, 0xEC, 0x28,								// + 0x06	-> sub rsp, 0x28
		0xFF, 0x50, 0x08,									// + 0x0A	-> call qword ptr [rax + 0x08]
		0x48, 0x83, 0xC4, 0x28,								// + 0x0D	-> add rsp, 0x28

		0x48, 0x8D, 0x0D, 0xD8, 0xFF, 0xFF, 0xFF,			// + 0x11	-> lea rcx, [pCodecave]
		0x48, 0x89, 0x01,									// + 0x18	-> mov [rcx], rax
		0x48, 0x31, 0xC0,									// + 0x1B	-> xor rax, rax

		0xC3												// + 0x1E	-> ret
}; // SIZE = 0x1F (+ 0x10)
	

	*(void**)(Shellcode + 0) = argMem;
	*(void**)(Shellcode + sizeof(uintptr_t)) = loadlibrary_a;

	void* newmem = VirtualAllocEx(hProcess, nullptr, 0x100, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	WriteProcessMemory(hProcess, newmem, Shellcode, sizeof(Shellcode), nullptr);

	void* pRemoteArg = newmem;
	void* pRemoteFunc = (BYTE*)((uintptr_t)newmem + (sizeof(uintptr_t) * 2));

	HANDLE hThread = nullptr;
	NTSTATUS ntRet = nt_create_thread_ex(&hThread, THREAD_ALL_ACCESS, 0, hProcess, pRemoteFunc, pRemoteArg, 0, 0, 0, 0, 0);
	printf("%d\n", ntRet);
	WaitForSingleObject(hThread, INFINITE);

#else

	HANDLE hThread = nullptr;
	NTSTATUS ntRet = nt_create_thread_ex(&hThread, THREAD_ALL_ACCESS, 0, hProcess, loadlibrary_a, argMem, 0, 0, 0, 0, 0);
	printf("%d\n", ntRet);
	WaitForSingleObject(hThread, INFINITE);
#endif
	*/

	HANDLE hThread = nullptr;
	NTSTATUS ntRet = nt_create_thread_ex(&hThread, THREAD_ALL_ACCESS, 0, hProcess, loadlibrary_a, argMem, 0, 0, 0, 0, 0);
	printf("%d\n", ntRet);
	WaitForSingleObject(hThread, INFINITE);

	
	VirtualFreeEx(hProcess, argMem, 0, MEM_RELEASE);
	CloseHandle(hThread);
	CloseHandle(hProcess);
	return true;
}

int main()
{
	//const string processName = "game2.exe";
	//const string dllpath = "C:\\Users\\ajanuw\\Desktop\\EmptyDll\\Release\\EmptyDll.dll";

	const string processName = "ida64.exe";
	const string dllpath = "C:\\Users\\ajanuw\\Desktop\\EmptyDll\\x64\\Release\\EmptyDll.dll";

	inject(processName, dllpath.c_str());
	return 0;
}