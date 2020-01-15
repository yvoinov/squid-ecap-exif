#include "ContentIOFactory.hpp"

#include "Config.hpp"
#include "ContentFileIO.hpp"
#include "ContentMemoryIO.hpp"
#include "Log.hpp"
#include "MultipartContentIO.hpp"

using namespace ExifAdapter;

const char* MULTIPART_FORM_DATA_TYPE = "multipart/form-data";

//------------------------------------------------------------------------------
static libecap::shared_ptr<ContentIO> GetContentIO(
    const std::string& content_type,
    bool store_in_memory,
    uint64_t length)
{
    libecap::shared_ptr<ContentIO> content_io;

    if (content_type.find(MULTIPART_FORM_DATA_TYPE) == 0)
    {
        content_io.reset(
            new MultipartContentIO(content_type, length));
    }
    else
    if (store_in_memory)
    {
        Log(libecap::flXaction | libecap::ilDebug) << "store in memory";
        content_io.reset(new ContentMemoryIO(length));
    }
    else
    {
        Log(libecap::flXaction | libecap::ilDebug) << "store on disk";
        content_io = ContentFileIO::FromTemporaryFile();
    }

    return content_io;
}

//------------------------------------------------------------------------------
libecap::shared_ptr<ContentIO> ContentIOFactory::CreateContentIO(
    const std::string& content_type)
{
    return GetContentIO(content_type, false, 0);
}

//------------------------------------------------------------------------------
libecap::shared_ptr<ContentIO> ContentIOFactory::CreateContentIO(
    const std::string& content_type,
    uint64_t content_length,
    bool can_be_stored_in_memory)
{
    Config* config = Config::GetConfig();
    uint64_t memory_store_limit = config->GetMemoryFilesizeLimit();

    if (can_be_stored_in_memory)
        if (content_length > memory_store_limit)
            can_be_stored_in_memory = false;

    return GetContentIO(
        content_type,
        can_be_stored_in_memory,
        content_length);
}
