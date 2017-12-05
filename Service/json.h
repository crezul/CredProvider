

#pragma once 
#include "inclfiles.h"

int fromJSON(const rapidjson::Value& ,  char *&,  char *&);
rapidjson::Document toJSONActiveSessionRespon(int, char* );//respone to ActiveMode
