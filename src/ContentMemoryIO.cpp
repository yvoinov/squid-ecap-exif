#include "ContentMemoryIO.hpp"

#include <cstdlib>
#include <cstring>
#include <stdexcept>

#include <libecap/common/errors.h>

#include "MemoryAreaDetails.hpp"

using namespace ExifAdapter;

//------------------------------------------------------------------------------
ContentMemoryIO::ContentMemoryIO(libecap::size_type expected_size)
    : size(expected_size)
    , offset(0)
    , written(0)
{
    buffer = static_cast<uint8_t*>(std::malloc(expected_size));
    if (!buffer)
        throw libecap::TextException("Failed to allocate memory");
    std::memset(buffer, 0, expected_size);
}

//------------------------------------------------------------------------------
ContentMemoryIO::~ContentMemoryIO()
{
    std::free(buffer);
}

//------------------------------------------------------------------------------
libecap::size_type ContentMemoryIO::Write(const libecap::Area& data)
{
    const libecap::size_type new_size = data.size + offset;
    if (new_size > size)
    {
        uint8_t* buffer = static_cast<uint8_t*>(
            std::realloc(this->buffer, new_size));
        if (!buffer)
            throw libecap::TextException("Failed to allocate memory");
        this->buffer = buffer;
        size = new_size;
    }

    memcpy(buffer + offset, data.start, data.size);
    offset += data.size;
    written += data.size;

    return data.size;
}

//------------------------------------------------------------------------------
void ContentMemoryIO::Shift(libecap::size_type bytes)
{
    offset += bytes;
}

//------------------------------------------------------------------------------
void ContentMemoryIO::ResetOffset()
{
    offset = 0;
}

//------------------------------------------------------------------------------
libecap::Area ContentMemoryIO::Read(
    libecap::size_type offset,
    libecap::size_type size)
{
    const libecap::size_type position = this->offset + offset;

    if (position > this->written)
        return libecap::Area();

    libecap::size_type correct_size =
        std::min(size, this->written - position);

    if (correct_size == 0)
        return libecap::Area();

    libecap::shared_ptr<MemoryAreaDetails> details(
        new MemoryAreaDetails(correct_size));
    memcpy(details->GetAreaStart(), this->buffer + position, correct_size);
    return libecap::Area(details->GetAreaStart(), correct_size, details);
}

//------------------------------------------------------------------------------
void ContentMemoryIO::ApplyFilter(
    libecap::shared_ptr<MetadataFilter> filter)
{
    if (!filter)
        return;

    int size = this->written; // because of data type
    filter->ProcessMemory(&buffer, &size);
    this->written = size;
}

//------------------------------------------------------------------------------
uint64_t ContentMemoryIO::GetLength() const
{
    return written;
}
