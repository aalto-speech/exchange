#ifndef SIMPLE_IO
#define SIMPLE_IO

#include <string>
#include <iostream>
#include <fstream>
#include <memory>

#ifndef NO_ZLIB
#include "zlib.h"
#endif


class FileInputType
{
public:
    virtual bool getline(std::string &line) = 0;
    virtual ~FileInputType() { };
};

class IFStreamInput : public FileInputType
{
public:
    IFStreamInput(std::string filename) { ifstr.open(filename, std::ios_base::in); };
    ~IFStreamInput() { ifstr.close(); }
    bool getline(std::string &line) { return (bool)std::getline(ifstr, 
line); }
private:
    std::ifstream ifstr;
};

#ifndef NO_ZLIB
class GZipFileInput : public FileInputType
{
public:
    GZipFileInput(std::string filename);
    ~GZipFileInput();
    bool getline(std::string &line);
private:
    gzFile gzf;
};
#endif

class SimpleFileInput
{
public:
    SimpleFileInput(std::string filename);
    ~SimpleFileInput() { };
    bool getline(std::string &line) { return infs->getline(line); }
private:
    bool ends_with(std::string const &filename,
                   std::string const &suffix)
    {
        if (filename.length() < suffix.length()) return false;
        return (0 == filename.compare(filename.length()-suffix.length(), suffix.length(), suffix));
    }
    std::unique_ptr<FileInputType> infs;
};


class FileOutputType
{
public:
    virtual void close() = 0;
    virtual FileOutputType& operator<<(const std::string &str) = 0;
    virtual FileOutputType& operator<<(int) = 0;
    virtual FileOutputType& operator<<(long int) = 0;
    virtual FileOutputType& operator<<(unsigned int) = 0;
    virtual FileOutputType& operator<<(long unsigned int) = 0;
    virtual FileOutputType& operator<<(float) = 0;
    virtual FileOutputType& operator<<(double) = 0;
    virtual ~FileOutputType() { };
};


class OFStream: public FileOutputType
{
public:
    OFStream(std::string filename);
    ~OFStream();
    void close();
    OFStream& operator<<(const std::string &str);
    OFStream& operator<<(int);
    OFStream& operator<<(long int);
    OFStream& operator<<(unsigned int);
    OFStream& operator<<(long unsigned int);
    OFStream& operator<<(float);
    OFStream& operator<<(double);
private:
    std::ofstream ofstr;
};


#ifndef NO_ZLIB
class GZipFileOutput: public FileOutputType
{
public:
    GZipFileOutput(std::string filename);
    ~GZipFileOutput();
    void close();
    GZipFileOutput& operator<<(const std::string &str);
    GZipFileOutput& operator<<(int);
    GZipFileOutput& operator<<(long int);
    GZipFileOutput& operator<<(unsigned int);
    GZipFileOutput& operator<<(long unsigned int);
    GZipFileOutput& operator<<(float);
    GZipFileOutput& operator<<(double);
private:
    gzFile gzf;
    bool open;
};
#endif


class SimpleFileOutput
{
public:
    SimpleFileOutput(std::string filename);
    ~SimpleFileOutput();
    void close();
    SimpleFileOutput& operator<<(const std::string &str);
    SimpleFileOutput& operator<<(int);
    SimpleFileOutput& operator<<(long int);
    SimpleFileOutput& operator<<(unsigned int);
    SimpleFileOutput& operator<<(long unsigned int);
    SimpleFileOutput& operator<<(float);
    SimpleFileOutput& operator<<(double);
private:
    bool ends_with(std::string const &filename,
                   std::string const &suffix)
    {
        if (filename.length() < suffix.length()) return false;
        return (0 == filename.compare(filename.length()-suffix.length(), suffix.length(), suffix));
    }
    std::unique_ptr<FileOutputType> outfs;
};


#endif
