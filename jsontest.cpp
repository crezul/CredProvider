// jsontest.cpp: определяет точку входа для консольного приложения.
//
// 
// benchmarks_simple.cpp -- a less complex version of benchmarks.cpp, that better reflects actual performance
// 
//     For some reason, the complexity of benchmarks.cpp doesn't allow
// the compiler to optimize code using json.hpp effectively.  The
// exact same tests, with the use of benchpress and cxxopts produces
// much faster code, at least under g++.
// 
#include "stdafx.h"

//#include "stdafx.h"
//
//

#include <iostream>
#include <iomanip> // for std::setw
#include "json.hpp"

using namespace std;
using json = nlohmann::json;

// json для принятиях
bool fromJsonUserPacket(json j, string  & log_str, string & pass_str, int &size) //char& log,char &pass) // structura
{
	/*if (j.is_null)
		return 0;*/
	bool whatQueryClientSend = false; // false - yhis query for list active users
	// true - query for Cred_provider
	if (j.size() != 4)
	{
		cout << "ERROR: json response has't got 4 arguments\n";
		return 0;
	}

	log_str = j["log"].get < std::string >();
	pass_str = j["pass"].get < std::string >();
	bool b= j["query"].get < bool >();
	size = j["size"].get<int>();
	// log_str = j ["log"];
	// pass_str = j["pass"];
	cout << "in funct fromJsonUserPacket= " << log_str << "   " <<pass_str << "  "<<b<<" "<<size<<'\n';
	return 1;
}
bool toJsonTest(string log_str, string pas_str, json &j)
{
	int size = 1212;
	bool b = false;
	 j =
	{
		{ "log", log_str },
		{ "pass", pas_str },
		{"query", b},
		{"size", size},
	};
	 std::cout << std::setw(2) << j << '\n';
	 return 1;
}
int main()
{
	json k;
	string log, pas;
	int size = 0;
	string logreq = "login1";
	string pasreq = "pass1";
	bool b1= toJsonTest(logreq,pasreq,k);
	bool b2 = fromJsonUserPacket(k, log, pas,size);
	if (b1 &&b2) cout << '\n'<<"OTVET\n" << log<< '\n' << pas << '\n';
	/*
	char data[200] = "";
	for (int i = 0; i < 200; i++)
		data[i] = '-';
	
	cout << "STROKA - \n";
	cin.getline(data,sizeof(data));
	for (int i = 0; i < 200; i++)
		cout << data;*/
	// create a JSON object
	json j =
	{
		{ "pi", 3.141 },
		{ "happy", true },
		{ "name", "Niels" },
		{ "nothing", nullptr },
		{
			"answer",{
				{ "everything", 42 }
			}
		},
		{ "list",{ 1, 0, 2 } },
		{
			"object",{
				{ "currency", "USD" },
				{ "value", 42.99 }
			}
		}
	};

	// add new values
	j["new"]["key"]["value"] = { "another", "list" };

	// count elements
	auto s = j.size();
	j["size"] = s;

	// pretty print with indent of 4 spaces
	std::cout << std::setw(4) << j << '\n';
	
}
