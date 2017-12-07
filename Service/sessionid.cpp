
#include "sessionid.h"

BOOL SpeakWithPipe(char * loginbuf, char* password)
{
	WriteTime();

	addLogMessage("Start SpeakWithPipe func");
	LPTSTR lpszPipename = TEXT("\\\\.\\pipe\\pipeforcp");

	char bufferRequest[255] = ""; // buffer for request in pipe
	HANDLE hPipe = INVALID_HANDLE_VALUE;
	int BUFSIZE = 255; //size buffer
	bool fConnected = FALSE; // link to the channel
	BOOL fSuccess = FALSE; // Boolean variable for writepipe

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
	HandleGuard  hFile2Guard(hPipe); // Guard for HNDLE class. Distructor delete this.
	if (hPipe == INVALID_HANDLE_VALUE)
	{
		addLogMessage("CreateNamedPipe failed in ClientTherd");
		return FALSE;
	}
	addLogMessage("Wait pipe connection");
	fConnected = ConnectNamedPipe(hPipe, NULL) ?
		TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);
	// CredProv start and took us - I got connected to us
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
			return FALSE;
		}
	}
	else
	{
		addLogMessage("Connected to pipe is lose");
		return FALSE;
	}

	// in the Provider when CreationFile is created, the synchronous data transfer will be created
	// You do not have to worry about sending asynchronous messages to the channel!
	// clear the channel and delete it and close it
	FlushFileBuffers(hPipe);
	DisconnectNamedPipe(hPipe);
	return TRUE;
}

BOOL GetCurrentUser( map<int,string>& allsession)
{
	WriteTime();
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
			addLogMessage("TEST to ACTIVE SESSION");
			// If the current session is active – store its ID
			addLogMessage("Active session is ");
			addLogMessage(si.SessionId);
			GetUserdomenName(si.SessionId, allsession);
		}
		if (WTSDisconnected == si.State)
		{
			addLogMessage("TEST to WTSDisconnected SESSION");
			addLogMessage("WTSDisconnected session is ");
			addLogMessage(si.SessionId);
			GetUserdomenName(si.SessionId, allsession);
		}
		
		if (WTSConnected == si.State)
		{
			addLogMessage("TEST to WTSConnected SESSION");
			addLogMessage("WTSConnected session is ");
			addLogMessage(si.SessionId);
			GetUserdomenName(si.SessionId, allsession);
		}
	}
	WTSFreeMemory(pSessionInfo);

	return TRUE;
}

BOOL GetUserdomenName(DWORD & dwSessionId,map<int, string>& allsession)
{
	LPTSTR szUpn;
	LPTSTR szUserName;
	LPTSTR szDomainName;
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
			size_t cbUpn = _tcslen(szUserName) + 1 + _tcslen(szDomainName);
			szUpn = (LPTSTR)LocalAlloc(0, (cbUpn + 1) * sizeof(TCHAR));

			_tcscpy(szUpn, szUserName);
			_tcscat(szUpn, _T("@"));
			_tcscat(szUpn, szDomainName);

			addLogMessage("UPN = "); addLogMessage(szUpn);
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
	char domenname[255] = "";
	WideCharToMultiByte(CP_ACP, 0, szUpn, -1, domenname, 255, 0, 0);
	addLogMessage("Insert in map");
	allsession.insert(pair<int,string>((int)dwSessionId, domenname));
	addLogMessage("map size in func = "); addLogMessage(allsession.size());
	//LocalFree(szUpn); // in global system
	return TRUE;
}