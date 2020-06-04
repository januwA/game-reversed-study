(window.webpackJsonp=window.webpackJsonp||[]).push([[25],{463:function(t,n,e){"use strict";e.r(n);var s=e(54),r=Object(s.a)({},(function(){var t=this,n=t.$createElement,e=t._self._c||n;return e("ContentSlotsDistributor",{attrs:{"slot-key":t.$parent.slotKey}},[e("ul",[e("li",[t._v("https://www.youtube.com/watch?v=wiX5LmdD5yk&list=PL2C03D3BB7FAF2EA0&index=2")])]),t._v(" "),e("h2",{attrs:{id:"读取值-和写入值"}},[e("a",{staticClass:"header-anchor",attrs:{href:"#读取值-和写入值"}},[t._v("#")]),t._v(" 读取值，和写入值")]),t._v(" "),e("div",{staticClass:"language- extra-class"},[e("pre",{pre:!0,attrs:{class:"language-text"}},[e("code",[t._v('#include <iostream>\n#include <Windows.h>\n#include <TlHelp32.h>\n#include <vector>\n\n\n// 获取进程名的pid\nDWORD getPID(const wchar_t* name)\n{\n\tDWORD pid = 0;\n\tHANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);\n\tif (hSnap != INVALID_HANDLE_VALUE)\n\t{\n\t\tPROCESSENTRY32 pe;\n\t\tpe.dwSize = sizeof(pe);\n\t\tif (Process32First(hSnap, &pe))\n\t\t{\n\t\t\tdo {\n\t\t\t\tif (!_wcsicmp(pe.szExeFile, name)) {\n\t\t\t\t\tpid = pe.th32ProcessID;\n\t\t\t\t\tbreak;\n\t\t\t\t}\n\t\t\t} while (Process32Next(hSnap, &pe));\n\t\t}\n\t}\n\tCloseHandle(hSnap);\n\treturn pid;\n}\n\n// 获取模块基址\nuintptr_t getModuleBaseAddress(DWORD pid, const wchar_t* modName)\n{\n\tuintptr_t modBaseAddr = 0;\n\tHANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);\n\n\tif (hSnap != INVALID_HANDLE_VALUE)\n\t{\n\t\tMODULEENTRY32 me;\n\t\tme.dwSize = sizeof(me);\n\t\tif (Module32First(hSnap, &me))\n\t\t{\n\t\t\tdo {\n\t\t\t\tif (!_wcsicmp(me.szModule, modName)) {\n\t\t\t\t\tmodBaseAddr = (uintptr_t)me.modBaseAddr;\n\t\t\t\t\tbreak;\n\t\t\t\t}\n\t\t\t} while (Module32Next(hSnap, &me));\n\t\t}\n\t}\n\tCloseHandle(hSnap);\n\treturn modBaseAddr;\n}\n\nuintptr_t readIntger(HANDLE hProc, uintptr_t ptr, std::vector<unsigned int> offsets)\n{\n\tuintptr_t addr = ptr;\n\tfor (unsigned int i = 0; i < offsets.size(); ++i)\n\t{\n\t\t// 进程句柄, 内存地址(从这里读), 值引用(存读取得值)，值的字节大小(读多大), null\n\t\tReadProcessMemory(hProc, (BYTE*)addr, &addr, sizeof(addr), 0);\n\t\taddr += offsets[i];\n\t}\n\treturn addr;\n}\n\nint main()\n{\n\n\t// 地址: [game.exe+009E820C]+338\n\n\t // 1 获取进程pid\n\tDWORD pid = getPID(L"game.exe");\n\tprintf_s("pid: %d\\n", pid);\n\n\t// 2 打开进程\n\tHANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);\n\tif (hProcess == NULL) return 0; // 打开进程失败\n\n\t// 3 获取模块地址\n\tuintptr_t moduleBaseAddress = getModuleBaseAddress(pid, L"game.exe");\n\t// 00905A4D\n\tprintf_s("module address: %x\\n", moduleBaseAddress);\n\n\tuintptr_t r = 0;\n\tReadProcessMemory(hProcess, (LPCVOID)(moduleBaseAddress + 0x9E820C), &r, sizeof(uintptr_t), 0);\n\tReadProcessMemory(hProcess, (LPCVOID)(r + 0x338), &r, 4, 0);\n\tprintf_s("value: %d\\n", r);\n\n\n\t// 写入新的值\n\tuintptr_t newValue = 20;\n\tReadProcessMemory(hProcess, (LPCVOID)(moduleBaseAddress + 0x9E820C), &r, sizeof(r), 0);\n\tWriteProcessMemory(hProcess, (LPVOID)(r + 0x338), (LPCVOID)&newValue, sizeof(newValue), 0);\n\n\tCloseHandle(hProcess);\n\treturn 0;\n}\n')])])]),e("h2",{attrs:{id:"dll"}},[e("a",{staticClass:"header-anchor",attrs:{href:"#dll"}},[t._v("#")]),t._v(" DLL")]),t._v(" "),e("p",[t._v("使用dll你可以向开发人员一样编写程序")]),t._v(" "),e("div",{staticClass:"language- extra-class"},[e("pre",{pre:!0,attrs:{class:"language-text"}},[e("code",[t._v('// dllmain.cpp : 定义 DLL 应用程序的入口点。\n#include "pch.h"\n#include <iostream>\n#include <Windows.h>\n#include <TlHelp32.h>\n#include <vector>\n#include <regex>\n#include <sstream>\n#include <string>\n\n// 获取模块基址\nuintptr_t getModuleBaseAddress(const wchar_t* modName)\n{\n\tuintptr_t modBaseAddr = 0;\n\tHANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, 0);\n\n\tif (hSnap != INVALID_HANDLE_VALUE)\n\t{\n\t\tMODULEENTRY32 me;\n\t\tme.dwSize = sizeof(me);\n\t\tif (Module32First(hSnap, &me))\n\t\t{\n\t\t\tdo {\n\t\t\t\tif (!_wcsicmp(me.szModule, modName)) {\n\t\t\t\t\tmodBaseAddr = (uintptr_t)me.modBaseAddr;\n\t\t\t\t\tbreak;\n\t\t\t\t}\n\t\t\t} while (Module32Next(hSnap, &me));\n\t\t}\n\t}\n\tCloseHandle(hSnap);\n\treturn modBaseAddr;\n}\n\nint Mythread(HMODULE hModule)\n{\n\t// 创建控制台\n\tAllocConsole();\n\tFILE* f;\n\tfreopen_s(&f, "CONOUT$", "w", stdout);\n\tstd::cout << "run...";\n\n\tuintptr_t miduleAddress = getModuleBaseAddress(L"game.exe");\n\tstd::cout << "miduleAddress: " << std::hex << miduleAddress << std::endl;\n\n\t// hack loop\n\twhile (true)\n\t{\n\t\t// 按下f2，修改值\n\t\tif (GetAsyncKeyState(VK_F2) & 1)\n\t\t{\n\t\t\t// [game.exe+009E820C]+338\n\t\t\t// addr = *(uintptr_t*)addr => mov addr [addr]\n\n\t\t\t// lea a, [miduleAddress + 0x009E820C]\n\t\t\tuintptr_t* a = (uintptr_t*)(miduleAddress + 0x009E820C);\n\n\t\t\t// mov eax, [miduleAddress + 0x009E820C]\n\t\t\tstd::cout << "a value: " << std::hex << *a << std::endl;\n\n\t\t\tif (a)\n\t\t\t{\n\t\t\t\tstd::cout << "old value: " << std::hex << *(int*)(*a + 0x338) << std::endl;\n\t\t\t\t// mov [eax+0x338], #10\n\t\t\t\t*(int*)(*a + 0x338) = 10; // 设置新值\n\t\t\t}\n\t\t\tbreak;\n\t\t}\n\t\t\n\t\t// 修改字节集\n\t\tif (GetAsyncKeyState(VK_F3) & 1)\n\t\t{\n\t\t\tBYTE* address = (BYTE*)(miduleAddress + 0x3B79F7);\n\n\t\t\t// code: F3 0F 11 84 90 B8 02 00 00\n\t\t\tBYTE* bytes = (BYTE*)"\\x90\\x90\\x90\\x90\\x90\\x90\\x90\\x90\\x90";\n\t\t\tint size = 9;\n\n\t\t\t// 保存旧的保护值\n\t\t\tDWORD oldprotect;\n\n\t\t\t// 要更改任何进程的访问保护\n\t\t\tVirtualProtect(address, size, PAGE_EXECUTE_READWRITE, &oldprotect);\n\t\t\t\n\t\t\t// 从缓冲区复制字符\n\t\t\t// 设置新的，不同的字节集\n\t\t\t_memccpy(address, bytes, 0, size);\n\n\t\t\t// 将缓冲区设置为指定字符\n\t\t\t// 如果要设置相同的字节用这个方便点\n\t\t\t// memset(address, 0x90, size);\n\n\t\t\tVirtualProtect(address, size, oldprotect, &oldprotect);\n\t\t\tbreak;\n\t\t}\n\n\t\tif (GetAsyncKeyState(VK_F12) & 1)\n\t\t{\n\t\t\t// 释放资源\n\t\t\tfclose(f);\n\t\t\tFreeConsole();\n\t\t\tFreeLibraryAndExitThread(hModule, 0);\n\t\t\tbreak;\n\t\t}\n\n\t\tSleep(5);\n\t}\n\treturn 0;\n}\n\n\nBOOL APIENTRY DllMain(HMODULE hModule,\n\tDWORD  ul_reason_for_call,\n\tLPVOID lpReserved\n)\n{\n\tswitch (ul_reason_for_call)\n\t{\n\tcase DLL_PROCESS_ATTACH:\n\t\tCloseHandle(CreateThread(0, 0, (LPTHREAD_START_ROUTINE)Mythread, hModule, 0, 0));\n\tcase DLL_THREAD_ATTACH:\n\tcase DLL_THREAD_DETACH:\n\tcase DLL_PROCESS_DETACH:\n\t\tbreak;\n\t}\n\treturn TRUE;\n}\n')])])]),e("h2",{attrs:{id:"aascript-to-c"}},[e("a",{staticClass:"header-anchor",attrs:{href:"#aascript-to-c"}},[t._v("#")]),t._v(" AAScript to c++")]),t._v(" "),e("ul",[e("li",[t._v("https://www.youtube.com/watch?v=jTl3MFVKSUM")])]),t._v(" "),e("p",[t._v("ce的aa脚本是一种绕行/挂钩技术")]),t._v(" "),e("p",[t._v("源地址为"),e("code",[t._v("00433F86: mov [edi+00005578],esi")]),t._v("，将这里绕行到我们定义的函数，在最后jmp回去")]),t._v(" "),e("p",[t._v("AAScript:")]),t._v(" "),e("div",{staticClass:"language- extra-class"},[e("pre",{pre:!0,attrs:{class:"language-text"}},[e("code",[t._v("[ENABLE]\naobscanmodule(INJECT,PlantsVsZombies.exe,89 B7 78 55 00 00)\nalloc(newmem,$1000)\n\nlabel(return)\n\nnewmem:\n  add esi,#100\n  mov [edi+00005578],esi\n  jmp return\n\nINJECT:\n  jmp newmem\n  nop\nreturn:\nregistersymbol(INJECT)\n\n[DISABLE]\n\nINJECT:\n  db 89 B7 78 55 00 00\n\nunregistersymbol(INJECT)\ndealloc(newmem)\n\n{\n00433F84: 2B F3                          -  sub esi,ebx\n// ---------- INJECTING HERE ----------\n00433F86: 89 B7 78 55 00 00              -  mov [edi+00005578],esi\n// ---------- DONE INJECTING  ----------\n00433F8C: B0 01                          -  mov al,01\n}\n")])])]),e("p",[t._v("cpp DLL:")]),t._v(" "),e("div",{staticClass:"language- extra-class"},[e("pre",{pre:!0,attrs:{class:"language-text"}},[e("code",[t._v('#include "pch.h"\n#include <iostream>\n#include <Windows.h>\n\n\nbool handleHook(void * oldHook, void * newFunc, int len)\n{\n\tif (len < 5) return false;\n\n\t// 更改访问保护\n\tDWORD oldProc;\n\tVirtualProtect(oldHook, len, PAGE_EXECUTE_READWRITE, &oldProc);\n\n\t// 先将旧的字节集设置为nop\n\tmemset(oldHook, 0x90, len);\n\n\t// 计算新的字节集\n\t// 跳转目标地址 - 当前指令地址 - 5 = 字节集\n\tDWORD relativeAddress = ((DWORD)newFunc - (DWORD)oldHook - 5);\n\t\n\t// 设置jmp指令\n\t*(BYTE*)oldHook = 0xE9;\n\t*(DWORD*)((DWORD)oldHook + 1) = relativeAddress;\n\t\n\t// 修改后还原访问保护\n\tVirtualProtect(oldHook, len, oldProc, &oldProc);\n\n\treturn true;\n}\n\nDWORD returnAddress;\n\n// 定义新的处理函数\nvoid __declspec(naked) myNewFunc() {\n\n\t//newmem:\n\t__asm {\n\t\tadd esi, 0x64\n\t\tmov[edi + 0x00005578], esi\n\t\tjmp [returnAddress]\n\t}\n}\n\nint Mythread(HMODULE hModule)\n{\n\tDWORD oldHookAddress = 0x00433F86;\n\tint oldHookAddressLen = 6;\n\n\t// 0x433F8C\n\treturnAddress = oldHookAddress + oldHookAddressLen;\n\thandleHook((void*)oldHookAddress, myNewFunc, oldHookAddressLen);\n\n\twhile (true)\n\t{\n\t\tif (GetAsyncKeyState((VK_F2))) break;\n\t\tSleep(10);\n\t}\n\n\t// 脱钩\n\tFreeLibraryAndExitThread(hModule, 0);\n\n\treturn 0;\n}\n\nBOOL APIENTRY DllMain(HMODULE hModule,\n\tDWORD  ul_reason_for_call,\n\tLPVOID lpReserved\n)\n{\n\tswitch (ul_reason_for_call)\n\t{\n\tcase DLL_PROCESS_ATTACH:\n\t\tCloseHandle(CreateThread(0, 0, (LPTHREAD_START_ROUTINE)Mythread, hModule, 0, 0));\n\tcase DLL_THREAD_ATTACH:\n\tcase DLL_THREAD_DETACH:\n\tcase DLL_PROCESS_DETACH:\n\t\tbreak;\n\t}\n\treturn TRUE;\n}\n')])])]),e("h2",{attrs:{id:"module-scan"}},[e("a",{staticClass:"header-anchor",attrs:{href:"#module-scan"}},[t._v("#")]),t._v(" module scan")]),t._v(" "),e("ul",[e("li",[t._v("https://www.youtube.com/watch?v=5M2rjjdX6DQ")]),t._v(" "),e("li",[t._v("https://www.youtube.com/watch?v=S_SR5l_hquw")])]),t._v(" "),e("p",[t._v("aa脚本：")]),t._v(" "),e("div",{staticClass:"language- extra-class"},[e("pre",{pre:!0,attrs:{class:"language-text"}},[e("code",[t._v("[ENABLE]\n\naobscanmodule(INJECT,game2.exe,A3 24 37 4B 00)\nalloc(newmem,$1000)\n\nlabel(code)\nlabel(return)\n\nnewmem:\ncode:\n  mov [004B3724],eax\n  jmp return\n\nINJECT:\n  jmp newmem\nreturn:\nregistersymbol(INJECT)\n\n[DISABLE]\n\nINJECT:\n  db A3 24 37 4B 00\n\nunregistersymbol(INJECT)\ndealloc(newmem)\n\n{\n00401570: E8 97 FA FF FF        -  call game2.exe+100C\n// ---------- INJECTING HERE ----------\n00401575: A3 24 37 4B 00        -  mov [game2.exe+B3724],eax\n// ---------- DONE INJECTING  ----------\n0040157A: 68 01 03 00 80        -  push 80000301\n}\n")])])]),e("p",[t._v("c++")]),t._v(" "),e("div",{staticClass:"language- extra-class"},[e("pre",{pre:!0,attrs:{class:"language-text"}},[e("code",[t._v('#include "pch.h"\n#include <iostream>\n#include <Windows.h>\n#include <TlHelp32.h>\n#include <Psapi.h>\n\n\nMODULEINFO GetModuleInfo(const wchar_t* mName)\n{\n\tMODULEINFO mInfo = { 0 };\n\tHMODULE hModule = GetModuleHandleW(mName);\n\tif (hModule == 0) return mInfo;\n\n\t// 在MODULEINFO结构中检索有关指定模块的信息\n\tGetModuleInformation(GetCurrentProcess(), hModule, &mInfo, sizeof(MODULEINFO));\n\treturn mInfo;\n}\n\nDWORD ModuleScan(const wchar_t* moduleName, BYTE* bytes, const wchar_t* mask)\n{\n\t// https://docs.microsoft.com/en-us/windows/win32/api/psapi/ns-psapi-moduleinfo\n\tMODULEINFO mInfo = GetModuleInfo(moduleName);\n\n\t// 起始位置\n\tuintptr_t base = (uintptr_t)mInfo.lpBaseOfDll;\n\n\t// 模块大小\n\tuintptr_t size = (uintptr_t)mInfo.SizeOfImage;\n\n\tint patternLen = wcslen(mask);\n\twchar_t anyByte{ L\'?\' };\n\n\tfor (size_t i = 0; i < size - patternLen; i++)\n\t{\n\t\tbool found = true;\n\t\tfor (size_t j = 0; j < patternLen; j++)\n\t\t{\n\t\t\tfound &= mask[j] == anyByte || bytes[j] == *(BYTE*)(base + i + j);\n\t\t}\n\t\t\n\t\t// return find address start\n\t\tif (found) return base + i;\n\t}\n\n\treturn 0;\n}\n\nint Mythread(HMODULE hModule)\n{\n\tAllocConsole();\n\tFILE* f;\n\tfreopen_s(&f, "CONOUT$", "w", stdout);\n\n\n\tconst wchar_t* moduleName = L"game2.exe";\n\tBYTE* bytes = (BYTE*)"\\xA3\\x24\\x37\\x4B\\x00";\n\t// std::cout << (bytes[0] == 0xA3) << std::endl;\n\tconst wchar_t* mask = L"xxxxx"; // 每一个代表一个字节?代表any\n\n\tDWORD address = ModuleScan(moduleName, bytes, mask);\n\n\t// 00401575\n\tstd::cout << std::hex << address << std::endl;\n\n\twhile (true)\n\t{\n\t\tif (GetAsyncKeyState(VK_F12) & 1)\n\t\t{\n\t\t\tbreak;\n\t\t}\n\n\t\tSleep(10);\n\t}\n\n\n\tfclose(f);\n\tFreeConsole();\n\t// 脱钩\n\tFreeLibraryAndExitThread(hModule, 0);\n\treturn 0;\n}\n\nBOOL APIENTRY DllMain(HMODULE hModule,\n\tDWORD  ul_reason_for_call,\n\tLPVOID lpReserved\n)\n{\n\tswitch (ul_reason_for_call)\n\t{\n\tcase DLL_PROCESS_ATTACH:\n\t\tCloseHandle(CreateThread(0, 0, (LPTHREAD_START_ROUTINE)Mythread, hModule, 0, 0));\n\tcase DLL_THREAD_ATTACH:\n\tcase DLL_THREAD_DETACH:\n\tcase DLL_PROCESS_DETACH:\n\t\tbreak;\n\t}\n\treturn TRUE;\n}\n')])])]),e("h2",{attrs:{id:"c-调用游戏函数"}},[e("a",{staticClass:"header-anchor",attrs:{href:"#c-调用游戏函数"}},[t._v("#")]),t._v(" C++调用游戏函数")]),t._v(" "),e("ul",[e("li",[t._v("https://www.youtube.com/watch?v=jmgwFpVnRmU")]),t._v(" "),e("li",[t._v("https://www.youtube.com/watch?v=gZN2damgYHg")]),t._v(" "),e("li",[t._v("https://www.youtube.com/watch?v=-hwUPT5Gyvc")])]),t._v(" "),e("h2",{attrs:{id:"绕行挂钩技术-x86"}},[e("a",{staticClass:"header-anchor",attrs:{href:"#绕行挂钩技术-x86"}},[t._v("#")]),t._v(" 绕行挂钩技术 x86")]),t._v(" "),e("div",{staticClass:"language- extra-class"},[e("pre",{pre:!0,attrs:{class:"language-text"}},[e("code",[t._v('#include "pch.h"\n#include <iostream>\n#include <Windows.h>\n#include <TlHelp32.h>\n\nusing namespace std;\n\nuintptr_t GetModuleBaseAddr(const wchar_t* name)\n{\n\tuintptr_t addr = 0;\n\tHANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, 0);\n\n\tif (hSnap != INVALID_HANDLE_VALUE)\n\t{\n\t\tMODULEENTRY32 me;\n\t\tme.dwSize = sizeof(me);\n\t\tif (Module32First(hSnap, &me))\n\t\t{\n\t\t\tdo\n\t\t\t{\n\t\t\t\tif (!_wcsicmp(me.szModule, name))\n\t\t\t\t{\n\t\t\t\t\taddr = (uintptr_t)me.modBaseAddr;\n\t\t\t\t\tbreak;\n\t\t\t\t}\n\t\t\t} while (Module32Next(hSnap, &me));\n\t\t}\n\t}\n\n\tCloseHandle(hSnap);\n\treturn addr;\n}\n\nuintptr_t returnAddr = 0;\n\nvoid __declspec(naked) mymemnew()\n{\n\t__asm {\n\t\tinc [esi]\n\t\tpush edi\n\t\tmov edi, [esp + 0x14]\n\t\tjmp [returnAddr]\n\t}\n}\n\nint MyThread(HMODULE hModule)\n{\n\tAllocConsole();\n\tFILE* f;\n\tfreopen_s(&f, "CONOUT$", "w", stdout);\n\tcout << "INJECT OK" << endl;\n\n\t// 1.获取模块地址\n\t// 2.创建钩子\n\t// 3.将源地址跳到钩子\n\n\tuintptr_t addr = GetModuleBaseAddr(L"ac_client.exe");\n\tBYTE* address = (BYTE*)(addr + 0x637E9);\n\tBYTE bytes[] = { 0xFF, 0x0E, 0x57, 0x8B, 0x7C, 0x24, 0x14 };\n\tunsigned int size = sizeof(bytes);\n\treturnAddr = (uintptr_t)address + size;\n\tDWORD jmpBytes = (DWORD)mymemnew - (DWORD)address - 5;\n\n\tbool bEnable = false;\n\twhile (true)\n\t{\n\n\t\tif (GetAsyncKeyState(VK_F2) & 1)\n\t\t{\n\t\t\tbEnable = !bEnable;\n\t\t\tDWORD oldProc;\n\t\t\tVirtualProtect(address, size, PAGE_EXECUTE_READWRITE, &oldProc);\n\t\t\tif (bEnable)\n\t\t\t{\n\t\t\t\tmemset(address, 0x90, size);\n\t\t\t\t*(BYTE*)address = 0xE9;\n\t\t\t\t*(DWORD*)(address + 1) = jmpBytes;\n\t\t\t}\n\t\t\telse\n\t\t\t{\n\t\t\t\t_memccpy(address, bytes, 0, size);\n\t\t\t}\n\t\t\tVirtualProtect(address, size, oldProc, 0);\n\t\t}\n\n\t\tif( GetAsyncKeyState(VK_F12)&1 )\n\t\t{\n\t\t\tbreak;\n\t\t}\n\n\t\tSleep(20);\n\t}\n\n\tfclose(f);\n\tFreeConsole();\n\tFreeLibraryAndExitThread(hModule, 0);\n\treturn 0;\n}\n')])])]),e("h2",{attrs:{id:"绕行挂钩技术-x64"}},[e("a",{staticClass:"header-anchor",attrs:{href:"#绕行挂钩技术-x64"}},[t._v("#")]),t._v(" 绕行挂钩技术 x64")]),t._v(" "),e("div",{staticClass:"language- extra-class"},[e("pre",{pre:!0,attrs:{class:"language-text"}},[e("code",[t._v('#include "pch.h"\n#include <iostream>\n#include <Windows.h>\n#include <TlHelp32.h>\n#include <Psapi.h>\n\nusing namespace std;\n\nMODULEINFO GetModuleInfo(const wchar_t* modname)\n{\n\tMODULEINFO mi{ 0 };\n\tHMODULE hMod = GetModuleHandle(modname);\n\tGetModuleInformation(GetCurrentProcess(), hMod, &mi, sizeof(mi));\n\treturn mi;\n}\n\nint Mythread(HMODULE hModule)\n{\n\tAllocConsole();\n\tFILE* f;\n\tfreopen_s(&f, "CONOUT$", "w", stdout);\n\tcout << "INJECT OK" << endl;\n\n\t// 获取模块信息\n\tMODULEINFO mi = GetModuleInfo(L"MonsterHunterWorld.exe");\n\n\t// "MonsterHunterWorld.exe"+CE7462A\n\tBYTE* address = (BYTE*)((uintptr_t)mi.lpBaseOfDll + 0xCE7462A);\n\tBYTE bytes[] = {\n\t\t0xF3, 0x0F, 0x11, 0x41, 0x6C, // movss [rcx+6C],xmm0\n\t};\n\tint bytesSize = sizeof(bytes);\n\n\t// "MonsterHunterWorld.exe"+CE7462F\n\tuintptr_t returnAddr = (uintptr_t)address + bytesSize;\n\n\n\t/* newmem */\n\tBYTE codeBytes[] = {\n\t\t0xC7, 0x41, 0x6C, 0x00, 0x00, 0xC8, 0x42, // mov [rcx+6C],42C80000\n\t};\n\tint codeSize = sizeof(codeBytes);\n\n\t// 申请500字节虚拟空间存代码\n\tBYTE* newmem = (BYTE*)VirtualAlloc((BYTE*)((uintptr_t)mi.lpBaseOfDll - 0x10000)/* 2-4GB */, 500,\n\t\tMEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);\n\n\t/*\n\tBYTE* newmem2 = (BYTE*)VirtualAlloc((BYTE*)((uintptr_t)newmem - 0x10000), 4,\n\t\tMEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);\n\t*/\n\n\n\t// 将代码复制到newmem\n\tmemcpy_s(newmem, codeSize, codeBytes, codeSize);\n\n\t// jmp return\n\tuintptr_t newmemJmpReturnAddr = ((uintptr_t)newmem + codeSize);\n\n\t// 计算jmp字节集，丢弃高位\n\tDWORD returnBytes = (DWORD)(returnAddr - newmemJmpReturnAddr - 5);\n\n\t// jmp "MonsterHunterWorld.exe"+CE7462F\n\t*(BYTE*)(newmemJmpReturnAddr) = 0xE9; // jmp\n\t*(DWORD*)(newmemJmpReturnAddr + 1) = returnBytes; // \n\t/* newmem */\n\n\tDWORD jmpNewmemBytes = (DWORD)((uintptr_t)newmem - (uintptr_t)address - 5);\n\tbool bEnable = false;\n\twhile (true)\n\t{\n\t\tif (GetAsyncKeyState(VK_F2) & 1)\n\t\t{\n\t\t\tbEnable = !bEnable;\n\t\t\tDWORD oldProc;\n\t\t\tVirtualProtect(address, bytesSize, PAGE_EXECUTE_READWRITE, &oldProc);\n\t\t\tif (bEnable)\n\t\t\t{\n\t\t\t\tcout << "[ENABLE]" << endl;\n\t\t\t\tmemset(address, 0x90, bytesSize);\n\n\t\t\t\t// jmp newmem\n\t\t\t\t*(BYTE*)(address) = 0xE9; // jmp\n\t\t\t\t*(DWORD*)(address + 1) = jmpNewmemBytes;\n\t\t\t}\n\t\t\telse\n\t\t\t{\n\t\t\t\tcout << "[DISABLE]" << endl;\n\t\t\t\tmemcpy_s(address, bytesSize, bytes, bytesSize);\n\t\t\t}\n\t\t\tVirtualProtect(address, bytesSize, oldProc, 0);\n\t\t}\n\n\t\tif (GetAsyncKeyState(VK_F12) & 1)\n\t\t{\n\t\t\tbreak;\n\t\t}\n\t\tSleep(20);\n\t}\n\t\n\tVirtualFree(newmem, 0, MEM_RELEASE);\n\tfclose(f);\n\tFreeConsole();\n\tFreeLibraryAndExitThread(hModule, 0);\n\treturn 0;\n}\n')])])]),e("h2",{attrs:{id:"钩住opengl32-dll-wglswapbuffers函数头"}},[e("a",{staticClass:"header-anchor",attrs:{href:"#钩住opengl32-dll-wglswapbuffers函数头"}},[t._v("#")]),t._v(" 钩住opengl32.dll->wglSwapBuffers函数头")]),t._v(" "),e("ul",[e("li",[t._v("https://guidedhacking.com/threads/how-to-hook-functions-code-detouring-guide.14185/")])]),t._v(" "),e("div",{staticClass:"language- extra-class"},[e("pre",{pre:!0,attrs:{class:"language-text"}},[e("code",[t._v('#include "pch.h"\n#include <iostream>\n#include <Windows.h>\n#include <TlHelp32.h>\n\nusing namespace std;\n\ntypedef BOOL(__stdcall* mywglSwapBuffers)(HDC hDc);\nmywglSwapBuffers  owglSwapBuffers;\n\nHMODULE _hModule = 0;\nFILE* _f;\n\n\n// 定义自己的钩子函数\nBOOL __stdcall mywglSwapBuffersHook(HDC hDc)\n{\n\t\tif (GetAsyncKeyState(VK_F2) & 1)\n\t\t{\n\t\t\tcout << "F2" << endl;\n\t\t}\n\n\t\t/* 脱钩失败\n\t\tif (GetAsyncKeyState(VK_F12) & 1)\n\t\t{\n\t\t\tfclose(_f);\n\t\t\tFreeConsole();\n\t\t\tFreeLibraryAndExitThread(_hModule, 0);\n\t\t}\n\t\t*/\n\treturn owglSwapBuffers(hDc);\n}\n\nuintptr_t GetModuleBaseAddress(const wchar_t* modName)\n{\n\tuintptr_t modBaseAddr = 0;\n\tHANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, 0);\n\n\tif (hSnap != INVALID_HANDLE_VALUE)\n\t{\n\t\tMODULEENTRY32 me;\n\t\tme.dwSize = sizeof(me);\n\t\tif (Module32First(hSnap, &me))\n\t\t{\n\t\t\tdo {\n\t\t\t\tif (!_wcsicmp(me.szModule, modName)) {\n\t\t\t\t\tmodBaseAddr = (uintptr_t)me.modBaseAddr;\n\t\t\t\t\tbreak;\n\t\t\t\t}\n\t\t\t} while (Module32Next(hSnap, &me));\n\t\t}\n\t}\n\tCloseHandle(hSnap);\n\treturn modBaseAddr;\n}\n\n\n/* 钩住wglSwapBuffers */\nbool Detour32(BYTE* origen, BYTE* hook, int len)\n{\n\tif (len < 5) return false;\n\tDWORD oldProc;\n\tVirtualProtect(origen, len, PAGE_EXECUTE_READWRITE, &oldProc);\n\t\n\t// 将wglSwapBuffers函数开始处设置为\n\t// jmp mywglSwapBuffersHook\n\tuintptr_t jmpHookBytes =  hook - origen - 5;\n\t*origen = 0xE9;\n\t*(uintptr_t*)(origen + 1) = jmpHookBytes;\n\n\tVirtualProtect(origen, len, oldProc, 0);\n}\n\nBYTE* TrampHook32(BYTE* origen, BYTE* hook, int len)\n{\n\tif (len < 5) return 0;\n\n\t// 申请一块虚拟内存存代码\n\tBYTE* gateway = (BYTE*)VirtualAlloc(0, len, \n\t\tMEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);\n\n\t// 从origen中拷贝len大小的字节到gateway\n\t_memccpy(gateway, origen, 0, len);\n\n\t// jmp wglSwapBuffers+5\n\t// uintptr_t returnBytes = (origen+len) - (gateway+len) - 5;\n\tuintptr_t returnBytes = origen - gateway - 5;\n\t*(gateway + len) = 0xE9;\n\t*(uintptr_t*)((uintptr_t)gateway + len + 1) = returnBytes;\n\n\tDetour32(origen, hook, len);\n\n\treturn gateway;\n}\n\nint Mythread()\n{\n\tAllocConsole();\n\tfreopen_s(&_f, "CONOUT$", "w", stdout);\n\tcout << "INJECT OK" << endl;\n\n\t// 显示连接DLL函数\n\t// opengl32.dll导出的wglSwapBuffers函数\n\towglSwapBuffers = (mywglSwapBuffers)GetProcAddress(\n\t\tGetModuleHandleW(L"opengl32.dll"), "wglSwapBuffers");\n\n\t// 将返回的虚拟内存地址给owglSwapBuffers\n\t// 在mywglSwapBuffersHook中的owglSwapBuffers将执行\n\towglSwapBuffers = (mywglSwapBuffers)TrampHook32(\n\t\t(BYTE*)owglSwapBuffers, (BYTE*)mywglSwapBuffersHook, 5);\n\treturn 0;\n}\n\nBOOL APIENTRY DllMain(HMODULE hModule,\n\tDWORD  ul_reason_for_call,\n\tLPVOID lpReserved\n)\n{\n\tswitch (ul_reason_for_call)\n\t{\n\tcase DLL_PROCESS_ATTACH:\n\t\t_hModule = hModule;\n\t\tCloseHandle(CreateThread(0, 0, (LPTHREAD_START_ROUTINE)Mythread, 0, 0, 0));\n\tcase DLL_THREAD_ATTACH:\n\tcase DLL_THREAD_DETACH:\n\tcase DLL_PROCESS_DETACH:\n\t\tbreak;\n\t}\n\treturn TRUE;\n}\n')])])])])}),[],!1,null,null,null);n.default=r.exports}}]);