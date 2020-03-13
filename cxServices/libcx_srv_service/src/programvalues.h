#ifndef PROGRAMVALUES_H
#define PROGRAMVALUES_H

#include <string>

class ProgramValues
{
public:
    ProgramValues();

    void initProgramName(const std::string &value);

    std::string getProgramName() const;
    void setProgramName(const std::string &value);

    std::string getAuthor() const;
    void setAuthor(const std::string &value);

    std::string getEmail() const;
    void setEmail(const std::string &value);

    std::string getVersion() const;
    void setVersion(const std::string &value);

    std::string getDescription() const;
    void setDescription(const std::string &value);

    std::string getDaemonName() const;
    void setDaemonName(const std::string &value);

protected:
    std::string programName, author, email, version, description;
    std::string daemonName;

};

#endif // PROGRAMVALUES_H
