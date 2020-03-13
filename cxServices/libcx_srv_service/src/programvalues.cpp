#include "programvalues.h"
#include <vector>
#include <sstream>
#include <iostream>
ProgramValues::ProgramValues()
{

}

void ProgramValues::initProgramName(const std::string &value)
{
    std::vector<std::string> paths;

    // Split into /.
    std::istringstream f(value);
    std::string s;
    while (getline(f, s, '/')) paths.push_back(s);

    programName = !paths.size()? "unknownprogram" : paths.at(paths.size()-1);
    description = programName;
    daemonName = programName;
}

std::string ProgramValues::getProgramName() const
{
    return programName;
}

void ProgramValues::setProgramName(const std::string &value)
{
    programName = value;
}

std::string ProgramValues::getAuthor() const
{
    return author;
}

void ProgramValues::setAuthor(const std::string &value)
{
    author = value;
}

std::string ProgramValues::getEmail() const
{
    return email;
}

void ProgramValues::setEmail(const std::string &value)
{
    email = value;
}

std::string ProgramValues::getVersion() const
{
    return version;
}

void ProgramValues::setVersion(const std::string &value)
{
    version = value;
}

std::string ProgramValues::getDescription() const
{
    return description;
}

void ProgramValues::setDescription(const std::string &value)
{
    description = value;
}

std::string ProgramValues::getDaemonName() const
{
    return daemonName;
}

void ProgramValues::setDaemonName(const std::string &value)
{
    daemonName = value;
}

