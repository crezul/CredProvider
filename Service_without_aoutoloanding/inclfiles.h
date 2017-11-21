

#pragma once
//default
#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
//for log
#include <ctime>
#include <fstream>
//for service
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib,"wsock32.lib")
//for filestream
using namespace std;
//for json
//#include <iostream>
//#include <iomanip> // for std::setw
//#include "json.hpp"
//for Active Session
#include <WtsApi32.h>
#include <tchar.h>
#pragma comment(lib, "WtsApi32.lib")