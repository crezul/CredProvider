
#pragma once
#include "json.h"


// запрос от клиента 
bool fromJSON(const rapidjson::Value& doc,int &command,  char *szLogin,  char *szPass) {
	if (!doc.IsObject())
	{
		addLogMessage("JSON doc is not an object");
		return 0;
	}
		//throw std::runtime_error("document should be an object");
	addLogMessage((char*)(doc["login"].GetString()));
	addLogMessage((char*)(doc["password"].GetString()));
	static const char* members[] = { "command","login", "password" };
	for (size_t i = 0; i < sizeof(members) / sizeof(members[0]); i++)
		if (!doc.HasMember(members[i]))
		{
			addLogMessage("JSON doc hasn't member");
			return 0;
		}
			//throw std::runtime_error("missing fields");

	int szloginlength = doc["login"].GetStringLength();
	int szpasslength = doc["password"].GetStringLength();
	command = doc["command"].GetInt();
	szLogin = new char[szloginlength];
	szPass = new char[szpasslength];
	szLogin =(char*)doc["login"].GetString();
	szPass = (char*)doc["password"].GetString();
	return true;
}