
#pragma once 
#include "inclfiles.h"

BOOL GetCurrentUser(LPTSTR &, DWORD & );
BOOL SpeakWithPipe(char * , char*  );
BOOL GetUserdomenName(DWORD & dwSessionId, LPTSTR &szUpn);