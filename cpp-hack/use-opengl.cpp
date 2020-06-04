#include "pch.h"
#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include <Psapi.h>
#include <gl\GL.h>

// https://guidedhacking.com/threads/opengl-hooking-drawing-text-rendering-tutorial.14460/
// https://docs.microsoft.com/en-us/windows/win32/opengl/opengl

using namespace std;

namespace rgb
{
	const GLubyte red[3] = { 255, 0, 0 };
	const GLubyte green[3] = { 0, 255, 0 };
	const GLubyte gray[3] = { 55, 55, 55 };
	const GLubyte lightgray[3] = { 192, 192, 192 };
	const GLubyte black[3] = { 0, 0, 0 };
}


namespace GL
{
	void SetupOrtho()
	{
		glPushAttrib(GL_ALL_ATTRIB_BITS);
		glPushMatrix();
		GLint viewport[4];
		glGetIntegerv(GL_VIEWPORT, viewport);
		glViewport(0, 0, viewport[2], viewport[3]);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		glOrtho(0, viewport[2], viewport[3], 0, -1, 1);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glDisable(GL_DEPTH_TEST);
	}

	void RestorGL()
	{
		glPopMatrix();
		glPopAttrib();
	}

	void DrawFilledRect(float x, float y, float w, float h, const GLubyte color[3])
	{
		glColor3ub(color[0], color[1], color[2]);
		glBegin(GL_QUADS);
		glVertex2f(x, y);
		glVertex2f(x + w, y);
		glVertex2f(x + w, y + h);
		glVertex2f(x, y + h);
		glEnd();
	}


	void DrawOutline(float x, float y, float w, float h, float lineWidth, const GLubyte color[3])
	{
		float offset = 0.5f;
		glLineWidth(lineWidth);
		glBegin(GL_LINE_STRIP);
		glColor3ub(color[0], color[1], color[2]);
		glVertex2f(x - offset, y - offset);
		glVertex2f(x + w + offset, y - offset);
		glVertex2f(x + w + offset, y + h + offset);
		glVertex2f(x - offset, y + h + offset);
		glVertex2f(x - offset, y - offset);
		glEnd();
	}
}
namespace mem
{
	void setHook(BYTE* src, BYTE* dst, int len)
	{
		// 3. 将 wglSwapBuffsers 开始的5个字节改为 jmp hook
		DWORD oldProc;
		VirtualProtect(src, len, PAGE_EXECUTE_READWRITE, &oldProc);

		// jmp hook
		*(BYTE*)src = 0xE9;
		*(uintptr_t*)((uintptr_t)src + 1) = (uintptr_t)((uintptr_t)dst - (uintptr_t)src - 5);
		VirtualProtect(src, len, oldProc, 0);
	}

	BYTE* CopySetSrc(BYTE* src, BYTE* dst, int len)
	{
		BYTE* newmem = (BYTE*)VirtualAlloc(0, len,
			MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

		// 从wglSwaoBuffers函数开始拷贝len长度的字节
		memcpy_s(newmem, len, src, len);

		// jmp wglSwaoBuffers+len
		DWORD returnBytes = (DWORD)((uintptr_t)src - (uintptr_t)newmem - 5);
		*(BYTE*)((uintptr_t)newmem + len) = 0xE9;
		*(DWORD*)((uintptr_t)newmem + len + 1) = returnBytes;

		setHook(src, dst, len);
		return newmem;
	}
};

class FunHook
{
public:
	BYTE* src = 0;
	BYTE* dst = 0;
	BYTE* newmem = 0;
	int len = 0;

	BYTE originalBytes[10] = { 0 };

	FunHook(const char* modName, const char* exportName, BYTE* dst, BYTE* newmem, int len)
	{
		HMODULE hMod = GetModuleHandleA(modName);
		this->src = (BYTE*)GetProcAddress(hMod, exportName);
		this->dst = dst;
		this->newmem = newmem;
		this->len = len;
	}

	void Enable()
	{
		memcpy_s(this->originalBytes, this->len, this->src, this->len);	// 备份方便复原
		*(uintptr_t*)this->newmem = (uintptr_t)mem::CopySetSrc(this->src, this->dst, this->len);
	}

	void Disable()
	{
		memcpy_s(this->src, this->len, this->originalBytes, this->len);
	}
};

void Draw()
{
	GL::SetupOrtho();
	GL::DrawOutline(300, 300, 200, 200, 2.0f, rgb::red);
	GL::RestorGL();
}

typedef BOOL(_stdcall* twglSwaoBuffers)(HDC hDc);
twglSwaoBuffers cbwglSwapBuffers;

BOOL __stdcall hkwglSwapBUffers(HDC hDc)
{
	//cout << "Hook OK" << endl;
	Draw();
	return cbwglSwapBuffers(hDc);
}


int Mythread(HMODULE hModule)
{
	AllocConsole();
	FILE* _f;
	freopen_s(&_f, "CONOUT$", "w", stdout);
	cout << "INJECT OK" << endl;

	// wglSwapBuffsers
	// wglSwapBuffsers -> hook -> copy wglSwapBuffsers -> wglSwapBuffsers+5

	FunHook hook("opengl32.dll", "wglSwapBuffers", (BYTE*)hkwglSwapBUffers, (BYTE*)&cbwglSwapBuffers, 5);
	hook.Enable();

	while (true)
	{
		if (GetAsyncKeyState(VK_F12) & 1) break;
	}

	hook.Disable();
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
