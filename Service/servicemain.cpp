
#include "servicemain.h"

int _tmain(int argc, TCHAR *argv[])
{
	if (isfstream())
		addLogMessage("Stream in: ");
	addLogMessage(fstremname);
	WriteTime();
	addLogMessage("Start main function");

		if (wcscmp(argv[argc - 1], _T("install")) == 0)
	{
		InstallService();
		return 0;
	}
	else 
		if (wcscmp(argv[argc - 1], _T("remove")) == 0) 
	{
		RemoveService();
		return 0;
	}


	SERVICE_TABLE_ENTRY ServiceTable[] =
	{
		{ SERVICE_NAME, (LPSERVICE_MAIN_FUNCTION)ServiceMain },
		{ NULL, NULL }
	};

	if (StartServiceCtrlDispatcher(ServiceTable) == FALSE)
	{
		addLogMessage("In MAIN funcn error: StartServiceCtrlDispatcher(ServiceTable) = FALSE");
		return 0;
	}
	addLogMessage("End main funcn");
	return 0;
}

VOID WINAPI ServiceMain(DWORD argc, LPTSTR *argv)
{
	WriteTime();
	DWORD Status = E_FAIL;
	addLogMessage("Start ServiceMain funcn");
	// Register our service control handler with the SCM
	g_StatusHandle = RegisterServiceCtrlHandler(SERVICE_NAME, ServiceCtrlHandler);

	if (g_StatusHandle == NULL)
	{
		addLogMessage("RegisterServiceCtrlHandler field");
		return;
	}

	// Tell the service controller we are starting
	ZeroMemory(&g_ServiceStatus, sizeof(g_ServiceStatus));
	g_ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	g_ServiceStatus.dwControlsAccepted = 0;
	g_ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
	g_ServiceStatus.dwWin32ExitCode = 0;
	g_ServiceStatus.dwServiceSpecificExitCode = 0;
	g_ServiceStatus.dwCheckPoint = 0;

	if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
	{
		addLogMessage(
			"Service: ServiceMain: SetServiceStatus returned error");
	}

	/*
	* Perform tasks necessary to start the service here
	*/

	// Create a service stop event to wait on later
	g_ServiceStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	HandleGuard hServiceStopEvent2Guard(g_ServiceStopEvent);

	if (g_ServiceStopEvent == NULL)
	{
		// Error creating event
		// Tell service controller we are stopped and exit
		g_ServiceStatus.dwControlsAccepted = 0;
		g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
		g_ServiceStatus.dwWin32ExitCode = GetLastError();
		g_ServiceStatus.dwCheckPoint = 1;

		if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
		{
			addLogMessage(
				"Service: ServiceMain: SetServiceStatus returned error. StopEvent = NULL");
		}
		return;
	}

	// Tell the service controller we are started
	g_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
	g_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
	g_ServiceStatus.dwWin32ExitCode = 0;
	g_ServiceStatus.dwCheckPoint = 0;

	if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
	{
		addLogMessage(
			" Service: ServiceMain: SetServiceStatus returned error");
	}

	// Start a thread that will perform the main task of the service
	HANDLE hThread = CreateThread(NULL, 0, ServiceWorkerThread, NULL, 0, NULL);

	HandleGuard hThread2Guard(hThread); // guard


	// Wait until our worker thread exits signaling that the service needs to stop
	WaitForSingleObject(hThread, INFINITE); // ожидание каждых 5 сек можно поставить
	/*
	* Perform any cleanup tasks
	*/


	// Tell the service controller we are stopped
	g_ServiceStatus.dwControlsAccepted = 0;
	g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
	g_ServiceStatus.dwWin32ExitCode = 0;
	g_ServiceStatus.dwCheckPoint = 3;

	if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
	{
		addLogMessage(
			" Service: ServiceMain: SetServiceStatus returned error");
	}
	addLogMessage("END ServiceMain funcn");
	return;
}

VOID WINAPI ServiceCtrlHandler(DWORD CtrlCode)
{
	WriteTime();
	//addLogMessage("ServiceCtrlHandler start");
	switch (CtrlCode)
	{
	case SERVICE_CONTROL_SHUTDOWN:
	case SERVICE_CONTROL_STOP:
		addLogMessage("STOPPED");
		if (g_ServiceStatus.dwCurrentState != SERVICE_RUNNING)
		{
			addLogMessage("Service status in stopped function !=SERVICE_RUNNING");
			break;
		}

		/*
		* Perform tasks necessary to stop the service here
		*/
		// This will signal the worker thread to start shutting down

		SetEvent(g_ServiceStopEvent);

		g_ServiceStatus.dwControlsAccepted = 0;
		g_ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
		g_ServiceStatus.dwWin32ExitCode = 0;
		g_ServiceStatus.dwCheckPoint = 4;

		if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
		{
			addLogMessage(
				" Service: ServiceCtrlHandler: SetServiceStatus returned error in stopped section");
		}
	
		addLogMessage("Function STOPSERVICE is finished!");
		break;

	default:
		break;
	}
}



DWORD WINAPI ServiceWorkerThread(LPVOID lpParam)
{
	WriteTime();
	addLogMessage("Start ServiceWorkerThread funcn");

	WSADATA Data;
	int status = WSAStartup(MAKEWORD(1, 1), &Data);
	if (status != 0)
	{
		addLogMessage("ERROR: WSAStartup unsuccessful");
		WSACleanup();
		return  -1;
	}
	//socket setting
	unsigned __int64 s;
	SOCKADDR_IN servAdr;
	memset(&servAdr, 0, sizeof(servAdr));
	servAdr.sin_port = htons(87);
	
	servAdr.sin_family =  AF_INET;
	servAdr.sin_addr.s_addr = INADDR_ANY; //  inet_addr("192.168.0.100");   htonl(INADDR_LOOPBACK);
	s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	
	if (s == INVALID_SOCKET)
	{
		addLogMessage("ERROR: socket unsuccessful");
		WSACleanup();
		return -1;
	}
	if (servAdr.sin_addr.s_addr == INADDR_NONE) {
		addLogMessage("inet_addr return INADDR_NONE ");
		WSACleanup();
		return -1;
	}
	

	if (bind(s, (SOCKADDR*)&servAdr, sizeof(servAdr)) == -1)
	{
		addLogMessage("ERROR: bind unsuccessful");
		WSACleanup();
		return -1;
	}
	addLogMessage(" My adress is :"); addLogMessage(inet_ntoa(servAdr.sin_addr));
	listen(s, SOMAXCONN);	// SOMAXCONN - number of clients that can connect
					// Get the Indicator in the buffer
	

	SOCKADDR_IN from_sin;
	int from_len;
	from_len = sizeof(from_sin);

	while (WaitForSingleObject(g_ServiceStopEvent, 0) != WAIT_OBJECT_0) //1
	{
		fd_set s_set = { 1,{ s } }; // for out cicl
		timeval timeout = { 0, 0 };//
		int select_res = select(0, &s_set, 0, 0, &timeout);
		if (select_res == SOCKET_ERROR)
		{
			addLogMessage("Select is field. Error of network!"); //debug
		}
		if (select_res)
		{
			addLogMessage("Waiting for a connection..."); // debug

			unsigned __int64 *s_new = new unsigned __int64(
				accept(s, (SOCKADDR*)&from_sin, &from_len));

			addLogMessage("Connected client ip = "); 
			addLogMessage(inet_ntoa(from_sin.sin_addr));
		
			addLogMessage("Socket new client = "); addLogMessage(*s_new);
			if (*s_new < 0) // not a global error
			{
				addLogMessage("ERROR. Acept with client field");
				WSACleanup();
			}
			else
			{
				addLogMessage("Client connect");
				// server processing function
				HANDLE hThreadForClient = ERROR_CLIENT_THEARD;// variable for client processing flow

				hThreadForClient = CreateThread(NULL, 0, ServerWorkerClient, (LPVOID)s_new, 0, NULL);


				if (hThreadForClient == ERROR_CLIENT_THEARD)
				{
					addLogMessage("Client theard didnt created"); // not a global error

					WSACleanup();
				}
				HandleGuard hThreadForClientGuard(hThreadForClient); //guard for theard
			}
		}

	}

	closesocket(s);
	WSACleanup();
	addLogMessage("END: Function servicetheard is finished!!");
	return 0;
}





int InstallService() {
	WriteTime();
	addLogMessage("Install block");

	wchar_t szPath[MAX_PATH];


	if (GetModuleFileName(NULL, szPath, ARRAYSIZE(szPath)) == 0)
	{
		addLogMessage("Error: Can't get Module File Name");
		return -1;
	}

	SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
	if (!hSCManager) {
		addLogMessage("Error: Can't open Service Control Manager");
		return -1;
	}

	SC_HANDLE hService = CreateService(
		hSCManager,
		SERVICE_NAME,
		SERVICE_NAME,
		SERVICE_ALL_ACCESS,
		SERVICE_WIN32_OWN_PROCESS,
		SERVICE_AUTO_START, // autoload
		SERVICE_ERROR_NORMAL,
		szPath,
		NULL, NULL, NULL, NULL, NULL
	);

	if (!hService) {
		int err = GetLastError();
		switch (err) {
		case ERROR_ACCESS_DENIED:
			addLogMessage("Error: ERROR_ACCESS_DENIED");
			break;
		case ERROR_CIRCULAR_DEPENDENCY:
			addLogMessage("Error: ERROR_CIRCULAR_DEPENDENCY");
			break;
		case ERROR_DUPLICATE_SERVICE_NAME:
			addLogMessage("Error: ERROR_DUPLICATE_SERVICE_NAME");
			break;
		case ERROR_INVALID_HANDLE:
			addLogMessage("Error: ERROR_INVALID_HANDLE");
			break;
		case ERROR_INVALID_NAME:
			addLogMessage("Error: ERROR_INVALID_NAME");
			break;
		case ERROR_INVALID_PARAMETER:
			addLogMessage("Error: ERROR_INVALID_PARAMETER");
			break;
		case ERROR_INVALID_SERVICE_ACCOUNT:
			addLogMessage("Error: ERROR_INVALID_SERVICE_ACCOUNT");
			break;
		case ERROR_SERVICE_EXISTS:
			addLogMessage("Error: ERROR_SERVICE_EXISTS");
			break;
		default:
			addLogMessage("Error: Undefined");
		}
		CloseServiceHandle(hSCManager);
		return -1;
	}
	CloseServiceHandle(hService);

	CloseServiceHandle(hSCManager);
	addLogMessage("Success install service!");
	return 0;
}

int RemoveService() {
	WriteTime();
	addLogMessage("RemoveService func");
	//stopservice
	if (g_ServiceStatus.dwCurrentState == SERVICE_RUNNING)
	{

		SetEvent(g_ServiceStopEvent);
	}

	SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (!hSCManager) {
		addLogMessage("Error: Can't open Service Control Manager");
		return -1;
	}
	SC_HANDLE hService = OpenService(hSCManager, SERVICE_NAME, SC_MANAGER_ALL_ACCESS);
	if (!hService) {
		addLogMessage("Error: Can't remove service");
		CloseServiceHandle(hSCManager);
		return -1;
	}

	if (!DeleteService(hService))
	{
		addLogMessage("Remove field");
	}
	else
	{
		addLogMessage("Success remove service!");
	}
	CloseServiceHandle(hService);
	CloseServiceHandle(hSCManager);
	return 0;
}

