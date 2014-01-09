// dllmain.cpp : Defines the entry point for the DLL application.
#include "project.h"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		OutputDebugStringA("DLL_PROCESS_ATTACH\n");
		break;
	case DLL_THREAD_ATTACH:
		OutputDebugStringA("DLL_THREAD_ATTACH\n");
		break;
	case DLL_THREAD_DETACH:
		OutputDebugStringA("DLL_THREAD_DETACH\n");
		break;
	case DLL_PROCESS_DETACH:
		OutputDebugStringA("DLL_PROCESS_DETACH\n");
		break;
	}

	return TRUE;
}

