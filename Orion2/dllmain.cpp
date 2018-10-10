/**
* Orion2 - A MapleStory2 Dynamic Link Library Localhost
*
* @author Eric
*
*/
#include "Orion.h"
#include "OrionHacks.h"
#include "WinSockHook.h"
#include "NMCOHook.h"

void MapleStory2_Bypass(void) {
	while (true) {
		bool bInit = InitializeOrion2();
		if (bInit) {
			printf("Attempting to initialize Orion2... (Success!)\n");
			break;
		}
		printf("Attempting to initialize Orion2... (Failed)\n");
	}
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved) {
	/* Initial injection from process attach. */
	if (dwReason == DLL_PROCESS_ATTACH) {
		/* Update the locale to properly decode foreign StringPools. */
		setlocale(LC_ALL, STRING_LOCALE);

#if DEBUG_MODE
			NotifyMessage("Injection successful - haulting process for memory alterations.", Orion::NotifyType::Information);

			AllocConsole();
			SetConsoleTitleA("Orion 2.0 - Make Orion Great Again");
			AttachConsole(GetCurrentProcessId());
			freopen("CON", "w", stdout);
#endif

		/* Initiate the winsock hook for socket spoofing and redirection. */
		if (!Hook_WSPStartup(true)) {
			NotifyMessage("Failed to hook WSPStartup", Orion::NotifyType::Error);
			return FALSE;
		}

		/* Initiate the NMCO hook to fix our login passport handling. */
		if (!NMCOHook_Init()) {
			NotifyMessage("Failed to hook CallNMFunc", Orion::NotifyType::Error);
			return FALSE;
		}

		/* Initiate the CreateWindowExA hook to customize the main window. */
		if (!Hook_CreateWindowExA(true)) {
			NotifyMessage("Failed to hook CreateWindowExA", Orion::NotifyType::Error);
			return FALSE;
		}

		/* Hook GetCurrentDirectoryA and begin to rape the client. */
		if (!Hook_GetCurrentDirectoryA(true)) {
			NotifyMessage("Failed to hook GetCurrentDirectoryA", Orion::NotifyType::Error);
			return FALSE;
		}

		CreateThread(0, 0, (LPTHREAD_START_ROUTINE)MapleStory2_Bypass, 0, 0, 0);

		DisableThreadLibraryCalls(hModule);
	} else if (dwReason == DLL_PROCESS_DETACH) {
		FreeConsole();
	}

	return TRUE;
}
