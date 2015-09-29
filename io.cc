#include "io.hh"

#include <cstring>

using namespace std;


SimpleFileInput::SimpleFileInput(string filename)
{
    if (ends_with(filename, ".gz"))
    {
#ifndef NO_ZLIB
        infs = unique_ptr<FileInputType>(new GZipFileInput(filename));
#else
        cerr << "No ZLIB support" << endl;
        exit(1);
#endif
    }
    else
        infs = unique_ptr<FileInputType>(new IFStreamInput(filename));
}


#ifndef NO_ZLIB
GZipFileInput::GZipFileInput(string filename)
{
    gzf = gzopen(filename.c_str(), "r");
}

GZipFileInput::~GZipFileInput()
{
    gzclose(gzf);
}

bool
GZipFileInput::getline(string &line)
{
    char buffer[8192];
    char *res = gzgets(gzf, buffer, 8192-1);
    if (res != nullptr) {
        strtok(buffer, "\r\n");
        line.assign(buffer);
        return true;
    }
    return false;
}
#endif


SimpleFileOutput::SimpleFileOutput(string filename)
{
    if (ends_with(filename, ".gz"))
    {
#ifndef NO_ZLIB
        outfs = unique_ptr<FileOutputType>(new GZipFileOutput(filename));
#else
        cerr << "No ZLIB support" << endl;
        exit(1);
#endif
    }
    else
        outfs = unique_ptr<FileOutputType>(new OFStream(filename));
}

SimpleFileOutput::~SimpleFileOutput()
{
    close();
}

void
SimpleFileOutput::close()
{
    outfs->close();
}


SimpleFileOutput&
SimpleFileOutput::operator<<(const std::string &str)
{
    *outfs << str;
    return *this;
}

SimpleFileOutput&
SimpleFileOutput::operator<<(int intr)
{
    *outfs << intr;
    return *this;
}

SimpleFileOutput&
SimpleFileOutput::operator<<(long int lintr)
{
    *outfs << lintr;
    return *this;
}


SimpleFileOutput&
SimpleFileOutput::operator<<(unsigned int uintr)
{
    *outfs << uintr;
    return *this;
}

SimpleFileOutput&
SimpleFileOutput::operator<<(long unsigned int luintr)
{
    *outfs << luintr;
    return *this;
}

SimpleFileOutput&
SimpleFileOutput::operator<<(float fltn)
{
    *outfs << fltn;
    return *this;
}

SimpleFileOutput&
SimpleFileOutput::operator<<(double dfltn)
{
    *outfs << dfltn;
    return *this;
}


OFStream::OFStream(string filename)
{
    ofstr.open(filename, ios_base::out);
}

OFStream::~OFStream()
{
    close();
}

void
OFStream::close()
{
    ofstr.close();
}

OFStream&
OFStream::operator<<(const std::string &str)
{
    ofstr << str;
    return *this;
}

OFStream&
OFStream::operator<<(int intr)
{
    ofstr << intr;
    return *this;
}

OFStream&
OFStream::operator<<(long int lintr)
{
    ofstr << lintr;
    return *this;
}


OFStream&
OFStream::operator<<(unsigned int uintr)
{
    ofstr << uintr;
    return *this;
}

OFStream&
OFStream::operator<<(long unsigned int luintr)
{
    ofstr << luintr;
    return *this;
}

OFStream&
OFStream::operator<<(float fltn)
{
    ofstr << fltn;
    return *this;
}

OFStream&
OFStream::operator<<(double dfltn)
{
    ofstr << dfltn;
    return *this;
}


#ifndef NO_ZLIB
GZipFileOutput::GZipFileOutput(string filename)
{
    gzf = gzopen(filename.c_str(), "w");
    open = true;
}

GZipFileOutput::~GZipFileOutput()
{
    close();
}

void
GZipFileOutput::close()
{
    if (open) gzclose(gzf);
    open = false;
}

GZipFileOutput&
GZipFileOutput::operator<<(const std::string &str)
{
    gzprintf(gzf, str.c_str());
    return *this;
}

GZipFileOutput&
GZipFileOutput::operator<<(int intr)
{
    gzprintf(gzf, "%d", intr);
    return *this;
}

GZipFileOutput&
GZipFileOutput::operator<<(long int lintr)
{
    gzprintf(gzf, "%ld", lintr);
    return *this;
}


GZipFileOutput&
GZipFileOutput::operator<<(unsigned int uintr)
{
    gzprintf(gzf, "%u", uintr);
    return *this;
}

GZipFileOutput&
GZipFileOutput::operator<<(long unsigned int luintr)
{
    gzprintf(gzf, "%lu", luintr);
    return *this;
}

GZipFileOutput&
GZipFileOutput::operator<<(float fltn)
{
    gzprintf(gzf, "%f", fltn);
    return *this;
}

GZipFileOutput&
GZipFileOutput::operator<<(double dfltn)
{
    gzprintf(gzf, "%f", dfltn);
    return *this;
}
#endif

