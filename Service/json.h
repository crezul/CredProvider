

#pragma once 
#include "inclfiles.h"

int fromJSON(const rapidjson::Value& ,  char *&,  char *&);
rapidjson::Document toJSONActiveSessionRespon(map<int, string>&);//respone to ActiveMode


