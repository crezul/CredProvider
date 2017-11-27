
#include "servicemain.h"

int _tmain(int argc, TCHAR *argv[])
{
	addLogMessage("Start main func");
	
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
		return -1;
	}
	addLogMessage("End main funcn");
	return 0;
}

VOID WINAPI ServiceMain(DWORD argc, LPTSTR *argv)
{
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

	// Wait until our worker thread exits signaling that the service needs to stop
	WaitForSingleObject(hThread, INFINITE); // ожидание каждых 5 сек можно поставить
	/*
	* Perform any cleanup tasks
	*/

	CloseHandle(g_ServiceStopEvent);

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
	addLogMessage("Start ServiceWorkerThread funcn");

	WSADATA Data;
	int status = WSAStartup(MAKEWORD(1, 1), &Data);
	if (status != 0)
	{
		addLogMessage("ERROR: WSAStartup unsuccessful");
		return  ERROR_SUCCESS;;
	}
	//socket setting
	unsigned __int64 s;
	SOCKADDR_IN servAdr;
	memset(&servAdr, 0, sizeof(servAdr));
	servAdr.sin_port = htons(87);
	
	servAdr.sin_family =  AF_INET; // AF_UNSPEC (любой тип АПИ протокола ИП4 ИП6)
	servAdr.sin_addr.s_addr = htonl(INADDR_ANY); //inet_addr("127.0.0.1");
		//htonl(INADDR_ANY);// htonl(INADDR_LOOPBACK);
												//INADDR_ANY;
												//    inet_addr("65.55.21.250");
	s = socket(AF_INET, SOCK_STREAM, 0);

	if (s == INVALID_SOCKET)
	{
		addLogMessage("ERROR: socket unsuccessful");
		return ERROR_SUCCESS;
	}
	if (servAdr.sin_addr.s_addr == INADDR_NONE) {
		addLogMessage("inet_addr не выполнен и возвращен INADDR_NONE ");
		WSACleanup();
		return ERROR_SUCCESS;
	}

	/*if (servAdr.sin_addr.s_addr == INADDR_ANY) {
		addLogMessage("inet_addr не выполнен и возвращен INADDR_ANY ");
		WSACleanup();
		return ERROR_SUCCESS;
	}*/
	if (bind(s, (SOCKADDR*)&servAdr, sizeof(servAdr)) == -1)
	{
		addLogMessage("ERROR: bind unsuccessful");
		return ERROR_SUCCESS;
	}
	addLogMessage("\n My adress is :"); addLogMessage(inet_ntoa(servAdr.sin_addr));
	listen(s, 10); // 10 - кол-во клиентов что могут подсоеденится 
				   //получаем в буфер индификатор
	HANDLE hThreadForClient = ERROR_CLIENT_THEARD; // переменная для потока обработки клиента

	SOCKADDR_IN from_sin;
	int from_len;
	from_len = sizeof(from_sin);

	while (WaitForSingleObject(g_ServiceStopEvent, 0) != WAIT_OBJECT_0) //1
	{
		fd_set s_set = { 1,{ s } }; // с цмкла вінести
		timeval timeout = { 0, 0 };//
		int select_res = select(0, &s_set, 0, 0, &timeout);
		if (select_res == SOCKET_ERROR)
		{
			addLogMessage("Select is field. Error of network!"); // для дебага
		}
		if (select_res)
		{
			addLogMessage("Waiting for a connection..."); // для дебага

			unsigned __int64 *s_new = new unsigned __int64(
				accept(s, (SOCKADDR*)&from_sin, &from_len));

			char socketstr[8];
			itoa(*s_new, socketstr, 10);
			addLogMessage("socket new client = "); addLogMessage(socketstr);
			if (*s_new < 0) //не глобальная ошибка
			{
				addLogMessage("ERROR. Acept with client field");
				WSACleanup();
			}
			else
			{
				addLogMessage("Client connect");
				// функция обработки сервера
				hThreadForClient = CreateThread(NULL, 0, ServerWorkerClient, (LPVOID)s_new, 0, NULL);

				if (hThreadForClient == ERROR_CLIENT_THEARD)
				{
					addLogMessage("Client theard didnt created"); // не глобальная ошибка
					CloseHandle(hThreadForClient);
					WSACleanup();
				}
			}
			//очистка буферов и сокета принятого клиента 
			//from_len = 0;
			//memset(&from_sin, 0, sizeof(from_sin));
		}
	}

	addLogMessage("END: Function servicetheard is finished!!");
	closesocket(s);
	WSACleanup();
	return SEC_E_OK;
}





int InstallService() {
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
	addLogMessage("RemoveService func");
	//stopservice
	if (g_ServiceStatus.dwCurrentState == SERVICE_RUNNING)
	{

		SetEvent(g_ServiceStopEvent);

		g_ServiceStatus.dwControlsAccepted = 0;
		g_ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
		g_ServiceStatus.dwWin32ExitCode = 0;
		g_ServiceStatus.dwCheckPoint = 4;

		if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
		{
			addLogMessage(
				" Service: ServiceCtrlHandler: SetServiceStatus returned error in stopped section");
			return -1;
		}

		addLogMessage("Function Remove is finished!");
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

// страный старт что то стартуеться и даже работает но не заходит в мои потоки и тд 
//дебаг
//
//int StartService_()
//{
//	addLogMessage("StartService_ func");
//
//	SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
//	if (!hSCManager) {
//		addLogMessage("Error: Can't open Service Control Manager");
//		return -1;
//	}
//
//	SC_HANDLE hService = OpenService(hSCManager, SERVICE_NAME, SERVICE_ALL_ACCESS);
//	if (!hSCManager)
//	{
//		CloseServiceHandle(hSCManager);
//		addLogMessage("Error: Can't open Service Control Manager");
//		return -1;
//	}
//	DWORD dwOldCheckPoint;
//	DWORD dwStartTickCount;
//	DWORD dwWaitTime;
//	DWORD dwBytesNeeded;
//	if (!QueryServiceStatusEx(
//		hService,                     // handle to service 
//		SC_STATUS_PROCESS_INFO,         // information level
//		(LPBYTE)&g_ServiceStatus,             // address of structure
//		sizeof(SERVICE_STATUS_PROCESS), // size of structure
//		&dwBytesNeeded))              // size needed if buffer is too small
//	{
//		addLogMessage("QueryServiceStatusEx failed ");
//		CloseServiceHandle(hService);
//		CloseServiceHandle(hSCManager);
//		return -1;
//	}
//
//	if (g_ServiceStatus.dwCurrentState != SERVICE_STOPPED &&
//					g_ServiceStatus.dwCurrentState != SERVICE_STOP_PENDING)
//	{
//		CloseServiceHandle(hService);
//		CloseServiceHandle(hSCManager);
//		addLogMessage("Cannot start the service because it is already running");
//		return -1;
//	}
//
//	
//	// Save the tick count and initial checkpoint.
//
//	dwStartTickCount = GetTickCount();
//	dwOldCheckPoint = g_ServiceStatus.dwCheckPoint;
//
//	// Wait for the service to stop before attempting to start it.
//
//	while (g_ServiceStatus.dwCurrentState == SERVICE_STOP_PENDING)
//	{
//		// Do not wait longer than the wait hint. A good interval is 
//		// one-tenth of the wait hint but not less than 1 second  
//		// and not more than 10 seconds. 
//
//		dwWaitTime = g_ServiceStatus.dwWaitHint / 10;
//
//		if (dwWaitTime < 1000)
//			dwWaitTime = 1000;
//		else if (dwWaitTime > 10000)
//			dwWaitTime = 10000;
//
//		Sleep(dwWaitTime);
//
//		// Check the status until the service is no longer stop pending. 
//
//		if (!QueryServiceStatusEx(
//			hService,                     // handle to service 
//			SC_STATUS_PROCESS_INFO,         // information level
//			(LPBYTE)&g_ServiceStatus,             // address of structure
//			sizeof(SERVICE_STATUS_PROCESS), // size of structure
//			&dwBytesNeeded))              // size needed if buffer is too small
//		{
//			addLogMessage("QueryServiceStatusEx failed ");
//			CloseServiceHandle(hService);
//			CloseServiceHandle(hSCManager);
//			return -1;
//		}
//
//		if (g_ServiceStatus.dwCheckPoint > dwOldCheckPoint)
//		{
//			// Continue to wait and check.
//
//			dwStartTickCount = GetTickCount();
//			dwOldCheckPoint = g_ServiceStatus.dwCheckPoint;
//		}
//		else
//		{
//			if (GetTickCount() - dwStartTickCount > g_ServiceStatus.dwWaitHint)
//			{
//				addLogMessage("Timeout waiting for service to stop");
//				CloseServiceHandle(hService);
//				CloseServiceHandle(hSCManager);
//				return -1;
//			}
//		}
//	}
//
//	if (!StartService(hService, 0, NULL))
//	{
//		CloseServiceHandle(hService);
//		CloseServiceHandle(hSCManager);
//		addLogMessage("Error: Can't start service");
//		return -1;
//	}
//	else addLogMessage("Service start pending...");
//
//	// Check the status until the service is no longer start pending. 
//
//	if (!QueryServiceStatusEx(
//		hService,                     // handle to service 
//		SC_STATUS_PROCESS_INFO,         // info level
//		(LPBYTE)&g_ServiceStatus,             // address of structure
//		sizeof(SERVICE_STATUS_PROCESS), // size of structure
//		&dwBytesNeeded))              // if buffer too small
//	{
//		printf("QueryServiceStatusEx failed (%d)\n", GetLastError());
//		CloseServiceHandle(hService);
//		CloseServiceHandle(hSCManager);
//		return -1;
//	}
//
//	// Save the tick count and initial checkpoint.
//
//	dwStartTickCount = GetTickCount();
//	dwOldCheckPoint = g_ServiceStatus.dwCheckPoint;
//
//	while (g_ServiceStatus.dwCurrentState == SERVICE_START_PENDING)
//	{
//		// Do not wait longer than the wait hint. A good interval is 
//		// one-tenth the wait hint, but no less than 1 second and no 
//		// more than 10 seconds. 
//
//		dwWaitTime = g_ServiceStatus.dwWaitHint / 10;
//
//		if (dwWaitTime < 1000)
//			dwWaitTime = 1000;
//		else if (dwWaitTime > 10000)
//			dwWaitTime = 10000;
//
//		Sleep(dwWaitTime);
//
//		// Check the status again. 
//
//		if (!QueryServiceStatusEx(
//			hService,             // handle to service 
//			SC_STATUS_PROCESS_INFO, // info level
//			(LPBYTE)&g_ServiceStatus,             // address of structure
//			sizeof(SERVICE_STATUS_PROCESS), // size of structure
//			&dwBytesNeeded))              // if buffer too small
//		{
//			addLogMessage("QueryServiceStatusEx failed");
//			break;
//		}
//
//		if (g_ServiceStatus.dwCheckPoint > dwOldCheckPoint)
//		{
//			// Continue to wait and check.
//
//			dwStartTickCount = GetTickCount();
//			dwOldCheckPoint = g_ServiceStatus.dwCheckPoint;
//		}
//		else
//		{
//			if (GetTickCount() - dwStartTickCount > g_ServiceStatus.dwWaitHint)
//			{
//				// No progress made within the wait hint.
//				break;
//			}
//		}
//	}
//
//	// Determine whether the service is running.
//
//	if (g_ServiceStatus.dwCurrentState == SERVICE_RUNNING)
//	{
//		addLogMessage("Service started successfully");
//	}
//	else
//	{
//		printf("Service not started. \n");
//		printf("  Current State: %d\n", g_ServiceStatus.dwCurrentState);
//		printf("  Exit Code: %d\n", g_ServiceStatus.dwWin32ExitCode);
//		printf("  Check Point: %d\n", g_ServiceStatus.dwCheckPoint);
//		printf("  Wait Hint: %d\n", g_ServiceStatus.dwWaitHint);
//	}
//
//	CloseServiceHandle(hService);
//	CloseServiceHandle(hSCManager);
//	addLogMessage("Start sucess");
//	return 0;
//}