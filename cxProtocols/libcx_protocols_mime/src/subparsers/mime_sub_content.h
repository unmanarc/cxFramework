#ifndef MIME_SUB_CONTENT_H
#define MIME_SUB_CONTENT_H

#include <cx_mem_streamparser/substreamparser.h>

class MIME_Sub_Content : public SubStreamParser
{
public:
    MIME_Sub_Content();
    ~MIME_Sub_Content();

    bool stream(WRStatus &wrStat) override;

    uint64_t getMaxContentSize() const;
    void setMaxContentSize(const uint64_t &value);

    std::string getFsTmpFolder() const;
    void setFsTmpFolder(const std::string &value);

    BinaryContainer_Base *getContentContainer() const;
    void replaceContentContainer(BinaryContainer_Base *value);

    std::string getBoundary() const;
    void setBoundary(const std::string &value);

    // TODO: implement using filesystem.
    uint64_t getMaxContentSizeUntilGoingToFS() const;
    void setMaxContentSizeUntilGoingToFS(const uint64_t &value);

protected:
    ParseStatus parse() override;
private:
    BinaryContainer_Base * contentContainer;

    std::string fsTmpFolder, boundary;
    uint64_t maxContentSize;
    uint64_t maxContentSizeUntilGoingToFS;
};

#endif // MIME_SUB_CONTENT_H
