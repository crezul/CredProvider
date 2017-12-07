

#pragma once
//default
#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
#include <iostream>

//for filestream
using namespace std;
//for log
#include <ctime>
#include <fstream>
//for service
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib,"wsock32.lib")

//for json
#include  "rapidjson\document.h " 
#include  "rapidjson\writer.h " 
#include  "rapidjson\stringbuffer.h " 
//for Active Session
#include <WtsApi32.h>
#include <tchar.h>
#include <string>
#pragma comment(lib, "WtsApi32.lib") 
#include <map>

//my
#include "sessionid.h"
#include "serviceapi.h"
#include "log.h"
#include "servicemain.h"
#include "crypto.h"
#include "json.h"
#include "wrappers.h"
//

