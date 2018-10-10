/**
* Orion2 - A MapleStory2 Dynamic Link Library Localhost
*
* @author Eric
*
*/
#include "Orion.h"
#include "OrionHacks.h"

/* Initializing Memory Alterations */
BOOL bInitialized = FALSE;

/* CreateWindowExA hook used to rename the main window */
bool Hook_CreateWindowExA(bool bEnable) {
	static decltype(&CreateWindowExA) _CreateWindowExA = &CreateWindowExA;

	decltype(&CreateWindowExA) CreateWindowExA_Hook = [](DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam) -> HWND {

		if (strstr(lpClassName, "MapleStory2")) {
			//NotifyMessage("Orion2 has loaded all data successfully!\r\n\r\nPress OK to launch the game~", Orion::NotifyType::Information);

			return _CreateWindowExA(dwExStyle, lpClassName, CLIENT_NAME, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
		}

		return _CreateWindowExA(dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
	};

	return SetHook(bEnable, reinterpret_cast<void**>(&_CreateWindowExA), CreateWindowExA_Hook);
}

/* Best way to inject into client memory after class load: hook GetCurrentDirectoryA(). */
bool Hook_GetCurrentDirectoryA(bool bEnable) {
	static decltype(&GetCurrentDirectoryA) _GetCurrentDirectoryA = &GetCurrentDirectoryA;

	decltype(&GetCurrentDirectoryA) GetCurrentDirectoryA_Hook = [](DWORD nBufferLength, LPSTR lpBuffer) -> DWORD {
		/* Only initialize the memory alterations once.. */
		if (!bInitialized) {
			if (InitializeOrion2()) {
				bInitialized = TRUE;
			} else {
				return FALSE;
			}
		}

		return _GetCurrentDirectoryA(nBufferLength, lpBuffer);
	};

	return SetHook(bEnable, reinterpret_cast<void**>(&_GetCurrentDirectoryA), GetCurrentDirectoryA_Hook);
}

int LogReport(/*ZExceptionHandler *this,*/ const char *sFormat, ...) {
	CHAR sBuff[4096] = { 0 };
	va_list arglist;

	va_start(arglist, sFormat);

	int v2 = wvsprintfA((LPSTR)(&sBuff), sFormat, arglist);

	OutputDebugStringA(sBuff);

	va_end(arglist);

	return v2;
}

LONG WINAPI DumpUnhandledException(LPEXCEPTION_POINTERS pExceptionInfo) {
	LogReport("==== DumpUnhandledException ==============================\r\n");

	auto v6 = pExceptionInfo->ExceptionRecord;
	LogReport("Fault Address : %08X\r\n", v6->ExceptionAddress);
	LogReport("Exception code: %08X\r\n", v6->ExceptionCode);

	auto v8 = pExceptionInfo->ContextRecord;
	LogReport("Registers:\r\n");
	LogReport(
		"EAX:%08X\r\nEBX:%08X\r\nECX:%08X\r\nEDX:%08X\r\nESI:%08X\r\nEDI:%08X\r\n",
		v8->Eax, v8->Ebx, v8->Ecx,
		v8->Edx, v8->Esi, v8->Edi);
	LogReport("CS:EIP:%04X:%08X\r\n", v8->SegCs, v8->Eip);
	LogReport("SS:ESP:%04X:%08X  EBP:%08X\r\n", v8->SegSs, v8->Esp, v8->Ebp);
	LogReport("DS:%04X  ES:%04X  FS:%04X  GS:%04X\r\n", v8->SegDs, v8->SegEs, v8->SegFs, v8->SegGs);
	LogReport("Flags:%08X\r\n", v8->EFlags);

	LogReport("\r\n");

	return EXCEPTION_EXECUTE_HANDLER;
}

void InitUnhandledExceptionFilter() {
	SetUnhandledExceptionFilter(DumpUnhandledException);
}

/* A defaulted MessageBox */
void NotifyMessage(const char* sText) {
	NotifyMessage(sText, Orion::NotifyType::None);
}

/* A MessageBox with information/warning/error/etc icons and dialogues */
void NotifyMessage(const char* sText, int nType) {
	MessageBoxA(NULL, sText, CLIENT_NAME, nType);
}

/* A MessageBox with the allowed use of arguments, used for misc debugging */
void NotifyDbgMessage(const char* sFormat, ...) {
	char* sText = new char[MAX_BUFFER];
	va_list args;
	va_start(args, sFormat);
	vsnprintf(sText, MAX_BUFFER - 1, sFormat, args);

	MessageBoxA(NULL, sText, CLIENT_NAME, NULL);

	va_end(args);
	delete[] sText;
}

/* A wide-character(W) version of NotifyDbgMessage */
void NotifyDbgMessageW(const wchar_t* sFormat, ...) {
	wchar_t* sText = new wchar_t[MAX_BUFFER];
	va_list args;
	va_start(args, sFormat);
	_vsnwprintf(sText, MAX_BUFFER - 1, sFormat, args);

	MessageBoxW(NULL, sText, NULL, NULL);

	va_end(args);
	delete[] sText;
}

/* Attach or detach a detour hook on to a given target */
BOOL SetHook(__in BOOL bInstall, __inout PVOID* ppvTarget, __in PVOID pvDetour) {
	if (DetourTransactionBegin() != NO_ERROR)
		return FALSE;

	if (DetourUpdateThread(GetCurrentThread()) == NO_ERROR)
		if ((bInstall ? DetourAttach : DetourDetach)(ppvTarget, pvDetour) == NO_ERROR)
			if (DetourTransactionCommit() == NO_ERROR)
				return TRUE;

	DetourTransactionAbort();
	return FALSE;
}
