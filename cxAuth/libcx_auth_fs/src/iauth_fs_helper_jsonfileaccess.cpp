#include "iauth_fs_helper_jsonfileaccess.h"

#include <iostream>
#include <fstream>

Json::Value loadJSONFile(const std::string &fileName)
{
    Json::Value root;
    std::ifstream file(fileName);
    file >> root;
    return root;
}

bool saveJSONFile(const std::string &fileName, const Json::Value &v)
{
    Json::StyledWriter styledWriter;
    std::ofstream ofstr;
    ofstr.open(fileName);
    ofstr << styledWriter.write(v);
    ofstr.close();
    return true;
}
