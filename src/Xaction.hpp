#ifndef __XACTION_HPP__
#define __XACTION_HPP__

#include <libecap/common/forward.h>
#include <libecap/adapter/xaction.h>

#include "ContentIO.hpp"
#include "MetadataFilter.hpp"

//#define MEASURE_EXECUTION_TIME

namespace ExifAdapter
{

class Xaction:
        public libecap::adapter::Xaction
{
public:
    Xaction(libecap::host::Xaction *x);
    virtual ~Xaction();

    // meta-info for the host transaction
    virtual const libecap::Area option(const libecap::Name &name) const;
    virtual void visitEachOption(libecap::NamedValueVisitor &visitor) const;

    // lifecycle
    virtual void start();
    virtual void stop();

    // adapted body transmission control
    virtual void abDiscard();
    virtual void abMake();
    virtual void abMakeMore();
    virtual void abStopMaking();

    // adapted body content extraction and consumption
    virtual libecap::Area abContent(libecap::size_type offset, libecap::size_type size);
    virtual void abContentShift(libecap::size_type size);

    // virgin body state notification
    virtual void noteVbContentDone(bool);
    virtual void noteVbContentAvailable();

private:
    bool shouldProcess(const std::string& content_type) const;
    void createAdaptedContentIo(const std::string& content_type);
    std::string getContentType() const;

    libecap::host::Xaction *hostx; // Host transaction rep

    enum class OpState { opUndecided, opOn, opComplete, opNever };
    OpState receivingVb;
    OpState sendingAb;

    void stopVb();	/* Stops receiving vb (if we are receiving it) */

    int vb_offset;
    bool vb_at_end;

    libecap::shared_ptr<ContentIO> content;
    libecap::shared_ptr<MetadataFilter> filter;

#ifdef MEASURE_EXECUTION_TIME
    uint64_t start_time;
    uint64_t first_data_received_time;
    uint64_t all_data_received_time;
    uint64_t data_processed_time;
#endif
};

}

#endif
