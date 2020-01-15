#include "Config.hpp"

#include <iostream> //FIXME
#include <sstream>
#include <vector>

#include <libecap/common/area.h>
#include <libecap/common/errors.h>
#include <libecap/common/name.h>

using namespace ExifAdapter;

static Config* config = NULL;

static const char* TMP_FILENAME_FORMAT = "/tmp/exif-ecap-XXXXXX";
static uint64_t MEMORY_FILESIZE_LIMIT = 512 * 1024;

//------------------------------------------------------------------------------
namespace
{
    std::vector<std::string> &split(
        const std::string &s,
        char delim,
        std::vector<std::string> &elems)
    {
        std::stringstream ss(s);
        std::string item;
        while(std::getline(ss, item, delim))
            elems.emplace_back(item);
        return elems;
    }
}

//------------------------------------------------------------------------------
Config* Config::GetConfig()
{
    if (!config)
        config = new Config();

    return config;
}

//------------------------------------------------------------------------------
Config::Config()
    : tmp_filename_format(TMP_FILENAME_FORMAT)
    , memory_filesize_limit(MEMORY_FILESIZE_LIMIT)
{
}

//------------------------------------------------------------------------------
void Config::visit(
    const libecap::Name& name,
    const libecap::Area& area_value)
{
    const std::string value = area_value.toString();

    if (name == "tmp_filename_format")
        SetTemporaryFilenameFormat(value);
    else if (name == "memory_filesize_limit")
        SetMemoryFilesizeLimit(value);
    else if (name == "exclude_types") {
        std::vector<std::string> types;
        excluded_types = split(value, ';', types);
    }
    else if (name.assignedHostId()) {}
    else
    {
        throw libecap::TextException(
            "EXIF-eCAP: unsupported adapter configuration parameter: " +
            name.image());
    }
}

//------------------------------------------------------------------------------
std::string Config::GetTemporaryFilenameFormat()
{
    return tmp_filename_format;
}

//------------------------------------------------------------------------------
uint64_t Config::GetMemoryFilesizeLimit()
{
    return memory_filesize_limit;
}

//------------------------------------------------------------------------------
const std::vector<std::string>& Config::GetExcludedTypes() const
{
    return excluded_types;
}

//------------------------------------------------------------------------------
void Config::SetTemporaryFilenameFormat(const std::string& value)
{
    if (value.empty() || value == "default")
        tmp_filename_format = TMP_FILENAME_FORMAT;
    else if (value.rfind("XXXXXX") != value.size() - 6)
        tmp_filename_format = value + "XXXXXX";
    else
        tmp_filename_format = value;
}

//------------------------------------------------------------------------------
void Config::SetMemoryFilesizeLimit(const std::string& value)
{
    std::istringstream input(value);
    uint64_t limit;
    if (input >> limit)
    {
        memory_filesize_limit = limit;
        return;
    }

    throw libecap::TextException("EXIF-eCAP: invalid memory_filesize_limit value");
}
