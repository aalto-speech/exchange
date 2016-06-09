#include "io.hh"

#include <cstring>

#define GZIP_BUFFER_SIZE 1048576

using namespace std;


SimpleFileInput::SimpleFileInput(string filename)
{
    if (ends_with(filename, ".gz"))
    {
#ifndef NO_ZLIB
        infs = new GZipFileInput(filename);
#else
        cerr << "No ZLIB support" << endl;
        exit(1);
#endif
    }
    else
        infs = new IFStreamInput(filename);
}

SimpleFileInput::~SimpleFileInput()
{
    if (infs) delete infs;
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
    char buffer[GZIP_BUFFER_SIZE];
    char *res = gzgets(gzf, buffer, GZIP_BUFFER_SIZE-1);
    if (res != NULL) {
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
        outfs = new GZipFileOutput(filename);
#else
        cerr << "No ZLIB support" << endl;
        exit(1);
#endif
    }
    else
        outfs = new OFStream(filename);
}

SimpleFileOutput::~SimpleFileOutput()
{
    close();
}

void
SimpleFileOutput::close()
{
    if (outfs) {
        outfs->close();
        delete outfs;
        outfs = NULL;
    }
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
    ofstr.open(filename.c_str(), ios_base::out);
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
    file_open = true;
}

GZipFileOutput::~GZipFileOutput()
{
    close();
}

void
GZipFileOutput::close()
{
    if (file_open) {
         gzclose(gzf);
         gzf = NULL;
    }
    file_open = false;
}

GZipFileOutput&
GZipFileOutput::operator<<(const std::string &str)
{
    gzprintf(gzf, "%s", str.c_str());
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

