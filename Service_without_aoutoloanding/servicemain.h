
#pragma once 
#include "inclfiles.h"


#define SERVICE_NAME  _T("Android CredentialProvider Service")
#define ERROR_CLIENT_THEARD NULL


static SERVICE_STATUS        g_ServiceStatus = { 0 };
static SERVICE_STATUS_HANDLE g_StatusHandle = NULL;
static HANDLE                g_ServiceStopEvent = INVALID_HANDLE_VALUE;


VOID WINAPI ServiceMain(DWORD argc, LPTSTR *argv);
VOID WINAPI ServiceCtrlHandler(DWORD);

DWORD WINAPI ServiceWorkerThread(LPVOID);


int InstallService();
int RemoveService();

