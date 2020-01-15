#include "ContentFileIO.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <stdexcept>
#include <sys/stat.h>
#include <unistd.h>

#include <libecap/common/errors.h>

#include "Config.hpp"
#include "Log.hpp"
#include "MemoryAreaDetails.hpp"

using namespace ExifAdapter;

//------------------------------------------------------------------------------
libecap::shared_ptr<ContentFileIO> ContentFileIO::FromTemporaryFile()
{
    Config* config = Config::GetConfig();
    std::string tmp_filename_format = config->GetTemporaryFilenameFormat();
    unsigned filename_length = tmp_filename_format.size();
    char* temporary_filename =
        static_cast<char*>(std::malloc(filename_length + 1));
    memset(temporary_filename, 0, filename_length + 1);
    strncpy(temporary_filename, tmp_filename_format.c_str(), filename_length);

    int result = mkstemp(temporary_filename);
    if (result == -1)
    {
        std::string msg =
            std::string("failed to create temporary file with format ") +
            temporary_filename + ": " + strerror(errno);
        std::free(temporary_filename);
        Log(libecap::flXaction | libecap::ilNormal) << msg;
        throw libecap::TextException(msg);
    }

    std::string filename = temporary_filename;
    std::free(temporary_filename);
    return libecap::shared_ptr<ContentFileIO>(
        new ContentFileIO(result, filename));
}

//------------------------------------------------------------------------------
ContentFileIO::ContentFileIO(
    int fd,
    const std::string& filename)
    : fd(fd)
    , filename(filename)
{
}

//------------------------------------------------------------------------------
ContentFileIO::~ContentFileIO()
{
    CloseFile();
    RemoveFile();
}

//------------------------------------------------------------------------------
size_t ContentFileIO::Write(const libecap::Area& data)
{
    if (!fd)
        OpenFile();

    const size_t written = write(fd, data.start, data.size);
    if (written != data.size)
    {
        std::string msg =
            std::string("Failed to write to file: ") + strerror(errno);
        Log(libecap::flXaction | libecap::ilNormal) << msg;
        throw libecap::TextException(msg);
    }

    return written;
}

//------------------------------------------------------------------------------
void ContentFileIO::Shift(libecap::size_type bytes)
{
    offset += bytes;
}

//------------------------------------------------------------------------------
void ContentFileIO::ResetOffset()
{
    offset = 0;
}

//------------------------------------------------------------------------------
libecap::Area ContentFileIO::Read(
    libecap::size_type offset,
    libecap::size_type size)
{
    if (size == 0)
        return libecap::Area();

    Must(fd);

    const libecap::size_type position = this->offset + offset;
    Must(lseek(fd, position, SEEK_SET) != -1);

    libecap::shared_ptr<MemoryAreaDetails> details(
        new MemoryAreaDetails(size));

    const ssize_t result = read(fd, details->GetAreaStart(), size);
    if (result != -1)
        return libecap::Area(details->GetAreaStart(), result, details);

    return libecap::Area();
}

//------------------------------------------------------------------------------
void ContentFileIO::ApplyFilter(libecap::shared_ptr<MetadataFilter> filter)
{
    if (!filter)
        return;

    CloseFile();

    try
    {
        filter->ProcessFile(filename);
    }
    catch (MetadataFilter::Exception& e)
    {
        OpenFile();
        throw e;
    }

    OpenFile();
}

//------------------------------------------------------------------------------
uint64_t ContentFileIO::GetLength() const
{
    struct stat statbuf;

    if (fstat(fd, &statbuf) == -1)
    {
        std::string msg = std::string("Failed to get file size of ") +
            filename + ": " + strerror(errno);
        Log(libecap::flXaction | libecap::ilNormal) << msg;
        throw libecap::TextException(msg);
    }

    return statbuf.st_size;
}

//------------------------------------------------------------------------------
void ContentFileIO::OpenFile()
{
    fd = open(filename.c_str(), O_RDONLY);
    if (fd == -1)
    {
        fd = 0;
        std::string msg = "Failed to open file " + filename +
            ": " + strerror(errno);
        Log(libecap::flXaction | libecap::ilNormal) << msg;
        throw libecap::TextException(msg);
    }
}

//------------------------------------------------------------------------------
void ContentFileIO::CloseFile()
{
    if (fd)
    {
        if (close(fd) != 0)
        {
            Log(libecap::flXaction | libecap::ilNormal)
                << "failed to close file "
                << filename << ": "
                << strerror(errno);
        }

        fd = 0;
    }

}

//------------------------------------------------------------------------------
void ContentFileIO::RemoveFile()
{
    if (!filename.empty())
    {
        if (remove(filename.c_str()) != 0)
        {
            Log(libecap::flXaction | libecap::ilDebug)
                << "failed to remove file "
                << filename << ": "
                << strerror(errno);
        }

        filename.clear();
    }

}
