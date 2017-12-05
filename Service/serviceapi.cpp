
#pragma once
#include "serviceapi.h"

DWORD WINAPI ServerWorkerClient(LPVOID lpParam)
{
	WriteTime();

	addLogMessage("Start ServerWorkerClient func");
	unsigned __int64 * socket_client = (unsigned __int64*)lpParam;

	SocketGuard socet_clientGuard(socket_client); // guard to socket
									// hen you exit from the scope, it will break the connection
									//to the socket and delete it

	//for debug client socket

	//
	char message[255] = "";
	int bytes_read = 0;

	bytes_read = recv(*socket_client, message, sizeof(message), 0); // message json

	if (bytes_read == 0) 
	{
		addLogMessage("ERROR. Bytes reader is field or error of network");
		return -1;
	}
	addLogMessage("Massage from client is accepted:");

	addLogMessage(message); 	addLogMessage("Long message = "); addLogMessage(bytes_read);

	bytes_read = 0;
	// With JSON
	
	rapidjson::Document doc;
	doc.Parse(message);
	char *login;
	char *pass;

	switch (fromJSON(doc, login, pass))
	{
	case 1:
	{
		LPTSTR userDomenName;
		DWORD sessionid;
		if (!GetCurrentUser(userDomenName, sessionid))
		{
			addLogMessage("ERROR:GetCurrentUser return field");
			return -1;
		}
		else
		{
			addLogMessage("GetCurrentUser function return true");
		}
		//
		char domenname[255] = "";
		WideCharToMultiByte(CP_ACP, 0, userDomenName, -1, domenname, 255, 0, 0);
		//respone to server

		//to respone JSON local varieble
		rapidjson::Document docrespon;
		rapidjson::StringBuffer buffer;
		rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

		docrespon = toJSONActiveSessionRespon(sessionid, domenname);

		docrespon.Accept(writer);
		//
		int sendbytes = 0;
		sendbytes = send(*socket_client, buffer.GetString(), (int)buffer.GetLength(), 0);

		if (sendbytes <= 0)
		{
			addLogMessage("Send respone to client is field. Networking or endpoint exception");
			return -1;
		}
		addLogMessage((char*)buffer.GetString());
	
		addLogMessage("Long message to respone = "); addLogMessage(sendbytes); //for debug

		break;
	}
	case 0:

		if (!SpeakWithPipe(login, pass))
		{
			addLogMessage("Speak with Pipe return error");
			return -1;
		}
		// respone to client of result autorization

		break;
	case -1:
		addLogMessage("fronJSON-func return error.");
		return -1;
		break;
	default:
		addLogMessage("JSON-func return false valuå ");
	}
	
	addLogMessage("END ServerWorkerClient func");
	return 0;
}
