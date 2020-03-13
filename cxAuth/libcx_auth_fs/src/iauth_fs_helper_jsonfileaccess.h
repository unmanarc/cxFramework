#ifndef IAUTH_HELPER_JSONFILEACCESS_H
#define IAUTH_HELPER_JSONFILEACCESS_H

#include <json/json.h>

Json::Value loadJSONFile(const std::string & fileName);
bool saveJSONFile(const std::string & fileName, const Json::Value & v);


#endif // IAUTH_HELPER_JSONFILEACCESS_H
