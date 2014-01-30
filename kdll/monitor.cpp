#include "project.h"
#include "monitor.h"
#include "debug.h"
#include "screenshot.h"
#include "device.h"

#define PAGE_SIZE 4096

static MONITOR g_Monitor;

PMONITOR 
	GetMonitor()
{
	return &g_Monitor;
}

typedef
BOOL
(WINAPI *PCLIENT_THREAD_SETUP)(VOID);

VOID
	PrepareMainThread()
{
	PMONITOR Monitor = GetMonitor();
	HMODULE hModule = NULL;
	HWINSTA hWinsta = NULL;
	HDESK hDesk = NULL;
	PCLIENT_THREAD_SETUP ClientThreadSetup = NULL;

	hModule = LoadLibrary(L"user32.dll");
	if (hModule == NULL) {
		DebugPrint("LoadLibrary failed\n");
		return;
	}
	
	DebugPrint("IsGUIThread=%x\n", IsGUIThread(TRUE));

	ClientThreadSetup = (PCLIENT_THREAD_SETUP)GetProcAddress(hModule, "ClientThreadSetup");
	if (ClientThreadSetup == NULL) {
		DebugPrint("ClientThreadSetup not found in mod=%p\n", hModule);
		goto cleanup;
	}
	
	BOOL Result = ClientThreadSetup();
	DebugPrint("ClientThreadSetup=%x\n", Result);
	hWinsta = DeviceOpenWinsta(L"WinSta0");
	if (hWinsta != NULL) {
		hDesk = DeviceOpenDesktop(hWinsta, L"Default");
	}
	
	DebugPrint("Opened hwinsta=%p, hdesk=%p\n", hWinsta, hDesk);

	if (hDesk != NULL) {
		if (!SetThreadDesktop(hDesk)) {
			DebugPrint("SetThreadDesktop failed, error=%d\n", GetLastError());
		}
	}

	if (hDesk != NULL)
		CloseDesktop(hDesk);

	if (hWinsta != NULL)
		CloseWindowStation(hWinsta);
	
cleanup:	
	FreeLibrary(hModule);
}


VOID
	GetKbdLayout()
{
	WCHAR kbdLayout[MAX_PATH];
	memset(kbdLayout, 0, sizeof(kbdLayout));
	if (GetKeyboardLayoutName(kbdLayout))
		DebugPrint("kbdLayout=%ws\n", kbdLayout);
	else
		DebugPrint("GetKeyboardLayoutName failed with err=%d", GetLastError());
}

DWORD 
WINAPI
	MonitorMainThreadRoutine(
	_In_  LPVOID lpParameter
)
{
	PMONITOR Monitor = (PMONITOR)lpParameter;

	DebugPrint("Monitor thread starting processId=%x, threadId=%x\n", GetCurrentProcessId(), GetCurrentThreadId());
	
	PrepareMainThread();


	GetKbdLayout();
	while (!Monitor->Stopping) {
		GetKbdLayout();
		CaptureScreenCallback();
		Sleep(30000);
	}


	DebugPrint("Monitor thread exiting processId=%x, threadId=%x\n", GetCurrentProcessId(), GetCurrentThreadId());
	return 0;
}

BOOL
	MonitorStart(PMONITOR Monitor)
{
	memset(Monitor, 0, sizeof(MONITOR));
	Monitor->Stopping = 0;
	Monitor->MainThreadHandle = CreateThread(NULL, 256 * PAGE_SIZE, MonitorMainThreadRoutine, GetMonitor(), 0, &Monitor->MainThreadId);
	if (Monitor->MainThreadHandle == NULL) {
		return FALSE;
	}

	return TRUE;
}

VOID
	MonitorStop(PMONITOR Monitor)
{
	Monitor->Stopping = 1;
	if (Monitor->MainThreadHandle != NULL) {
		WaitForSingleObject(Monitor->MainThreadHandle, INFINITE);
		CloseHandle(Monitor->MainThreadHandle);
		Monitor->MainThreadHandle = NULL;
	}
	Monitor->MainThreadId = 0;
}