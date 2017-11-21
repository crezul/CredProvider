
#include "inclfiles.h"


SERVICE_STATUS        g_ServiceStatus = { 0 };
SERVICE_STATUS_HANDLE g_StatusHandle = NULL;
HANDLE                g_ServiceStopEvent = INVALID_HANDLE_VALUE;

VOID WINAPI ServiceMain(DWORD argc, LPTSTR *argv);
VOID WINAPI ServiceCtrlHandler(DWORD);
DWORD WINAPI ServiceWorkerThread(LPVOID lpParam);
DWORD WINAPI ServerWorkerClient(LPVOID lpParam);
BOOL SpeakWithPipe(char*, char*);
void addLogMessage(char *);
void addLogMessageW(LPTSTR ); // to UNICODE MESSAGE
BOOL GetCurrentUser(LPTSTR &, DWORD &);

#define SERVICE_NAME  _T("My Sample Service")
#define ERROR_CLIENT_THEARD NULL


void addLogMessageW(LPTSTR un)
{
	char ch[40]="";
		WideCharToMultiByte(CP_ACP, 0, un, -1, ch, 40, 0, 0);
	fstream  fout("C:\\Users\\Danil\\Desktop\\ServiceCredProvLogFile.txt", ios::out | ios::app);
	time_t tt;
	tm* tp;
	tt = time(0);
	tp = localtime(&tt);
	fout << asctime(tp) << '\t' << ch << '\n';
	fout.close();
	return;
}
void addLogMessage(char *ch)
{
	fstream  fout("C:\\Users\\Danil\\Desktop\\ServiceCredProvLogFile.txt", ios::out | ios::app);
	time_t tt;
	tm* tp;
	tt = time(0);
	tp = localtime(&tt);
	fout << asctime(tp) << '\t' << ch << '\n';
	fout.close();
	return;
}

int _tmain(int argc, TCHAR *argv[])
{
	SERVICE_TABLE_ENTRY ServiceTable[] =
	{
		{ SERVICE_NAME, (LPSERVICE_MAIN_FUNCTION)ServiceMain },
		{ NULL, NULL }
	};

	if (StartServiceCtrlDispatcher(ServiceTable) == FALSE)
	{
		addLogMessage("In MAIN funcn error: StartServiceCtrlDispatcher(ServiceTable) = FALSE");
		return GetLastError();
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
			"My Sample Service: ServiceMain: SetServiceStatus returned error");
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
				"My Sample Service: ServiceMain: SetServiceStatus returned error. StopEvent = NULL");
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
			"My Sample Service: ServiceMain: SetServiceStatus returned error");
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
			"My Sample Service: ServiceMain: SetServiceStatus returned error");
	}
	addLogMessage("END ServiceMain funcn");
	return;
}

VOID WINAPI ServiceCtrlHandler(DWORD CtrlCode)
{
	addLogMessage("Start ServiceCtrlHandler funcn");
	switch (CtrlCode)
	{
	case SERVICE_CONTROL_STOP:
		addLogMessage("Start SERVICE_CONTROL_STOP");
		if (g_ServiceStatus.dwCurrentState != SERVICE_RUNNING)
		{
			addLogMessage("Service status if stopped function !=SERVICE_RUNNING");
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
				"My Sample Service: ServiceCtrlHandler: SetServiceStatus returned error in stopped section");
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
	int s;
	SOCKADDR_IN servAdr;
	memset(&servAdr, 0, sizeof(servAdr));
	servAdr.sin_port = htons(4000);
	servAdr.sin_family = AF_INET;
	servAdr.sin_addr.s_addr = htonl(INADDR_ANY);// htonl(INADDR_LOOPBACK);
	//INADDR_ANY;
	//    inet_addr("65.55.21.250");
	s = socket(AF_INET, SOCK_STREAM, 0);
	if (s == INVALID_SOCKET)
	{
		addLogMessage("ERROR: socket unsuccessful");
		return ERROR_SUCCESS;
	}
	if (bind(s, (SOCKADDR*)&servAdr, sizeof(servAdr)) == -1)
	{
		addLogMessage("ERROR: bind unsuccessful");
		return ERROR_SUCCESS;
	}
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
				from_len = 0;
				memset(&from_sin, 0, sizeof(from_sin));
			}
		}

			addLogMessage("END: Function servicetheard is finished!!");
			closesocket(s);
		   	WSACleanup();
	return SEC_E_OK;
}
//DWORD WINAPI ServerWorkerClient(LPVOID lpParam)
//{
//	addLogMessage("Start ServerWorkerClient funcn");
//	unsigned __int64 * socket_client = (unsigned __int64*)lpParam;
//	//for debug client socket
//	char socketstr[8];
//	itoa(*socket_client, socketstr, 10);
//	addLogMessage("socket  client in ServerWorkerClient = "); addLogMessage(socketstr);
//	char message[127] = ""; // можно денамически выделять память исходя из размера байтРидера
//	int bytes_read=0;
//	//bool b = TRUE; // для цикла но мы ожидаем конец события
//	//while (WaitForSingleObject(g_ServiceStopEvent, 0) != WAIT_OBJECT_0) //b 
//	//{
//		bytes_read = recv(*socket_client, message, sizeof(message), 0); // message будет json
//		if (bytes_read == 0) // потом 1 пакет будет идти как сообщение для размера передаваемых данных  
//		{
//			addLogMessage("ERROR. Bytes reader is field or error of network");
//			//ExitThread(-1);
//		}
//		addLogMessage("Massage from client is accepted:");
//		//debag mod (not read)
//		int lensms = strlen(message);
//		char * sms = new char[lensms];
//		itoa(bytes_read, socketstr, 10);
//		sms = message;
//		addLogMessage(sms); // для дебага!
//		addLogMessage("Long message = "); addLogMessage(socketstr);
//		bytes_read = 0;
////	b = FALSE; //выход с цикла (он нужен весь прием должен быть в цикле хотя ресв принимает все в своем цикле )																	   //посылаем результат смс провайдеру!
//	//}
//		// список активных сессий 
//		  // если клиент хочет залогиниться то у нас идет проверка 
//			 //  кто чейчас на ПК (залогиненый ли пользыватель?)
//			//   если нет то посыл данных в КП иначе ответ список активных сессий
//		LPTSTR userDomenName; 
//		DWORD sessionid;
//		if (GetCurrentUser(userDomenName, sessionid))
//		{
//			addLogMessageW(userDomenName);
//			addLogMessage("function GetWhoSessionIs return true!");
//		}
//		if (sessionid !=0)
//		{
//			// ответ клиенту что что уже есть активный пользыватель! передача имени и домена пользывателя
//
//		}
//		else
//		{
//			// общение с провайдером и принятие у него решение !
//
//		//GetCurrentUser();
//	//передача по каналу сообщения провайдеру!
//			LPTSTR lpszPipename = TEXT("\\\\.\\pipe\\pipeforcp");
//			char bufferRequest[255] = ""; // буфер для принятия ответа
//			HANDLE hPipe = INVALID_HANDLE_VALUE;
//			int BUFSIZE = 2048; //размер буфера
//			bool fConnected = FALSE; // соеенение с каналом 
//			BOOL fSuccess = FALSE; // логическая переменная для writepipe
//			hPipe = CreateNamedPipe(
//				lpszPipename,             // pipe name 
//				PIPE_ACCESS_DUPLEX,       // read/write access 
//				PIPE_TYPE_MESSAGE |       // message type pipe 
//				PIPE_READMODE_MESSAGE |   // message-read mode 
//				PIPE_WAIT,                // blocking mode 
//				10,							// max. instances  == listen_socket
//				BUFSIZE,                  // output buffer size 
//				BUFSIZE,                  // input buffer size 
//				0,                        // client time-out 
//				NULL);                    // default security attribute 
//
//			if (hPipe == INVALID_HANDLE_VALUE)
//			{
//				addLogMessage("CreateNamedPipe failed in ClientTherd");
//				CloseHandle(hPipe); // rewrite
//				ExitThread(-1);
//			}
//			// writefile! - сделать так чтоб дискриптор доступа ждал ответа от клиента! - супер важно!
//			addLogMessage("Wait pipe connection");
//			fConnected = ConnectNamedPipe(hPipe, NULL) ?
//				TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);
//			// CP заупситлся и принял нас - тоесть подключился к нам
//			if (fConnected)
//			{
//				DWORD  writtenBytes = 0; // записаных в канал байтов
//				DWORD bufferSizeWrite = 2048; //-нужно динамически редактировать эту переменную
//				addLogMessage("CP was connected, start to write in pipe..."); // debugMessage
//																			  // Loop until done reading
//
//				if (!WriteFile(
//					hPipe,        // handle to pipe 
//					message,     // buffer to write from 
//					bufferSizeWrite, // number of bytes to write  - нужно динамически редактировать эту переменную 
//					&writtenBytes,   // number of bytes written 
//					NULL))      // not overlapped I/O  )
//				{
//					addLogMessage("SocketWorkerThread WriteFile failed.");
//					CloseHandle(hPipe);
//					//ExitThread(-1);
//				}
//				// чтение ответа с Провайдера (ответ будет 1) да + список активных сессий 2) нет 
//				//вобщем не больше структуры запросаАктивных сесий и булевой переменной
//				DWORD buffsize = 512; // размер максимально принятих данніх
//				DWORD buffersizereaden = 0; // принятых
//				fSuccess = ReadFile(
//					hPipe,        // handle to pipe 
//					bufferRequest,    // buffer to receive data 
//					buffsize, // size of buffer 
//					&buffersizereaden, // number of bytes read 
//					NULL);        // not overlapped I/O 
//
//				if (!fSuccess || buffersizereaden == 0)
//				{
//					if (GetLastError() == ERROR_BROKEN_PIPE)
//					{
//						addLogMessage("SocketWorkerThread: CredentialProvider disconnected.");
//						CloseHandle(hPipe);
//						ExitThread(-1);
//					}
//					else
//					{
//						addLogMessage("SocketWorkerThread ReadFile failed.");
//						CloseHandle(hPipe);
//						return  ERROR_INVALID_HANDLE;
//						//ExitThread(-1);
//					}
//				}
//				DisconnectNamedPipe(hPipe);
//			}
//
//			//в Провайдере при создании дискриптора КреатеФайл будет создаваться синхроная передача данных 
//			//	можно не волноваться за асинхроные посылки в канал !
//			// очистка канала и удаление его и закрытие
//			FlushFileBuffers(hPipe);
//			CloseHandle(hPipe);
//		}
//	// дальше посыл нашему сокету ответ исходя из наших данных
//	shutdown(*socket_client, 2); //гасим сокет который мы открыли
//	bytes_read = 0;    // очищаем буфер
//	if(socket_client)
//		delete socket_client;	// удаляем глобально выделенный в куче сокет - очищаем память (сылку на который мы получили в функции)
//	socket_client = NULL;	// на всякий случий присваеваем адресу нолевой указатель
//	addLogMessage("END ServerWorkerClient func");
//	//ExitThread(0);
//	return SEC_E_OK;
//}
DWORD WINAPI ServerWorkerClient(LPVOID lpParam)
{
	addLogMessage("Start ServerWorkerClient funcn");
	unsigned __int64 * socket_client = (unsigned __int64*)lpParam;
	//for debug client socket
	char socketstr[8];
	itoa(*socket_client, socketstr, 10);
	addLogMessage("socket  client in ServerWorkerClient = "); addLogMessage(socketstr);
	char message[255] = ""; // можно денамически выделять память исходя из размера байтРидера
	int bytes_read = 0;
	//bool b = TRUE; // для цикла но мы ожидаем конец события
	//while (WaitForSingleObject(g_ServiceStopEvent, 0) != WAIT_OBJECT_0) //b 
	//{
	bytes_read = recv(*socket_client, message, sizeof(message), 0); // message будет json
	if (bytes_read == 0) // потом 1 пакет будет идти как сообщение для размера передаваемых данных  
	{
		addLogMessage("ERROR. Bytes reader is field or error of network");
		//ExitThread(-1);
	}
	addLogMessage("Massage from client is accepted:");
	//debag mod (not read)
	int lensms = strlen(message);
	char * sms = new char[lensms];

	itoa(bytes_read, socketstr, 10);
	sms = message;
	addLogMessage(message); // для дебага!
	addLogMessage("Long message = "); addLogMessage(socketstr);
	bytes_read = 0;

	char loginbuf[255]= "crazyboy109@rambler.ru"; // login 
	
	char password[255] = "bratiya12";//pass
	
	LPTSTR userDomenName;
	DWORD sessionid;
	if (GetCurrentUser(userDomenName, sessionid))
	{
		addLogMessageW(userDomenName);
		addLogMessage("function GetWhoSessionIs return true!");
		return -1;
	}
	if (!SpeakWithPipe(loginbuf, password))
	{
		addLogMessage("Speak with Pipe return error");
	}
		// общение с провайдером и принятие у него решение !

		//GetCurrentUser();
		//передача по каналу сообщения провайдеру!
	
	// дальше посыл нашему сокету ответ исходя из наших данных
	shutdown(*socket_client, 2); //гасим сокет который мы открыли
	bytes_read = 0;    // очищаем буфер
	if (socket_client)
		delete socket_client;	// удаляем глобально выделенный в куче сокет - очищаем память (сылку на который мы получили в функции)
	socket_client = NULL;	// на всякий случий присваеваем адресу нолевой указатель
	addLogMessage("END ServerWorkerClient func");
	return SEC_E_OK;
}
// funct SESSION ID

BOOL SpeakWithPipe(char * loginbuf,char* password)
{
	LPTSTR lpszPipename = TEXT("\\\\.\\pipe\\pipeforcp");
	char bufferRequest[255] = ""; // буфер для принятия ответа
	HANDLE hPipe = INVALID_HANDLE_VALUE;
	int BUFSIZE = 255; //размер буфера
	bool fConnected = FALSE; // соеенение с каналом 
	BOOL fSuccess = FALSE; // логическая переменная для writepipe
	hPipe = CreateNamedPipe(
		lpszPipename,             // pipe name 
		PIPE_ACCESS_DUPLEX,       // read/write access 
		PIPE_TYPE_MESSAGE |       // message type pipe 
		PIPE_READMODE_MESSAGE |   // message-read mode 
		PIPE_WAIT,                // blocking mode 
		10,							// max. instances  == listen_socket
		BUFSIZE,                  // output buffer size 
		BUFSIZE,                  // input buffer size 
		0,                        // client time-out 
		NULL);                    // default security attribute 

	if (hPipe == INVALID_HANDLE_VALUE)
	{
		addLogMessage("CreateNamedPipe failed in ClientTherd");
		CloseHandle(hPipe); // rewrite
		return FALSE;
	}
	addLogMessage("Wait pipe connection");
	fConnected = ConnectNamedPipe(hPipe, NULL) ?
		TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);
	// CP заупситлся и принял нас - тоесть подключился к нам
	if (fConnected)
	{
		DWORD  writtenBytes = 0; // записаных в канал байтов
		DWORD bufferSizeWrite = 255; //-нужно динамически редактировать эту переменную
		addLogMessage("CP was connected, start to write in pipe..."); // debugMessage
																	  // Loop until done reading

		if (!WriteFile(
			hPipe,        // handle to pipe 
			loginbuf,     // buffer to write from 
			bufferSizeWrite, // number of bytes to write  - нужно динамически редактировать эту переменную 
			&writtenBytes,   // number of bytes written 
			NULL))      // not overlapped I/O  )
		{
			addLogMessage("SocketWorkerThread WriteFile failed.");
			CloseHandle(hPipe);
			return FALSE;
		}
		if (!WriteFile(
			hPipe,        // handle to pipe 
			password,     // buffer to write from 
			bufferSizeWrite, // number of bytes to write  - нужно динамически редактировать эту переменную 
			&writtenBytes,   // number of bytes written 
			NULL))      // not overlapped I/O  )
		{
			addLogMessage("SocketWorkerThread WriteFile failed.");
			CloseHandle(hPipe);
			return FALSE;
		}
		// чтение ответа с Провайдера (ответ будет 1) да + список активных сессий 2) нет 
		//вобщем не больше структуры запросаАктивных сесий и булевой переменной
	}
	else
	{
		addLogMessage("Connected to pipe is lose");
		return FALSE;
	}

	//в Провайдере при создании дискриптора КреатеФайл будет создаваться синхроная передача данных 
	//	можно не волноваться за асинхроные посылки в канал !
	// очистка канала и удаление его и закрытие
	FlushFileBuffers(hPipe);
	DisconnectNamedPipe(hPipe);
	CloseHandle(hPipe);
	return TRUE;
}
BOOL GetCurrentUser(LPTSTR &szUpn, DWORD & pSessionId)
{
	LPTSTR szUserName;
	LPTSTR szDomainName;
	char tmpbuf[8] = "";
	int dwSessionId = 0;
	PHANDLE hUserToken = 0;
	PHANDLE hTokenDup = 0;

	PWTS_SESSION_INFO pSessionInfo = 0;
	DWORD dwCount = 0;

	// Get the list of all terminal sessions 
	WTSEnumerateSessions(WTS_CURRENT_SERVER_HANDLE, 0, 1,
		&pSessionInfo, &dwCount);

	int dataSize = sizeof(WTS_SESSION_INFO);

	// look over obtained list in search of the active session
	for (DWORD i = 0; i < dwCount; ++i)
	{
		WTS_SESSION_INFO si = pSessionInfo[i];
		if (WTSActive == si.State)
		{
			// If the current session is active – store its ID
			dwSessionId = si.SessionId;
			pSessionId = dwSessionId; // in global programm
			itoa(dwSessionId, tmpbuf, 10);
			addLogMessage("Active session is ");
			addLogMessage(tmpbuf);
			break;
		}
	}
	WTSFreeMemory(pSessionInfo);

	DWORD dwLen = 0;
	BOOL bStatus = WTSQuerySessionInformation(WTS_CURRENT_SERVER_HANDLE,
		dwSessionId,
		WTSDomainName,
		&szDomainName,
		&dwLen);
	if (bStatus)
	{
		bStatus = WTSQuerySessionInformation(WTS_CURRENT_SERVER_HANDLE,
			dwSessionId,
			WTSUserName,
			&szUserName,
			&dwLen);
		if (bStatus)
		{
			DWORD cbUpn = _tcslen(szUserName) + 1 + _tcslen(szDomainName);
			 szUpn = (LPTSTR)LocalAlloc(0, (cbUpn + 1) * sizeof(TCHAR));

			_tcscpy(szUpn, szUserName);
			_tcscat(szUpn, _T("@"));
			_tcscat(szUpn, szDomainName);

			addLogMessage("UPN = "); addLogMessageW(szUpn);
			//LocalFree(szUpn); // in global system
			WTSFreeMemory(szUserName);
			WTSFreeMemory(szDomainName);
		}
		else
		{
			addLogMessage("WTSQuerySessionInformation on WTSUserName failed with error");
			return FALSE;
		}
	}
	else
	{
		addLogMessage("WTSQuerySessionInformation on WTSDomainName failed with erro");
		return FALSE;
	}
	return TRUE;
}