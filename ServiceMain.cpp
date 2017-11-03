
#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
#include <ctime>
#include <fstream>
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib,"wsock32.lib")
using namespace std;



SERVICE_STATUS        g_ServiceStatus = { 0 };
SERVICE_STATUS_HANDLE g_StatusHandle = NULL;
HANDLE                g_ServiceStopEvent = INVALID_HANDLE_VALUE;

VOID WINAPI ServiceMain(DWORD argc, LPTSTR *argv);
VOID WINAPI ServiceCtrlHandler(DWORD);
DWORD WINAPI ServiceWorkerThread(LPVOID lpParam);
DWORD WINAPI ServerWorkerClient(LPVOID lpParam);
void addLogMessage(char *ch);

#define SERVICE_NAME  _T("My Sample Service")
#define ERROR_CLIENT_THEARD NULL

void addLogMessage(char *ch)
{
	fstream  fout("CredentialServiceProvider.logfile", ios::out | ios::app);
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
		return GetLastError();
	}

	return 0;
}
VOID WINAPI ServiceMain(DWORD argc, LPTSTR *argv)
{
	DWORD Status = E_FAIL;

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
				"My Sample Service: ServiceMain: SetServiceStatus returned error");
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
	WaitForSingleObject(hThread, INFINITE);


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
	return;
}

VOID WINAPI ServiceCtrlHandler(DWORD CtrlCode)
{
	switch (CtrlCode)
	{
	case SERVICE_CONTROL_STOP:

		if (g_ServiceStatus.dwCurrentState != SERVICE_RUNNING)
			break;

		/*
		* Perform tasks necessary to stop the service here
		*/

		g_ServiceStatus.dwControlsAccepted = 0;
		g_ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
		g_ServiceStatus.dwWin32ExitCode = 0;
		g_ServiceStatus.dwCheckPoint = 4;

		if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
		{
			addLogMessage(
				"My Sample Service: ServiceCtrlHandler: SetServiceStatus returned error");
		}

		// This will signal the worker thread to start shutting down
		SetEvent(g_ServiceStopEvent);

		break;

	default:
		break;
	}
}

DWORD WINAPI ServiceWorkerThread(LPVOID lpParam)
{
	//инициализация обьекта для работы с сокетом 
		WSADATA Data;
		int status = WSAStartup(MAKEWORD(1, 1), &Data);
		if (status != 0)
		{
			addLogMessage("ERROR: WSAStartup unsuccessful");
			return  ERROR_SUCCESS;;
		}
	//настройки сервера сокета 
	int s;
	SOCKADDR_IN servAdr;
	memset(&servAdr, 0, sizeof(servAdr));
	servAdr.sin_port = htons(4000);
	servAdr.sin_family = AF_INET;
	servAdr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
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
	//  Periodically check if the service has been requested to stop
	while (WaitForSingleObject(g_ServiceStopEvent, 0) != WAIT_OBJECT_0)
	{
		//Sleep(9000);
		//SetEvent(g_ServiceStopEvent); // событие можно устанавливать когда у нас глобальная ошибка сервиса (не биндится соккет или похожая проблема)
		/*
		* Perform main service function here
		*/
		SOCKADDR_IN from_sin;
		int from_len;
			while (1)
			{
				addLogMessage("Waiting for a connection..."); // для дебага
				from_len = sizeof(from_sin);
				int *s_new = new int (accept(s, (SOCKADDR*)&from_sin, &from_len)); // unsigned __int64 вызываемый сокет - можно переделать но целеобразно ли это?
				// после получения дискриптора клиента можно создать отдетьный поток 
				//для паралельной работы сервера и обработки многих запросов
				if (*s_new < 0) //не глобальная ошибка
				{
					addLogMessage("ERROR. Acept with client field");
				}
				else
				{
					addLogMessage("Client connect");
					// функция обработки сервера
					hThreadForClient = CreateThread(NULL, 0,ServerWorkerClient,(LPVOID) &s_new,	0, NULL);
					
					if (hThreadForClient == ERROR_CLIENT_THEARD)
					{
						addLogMessage("Client theard didnt created"); // не глобальная ошибка
					}
				}
				//очистка буферов и сокета принятого клиента 
				from_len = 0;
				memset(&from_sin, 0, sizeof(from_sin));
			}	
	}
	return ERROR_SUCCESS;
}
DWORD WINAPI ServerWorkerClient(LPVOID lpParam)
{
	int * socket_client = (int*)&lpParam;
	LPTSTR lpszPipename = TEXT("\\\\.\\pipe\\pipeforcp");
	char message[2048] = ""; // можно денамически выделять память исходя из размера байтРидера
	char bufferRequest[255] = ""; // буфер для принятия ответа
	int bytes_read;
	bool b = TRUE;
	while (b)
	{
		bytes_read = recv(*socket_client, message, sizeof(message), 0); // message будет json
		if (bytes_read <= 0)
		{
			addLogMessage("ERROR. Bytes reader is field or error of network");
			ExitThread(-1);
		}
		addLogMessage("Massage from client is accepted:"); addLogMessage(message); // для дебага!
		b = FALSE; //выход с цикла (он нужен весь прием должен быть в цикле хотя ресв принимает все в своем цикле )																	   //посылаем результат смс провайдеру!
	}
	//передача по каналу сообщения провайдеру!
	HANDLE hPipe = INVALID_HANDLE_VALUE;
	int BUFSIZE = 2048; //размер буфера
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
		ExitThread(-1);
	}
	// writefile! - сделать так чтоб дискриптор доступа ждал ответа от клиента! - супер важно!
	fConnected = ConnectNamedPipe(hPipe, NULL) ?
		TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);
	// CP заупситлся и принял нас - тоесть подключился к нам
	if (fConnected)
	{
		DWORD  writtenBytes = 0; // записаных в канал байтов
		DWORD bufferSizeWrite = 2048; //-нужно динамически редактировать эту переменную
		addLogMessage("CP was connected, start to write in pipe..."); // debugMessage
																	  // Loop until done reading
			fSuccess = WriteFile(
				hPipe,        // handle to pipe 
				message,     // buffer to write from 
				bufferSizeWrite, // number of bytes to write  - нужно динамически редактировать эту переменную 
				&writtenBytes,   // number of bytes written 
				NULL);        // not overlapped I/O 

			if (!fSuccess || bufferSizeWrite != writtenBytes)
			{
				addLogMessage("SocketWorkerThread WriteFile failed.");
				CloseHandle(hPipe);
				ExitThread(-1);
			}
			// чтение ответа с Провайдера (ответ будет 1) да + список активных сессий 2) нет 
			//вобщем не больше структуры запросаАктивных сесий и булевой переменной
			DWORD buffsize = 512; // размер максимально принятих данніх
			DWORD buffersizereaden = 0; // принятых
			fSuccess = ReadFile(
				hPipe,        // handle to pipe 
				bufferRequest,    // buffer to receive data 
				buffsize, // size of buffer 
				&buffersizereaden, // number of bytes read 
				NULL);        // not overlapped I/O 

			if (!fSuccess || buffersizereaden == 0)
			{
				if (GetLastError() == ERROR_BROKEN_PIPE)
				{
					addLogMessage("SocketWorkerThread: CredentialProvider disconnected.");
					CloseHandle(hPipe);
					ExitThread(-1);
				}
				else
				{
					addLogMessage("SocketWorkerThread ReadFile failed.");
					CloseHandle(hPipe);
					ExitThread(-1);
				}
			}
			DisconnectNamedPipe(hPipe);
	}
	//в Провайдере при создании дискриптора КреатеФайл будет создаваться синхроная передача данных 
	//	можно не волноваться за асинхроные посылки в канал !
	// очистка канала и удаление его и закрытие
	FlushFileBuffers(hPipe);
	CloseHandle(hPipe);
	// дальше посыл нашему сокету ответ исходя из наших данных
	shutdown(*socket_client, 2); //гасим сокет который мы открыли
	bytes_read = 0;    // очищаем буфер
	if(socket_client)
		delete socket_client;	// удаляем глобально выделенный в куче сокет - очищаем память (сылку на который мы получили в функции)
	socket_client = NULL;	// на всякий случий присваеваем адресу нолевой указатель
	ExitThread(0);
}
//int addLogMessage(char *ch)
//{
//	return  printf("\t %s \n", ch);
//}
