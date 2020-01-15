#include "MetadataFilterFactory.hpp"

#include <algorithm>

#include "Config.hpp"
#include "ExivMetadataFilter.hpp"
#include "Log.hpp"
#include "Mp3MetadataFilter.hpp"
#include "Mp4MetadataFilter.hpp"
#include "OdfMetadataFilter.hpp"
#include "OggVorbisMetadataFilter.hpp"
#include "OpenXmlMetadataFilter.hpp"
#include "PdfMetadataFilter.hpp"
#include "RuntimeMetadataFilter.hpp"

class FilterRegistry
{
public:
    void RegisterFilter(
        libecap::shared_ptr<ExifAdapter::MetadataFilter> filter)
        {
            filters.emplace_back(filter);
        }

    const ExifAdapter::MetadataFilterFactory::MetadataFilterList& GetFilters()
        {
            return filters;
        }

private:
    ExifAdapter::MetadataFilterFactory::MetadataFilterList filters;
};

static FilterRegistry* filter_registry = nullptr;

static FilterRegistry* GetFilterRegistry()
{
    if (!filter_registry)
    {
        filter_registry = new FilterRegistry();

        const std::vector<std::string>& excluded_types =
            ExifAdapter::Config::GetConfig()->GetExcludedTypes();
        libecap::shared_ptr<ExifAdapter::MetadataFilter> filter;

#define REGISTER_FILTER(name)                                       \
        filter.reset(new name());                                   \
        if (filter->DisableMimeTypes(excluded_types) != 0)          \
        {                                                           \
            ExifAdapter::Log(libecap::flXaction | libecap::ilDebug) \
                << "registered " << #name;                          \
            filter_registry->RegisterFilter(filter);                \
        }

        REGISTER_FILTER(ExifAdapter::ExivMetadataFilter);
        REGISTER_FILTER(ExifAdapter::Mp4MetadataFilter);
        REGISTER_FILTER(ExifAdapter::Mp3MetadataFilter);
        REGISTER_FILTER(ExifAdapter::OggVorbisMetadataFilter);
        REGISTER_FILTER(ExifAdapter::PdfMetadataFilter);
        REGISTER_FILTER(ExifAdapter::OdfMetadataFilter);
        REGISTER_FILTER(ExifAdapter::OpenXmlMetadataFilter);
    }

    return filter_registry;
}

static bool IsMimeTypeExcluded(const std::string& type)
{
    const std::vector<std::string>& excluded_types = ExifAdapter::Config::GetConfig()->GetExcludedTypes();
    if (std::find(excluded_types.begin(), excluded_types.end(), type) == excluded_types.end())
        return false;

    return true;
}

using namespace ExifAdapter;

//------------------------------------------------------------------------------
libecap::shared_ptr<MetadataFilter> MetadataFilterFactory::CreateFilter(
    const std::string& mime_type)
{
    if (mime_type.find("application/octet-stream") == 0)
    {
        if (IsMimeTypeExcluded("application/octet-stream"))
            return libecap::shared_ptr<MetadataFilter>();

        Log(libecap::flXaction | libecap::ilDebug)
            << "creating runtime metadata filter for "
            << "application/octet-stream mime type";
        libecap::shared_ptr<MetadataFilter> filter(
            new RuntimeMetadataFilter());
        return filter;
    }

    const MetadataFilterList& filters = GetFilterRegistry()->GetFilters();
    for (MetadataFilterList::const_iterator it = filters.begin();
         it != filters.end();
         ++it)
    {
        libecap::shared_ptr<MetadataFilter> filter = *it;
        if (filter->IsMimeTypeSupported(mime_type))
            return filter;
    }

    return libecap::shared_ptr<MetadataFilter>();
}

//------------------------------------------------------------------------------
bool MetadataFilterFactory::IsMimeTypeSupported(
    const std::string& mime_type)
{
    if (mime_type.find("multipart/form-data") == 0)
    {
        if (IsMimeTypeExcluded("multipart/form-data"))
            return false;

        return true;
    }
    else if (mime_type.find("application/octet-stream") == 0)
    {
        if (IsMimeTypeExcluded("multipart/form-data"))
            return false;

        return true;
    }

    const MetadataFilterList& filters = GetFilterRegistry()->GetFilters();
    for (MetadataFilterList::const_iterator it = filters.begin();
         it != filters.end();
         ++it)
    {
        libecap::shared_ptr<MetadataFilter> filter = *it;
        if (filter->IsMimeTypeSupported(mime_type))
            return true;
    }

    return false;
}

//------------------------------------------------------------------------------
const MetadataFilterFactory::MetadataFilterList& MetadataFilterFactory::GetFilters()
{
    return GetFilterRegistry()->GetFilters();
}
