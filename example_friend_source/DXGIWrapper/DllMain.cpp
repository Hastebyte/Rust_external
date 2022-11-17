#include "Headers.h"

DWORD WINAPI WaitingLooper(LPVOID)
{

}

VOID CreateConsole()
{
	if (!AllocConsole()) {
		FreeConsole();
		AllocConsole();
	}

	AttachConsole(GetCurrentProcessId());
	freopen("Con", "w", stdout);
	SetConsoleTitleA("Vito");
}

DWORD WINAPI DllMain(HMODULE hDll, DWORD dwReasonForCall, LPVOID lpReserved)
{
	if (dwReasonForCall == DLL_PROCESS_ATTACH)
	{
		DisableThreadLibraryCalls(hDll);

		CreateConsole();

		DirectXHook::D_OnLoad();

		return 1;
	}
	else if (dwReasonForCall == DLL_PROCESS_DETACH)
	{
		return 1;
	}

	return 1;
}
