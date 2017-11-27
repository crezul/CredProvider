
#pragma once
#include "serviceapi.h"

DWORD WINAPI ServerWorkerClient(LPVOID lpParam)
{
	addLogMessage("Start ServerWorkerClient funcn");
	unsigned __int64 * socket_client = (unsigned __int64*)lpParam;
	//for debug client socket
	char socketstr[8];
	itoa(*socket_client, socketstr, 10);
	addLogMessage("socket  client in ServerWorkerClient = "); addLogMessage(socketstr);
	//
	char message[255] = ""; // можно денамически выделять память исходя из размера байтРидера
	int bytes_read = 0;

	bytes_read = recv(*socket_client, message, sizeof(message), 0); // message будет json

	if (bytes_read == 0) // потом 1 пакет будет идти как сообщение для размера передаваемых данных  
	{
		addLogMessage("ERROR. Bytes reader is field or error of network");
		return -1;
	}
	addLogMessage("Massage from client is accepted:");

	itoa(bytes_read, socketstr, 10);
	
	addLogMessage(message); // для дебага!
	addLogMessage("Long message = "); addLogMessage(socketstr);
	bytes_read = 0;
	// With JSON
	
	//
	int command;

	rapidjson::Document doc;
	doc.Parse(message);
	if (!doc.IsObject())
	{
		addLogMessage("JSON doc is not an object");
		return -1;
	}
	if (!doc.HasMember("command"))
	{
		addLogMessage("JSON doc hasn't member command");
		return -1;
	}
	command = doc["command"].GetInt();
	
		if (command == 1)
		{
			addLogMessage("Active session list mode");
			LPTSTR userDomenName;
			DWORD sessionid;
			if (!GetCurrentUser(userDomenName, sessionid))
			{
				addLogMessage("ERROR:GetCurrentUser return field");
				return -1;
			}
			// ответ серверу с доменом и именем
		}
	else 
		if (command == 0)
		{
			addLogMessage("Autorization mode");
			static const char* members[] = {"login", "password" };
			for (size_t i = 0; i < sizeof(members) / sizeof(members[0]); i++)
				if (!doc.HasMember(members[i]))
				{
					addLogMessage("JSON doc hasn't member login or pass");
					return -1;
				}
			char *login = (char*)doc["login"].GetString();
			char *pass = (char*)doc["password"].GetString();

			addLogMessage("JSON packet is accept");
			addLogMessage(login); addLogMessage(pass);

				if (!SpeakWithPipe(login, pass))
				{
					addLogMessage("Speak with Pipe return error");
					return -1;
				}
			}
			else
			{
				addLogMessage("ERROR: code of command !=login or sessionlist");
				return -1;
			}
		
	shutdown(*socket_client, 2); //гасим сокет который мы открыли
	bytes_read = 0;    // очищаем буфер
	if (socket_client)
		delete socket_client;	// удаляем глобально выделенный в куче сокет - очищаем память (сылку на который мы получили в функции)
	socket_client = NULL;	// на всякий случий присваеваем адресу нолевой указатель
	addLogMessage("END ServerWorkerClient func");
	return SEC_E_OK;
}
