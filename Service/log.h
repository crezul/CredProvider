
#pragma once 
#include "inclfiles.h"


static char * fstremname = "C:\\Users\\Danil\\Desktop\\Service_android_provider.txt"; //local directory to LocalUser
void addLogMessage(LPTSTR ch);
void addLogMessage(char *ch);
void addLogMessage(unsigned __int64 ch);
bool isfstream();
void WriteTime();

