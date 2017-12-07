
#pragma once
#include "json.h"


// request
int fromJSON(const rapidjson::Value& doc,  char *&szLogin,  char *&szPass) {
	WriteTime();

	int command;
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
		addLogMessage("Active session list mode: JSON querty.");
		return 1;
	}
	else if (command == 0)
	{
		addLogMessage("Autorization mode: JSON querty:");
		addLogMessage((char*)(doc["login"].GetString()));
		addLogMessage((char*)(doc["password"].GetString()));
		static const char* members[] = { "login", "password" };

		for (size_t i = 0; i < sizeof(members) / sizeof(members[0]); i++)
			if (!doc.HasMember(members[i]))
			{
				addLogMessage("JSON doc hasn't member");
				return -1;
			}
		int szloginlength = doc["login"].GetStringLength();
		int szpasslength = doc["password"].GetStringLength();
		szLogin = new char[szloginlength];
		szPass = new char[szpasslength];
		szLogin = (char*)doc["login"].GetString();
		szPass = (char*)doc["password"].GetString();
		return 0;
	}
	return -1;
}

rapidjson::Document toJSONActiveSessionRespon(map<int, string>& session)  //respone to ActiveMode
{
	WriteTime();

	rapidjson::Value json_val;
	rapidjson::Document doc;
	auto& allocator = doc.GetAllocator();

	doc.SetObject();
	addLogMessage("In TOJSON func");
	for (auto it =session.begin(); it != session.end(); ++it)
	{
		addLogMessage(it->first); addLogMessage((char*)it->second.c_str());
		json_val.SetInt(it->first);
		doc.AddMember("session", json_val, allocator);
		json_val.SetString(it->second.c_str(), allocator);
		doc.AddMember("domenname", json_val, allocator);
	}
	addLogMessage("toJSONActiveSessionRespone:: String to respone client:"); 
	return doc;
}