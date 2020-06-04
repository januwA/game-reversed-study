// https://www.youtube.com/watch?v=qzZTXcBu3cE&feature=emb_logo 
// https://guidedhacking.com/threads/manual-mapping-dll-injection-tutorial-how-to-manual-map.10009/

#include <iostream>
#include <fstream>
#include <Windows.h>
#include <TlHelp32.h>
#include <stralign.h>


#define RELOC_FLAG32(RelInfo) ((RelInfo >> 0x0C) == IMAGE_REL_BASED_HIGHLOW)
#define RELOC_FLAG64(RelInfo) ((RelInfo >> 0x0C) == IMAGE_REL_BASED_DIR64)

#ifdef _WIN64
#define RELOC_FLAG RELOC_FLAG64
#else
#define RELOC_FLAG RELOC_FLAG32
#endif

using namespace std;
using f_LoadLibraryA	= HINSTANCE	(WINAPI*)(const char * lpLibFilename);
using f_GetProcAddress	= UINT_PTR	(WINAPI*)(HINSTANCE hModule, const char * lpProcName);
using f_DLL_ENTRY_POINT = BOOL		(WINAPI*)(void * hDll, DWORD dwReason, void * pReserved);

struct MANUAL_MAPPING_DATA
{
	f_LoadLibraryA		pLoadLibraryA;
	f_GetProcAddress	pGetProcAddress;
	HINSTANCE			hMod;
};



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

void __stdcall Shellcode(MANUAL_MAPPING_DATA* pData)
{
	// newmem
	BYTE* curDllImageBase = (BYTE*)pData;

	// 获取可选头
	auto* dll_opt_header = &reinterpret_cast<IMAGE_NT_HEADERS*>(curDllImageBase + reinterpret_cast<IMAGE_DOS_HEADER*>(pData)->e_lfanew)->OptionalHeader;

	auto _LoadLibraryA = pData->pLoadLibraryA;
	auto _GetProcAddress = pData->pGetProcAddress;

	// AddressOfEntryPoint 程序入口地址
	auto _DllMain = reinterpret_cast<f_DLL_ENTRY_POINT>(curDllImageBase + dll_opt_header->AddressOfEntryPoint);
	
	// 如果ImageBase没有被别的dll占用，那么结果将会是0
	// 如果ImageBase被别的dll占用，那么newmem将被分配到其它位置，那么就需要修复重定位表
	BYTE* LocationDelta = curDllImageBase - dll_opt_header->ImageBase;

	if (LocationDelta)
	{
		// https://www.bilibili.com/video/av626411416?p=11&t=915
		
		// 重定位表目录
		IMAGE_DATA_DIRECTORY brDir = dll_opt_header->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];
		if (!brDir.Size) return;
		
		// 重定位表信息数组指针
		auto* pRelocData = (IMAGE_BASE_RELOCATION*)(curDllImageBase + brDir.VirtualAddress);

		// 遍历，直到没有数据
		while (pRelocData->VirtualAddress)
		{	
			// SizeOfBlock 代表当前这一块总大小
			// 用SizeOfBlock减去8字节获取剩下的数据，每个数据的大小是WROD
			// 最后获取有多少个需要修复的地方
			size_t AmountOfEntries = (pRelocData->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);
			
			// 需要重定位数据数组指针
			// WORD* pRelativeInfo = (WORD*)(pRelocData + 1);
			WORD* pRelativeInfo = (WORD*)((uintptr_t)pRelocData + 8);

			for (size_t i = 0; i < AmountOfEntries; i++)
			{
				if (RELOC_FLAG(*pRelativeInfo))
				{
					// 计算重定位信息

					// 先找到当前地址
					uintptr_t* pPatch = (uintptr_t*)(curDllImageBase + pRelocData->VirtualAddress + ((*pRelativeInfo) & 0xFFF));

					// 修复偏移地址
					*pPatch += (uintptr_t)LocationDelta;
				}
				// next
				pRelativeInfo++;
			}

			// next
			pRelocData = (IMAGE_BASE_RELOCATION*)((uintptr_t)pRelocData + pRelocData->SizeOfBlock);
		}
	}

	//  dll 导入表
	if (dll_opt_header->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size)
	{
		// 获取导入表储数据地址，导入表是一个数组，这个地址是第一个导入表数据的起始地址
		auto* pImportDescr = reinterpret_cast<IMAGE_IMPORT_DESCRIPTOR*>(curDllImageBase + dll_opt_header->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

		while (pImportDescr->Name)
		{
			// 获取导入表dll的name VA地址
			char* szMod = reinterpret_cast<char*>(curDllImageBase + pImportDescr->Name);
			HINSTANCE hDll = _LoadLibraryA(szMod);
			
			// IAT 表 RVA地址
			ULONG_PTR* pThunkRef = reinterpret_cast<ULONG_PTR*>(curDllImageBase + pImportDescr->OriginalFirstThunk);

			// 加载前和OriginalFirstThunk一样，加载后存的函数地址表
			ULONG_PTR* pFuncRef = reinterpret_cast<ULONG_PTR*>(curDllImageBase + pImportDescr->FirstThunk);

			if (!pThunkRef)
				pThunkRef = pFuncRef;

			for (; *pThunkRef; ++pThunkRef, ++pFuncRef)
			{
				// 判断这个函数是名称导出还是序号导出，如果是序号则为1，否则为0
				if (IMAGE_SNAP_BY_ORDINAL(*pThunkRef))
				{
					*pFuncRef = _GetProcAddress(hDll, reinterpret_cast<char*>(*pThunkRef & 0xFFFF));
				}
				else
				{
					auto* pImport = reinterpret_cast<IMAGE_IMPORT_BY_NAME*>(curDllImageBase + (*pThunkRef));
					*pFuncRef = _GetProcAddress(hDll, pImport->Name);
				}
			}
			pImportDescr++;
		}
	}

	// dll TLS表
	if (dll_opt_header->DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].Size)
	{
		auto* pTLS = reinterpret_cast<IMAGE_TLS_DIRECTORY*>(curDllImageBase + dll_opt_header->DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress);

		// 回调函数数组指针
		auto* pCallback = reinterpret_cast<PIMAGE_TLS_CALLBACK*>(pTLS->AddressOfCallBacks);
		while (true)
		{
			if (pCallback && *pCallback)
			{
				(*pCallback)(curDllImageBase, DLL_PROCESS_ATTACH, nullptr);
				pCallback++;
			}
			break;
		}
	}

	_DllMain(curDllImageBase, DLL_PROCESS_ATTACH, nullptr);
	pData->hMod = (HINSTANCE)curDllImageBase;
}

bool ManualMap(HANDLE hProcess, string dllpath)
{

	// 1. 将dll读取到buffer
	if (!GetFileAttributesA(dllpath.c_str()))
	{
		printf("GetFileAttributesA error.\n");
		return false;
	}

	ifstream File(dllpath, ios::binary | ios::ate);
	if (File.fail())
	{
		printf("File error.\n");
		return false;
	}
	uintptr_t filesize = (uintptr_t)File.tellg();
	BYTE* dllBuffer = new BYTE[filesize];
	File.seekg(0, ios::beg);
	File.read((char*)dllBuffer, filesize);
	File.close();


	// 2. 解析dll的PE信息
	IMAGE_DOS_HEADER* dos_header = (IMAGE_DOS_HEADER*)dllBuffer;
	if (dos_header->e_magic != 0x5A4D)
	{
		printf("is not PE file.\n");
		delete[] dllBuffer;
		return false;
	}
	IMAGE_NT_HEADERS* nt_header = (IMAGE_NT_HEADERS*)((uintptr_t)dllBuffer + dos_header->e_lfanew);
	IMAGE_FILE_HEADER* file_header = &nt_header->FileHeader;
	IMAGE_OPTIONAL_HEADER* opt_header = &nt_header->OptionalHeader;
	
	// 3. 在游戏进程中申请一块虚拟内存
	// ImageBase 加载到内存中时的起始位置，但是这个位置可能被其它dll使用，就会导致分配失败
	// SizeOfImage 对齐后的总大小
	BYTE* newmem = (BYTE*)(VirtualAllocEx(hProcess, (void*)(opt_header->ImageBase),
		opt_header->SizeOfImage, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));
	if ( !newmem )
	{
		// 要是分配失败，就让系统自动找一块地址
		newmem = (BYTE*)(VirtualAllocEx(hProcess, 0, opt_header->SizeOfImage, 
			MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));
		if (!newmem)
		{
			printf("虚拟内存分配失败\n");
			delete[] dllBuffer;
			return false;
		}
	}

	
	// 节表的起始位置，file_header->NumberOfSections 存着一共有多少节
	// printf("%x\n", (uintptr_t)opt_header + file_header->SizeOfOptionalHeader);
	auto* pSectionHeader = IMAGE_FIRST_SECTION(nt_header);

	// 4. 将所有节分配到虚拟内存中
	for (size_t i = 0; i < file_header->NumberOfSections; i++)
	{
		DWORD SizeOfRawData = pSectionHeader->SizeOfRawData;
		if (SizeOfRawData) // 节加载到内存对齐后的大小
		{
			// 将节的信息写入申请的虚拟地址
			BYTE* dllVA = newmem + pSectionHeader->VirtualAddress;
			BYTE* dllFOA = dllBuffer + pSectionHeader->PointerToRawData;
			WriteProcessMemory(hProcess, dllVA, dllFOA, SizeOfRawData, 0);
		}
		pSectionHeader++;
	}

	MANUAL_MAPPING_DATA data{ 0 };
	data.pLoadLibraryA = LoadLibraryA;
	data.pGetProcAddress = (f_GetProcAddress)GetProcAddress;

	// 这一步把dllbuffer里面的dos头开始的数据被data数据替换
	// 通常dos头的大小是0x40字节，data有3个属性每个的大小是DWORD一共0x0C字节
	memcpy_s(dllBuffer, sizeof(data), &data, sizeof(data));

	// 将dllbuffer写入newmem, 0x1000 通常是dos头到以一个节的大小
	WriteProcessMemory(hProcess, newmem, dllBuffer, 0x1000, 0);
	
	// 整个dllbuffer被写入虚拟内存中了
	delete[] dllBuffer;
	
	// 申请一块虚拟内存 存shellcode
	void* pShellcode = VirtualAllocEx(hProcess, 0, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	WriteProcessMemory(hProcess, pShellcode, Shellcode, 0x1000, 0);

	// 创建远程线程，执行shellcode，然后传入参数地址
	HANDLE hThread = CreateRemoteThread(hProcess, 0, 0, (LPTHREAD_START_ROUTINE)pShellcode, newmem, 0, 0);
	if (!hThread)
	{
		printf("远程线程创建失败\n");
		VirtualFreeEx(hProcess, newmem, 0, MEM_RELEASE);
		VirtualFreeEx(hProcess, pShellcode, 0, MEM_RELEASE);
		return false;
	}
	CloseHandle(hThread);
	
	// 检查结果
	while (true)
	{
		MANUAL_MAPPING_DATA data_checked{ 0 };
		ReadProcessMemory(hProcess, newmem, &data_checked, sizeof(data_checked), 0);
		if (data_checked.hMod) break;
		Sleep(10);
	}
	VirtualFreeEx(hProcess, pShellcode, 0, MEM_RELEASE);
	return true;
};

int main()
{
	//string dllpath = "C:\\Users\\ajanuw\\Desktop\\EmptyDll\\Release\\EmptyDll.dll";
	//string name = "game2.exe";

	string dllpath = "C:\\Users\\ajanuw\\Desktop\\EmptyDll\\x64\\Release\\EmptyDll.dll";
	string name = "ida64.exe";

	DWORD pid =  getPID(name);
	if (!pid) return 0;

	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	if (!hProcess) return 0;

	ManualMap(hProcess, dllpath);

	CloseHandle(hProcess);
	return 0;
}