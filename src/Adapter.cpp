#include <libecap/common/registry.h>

#include "Service.hpp"

// create the adapter and register with libecap to reach the host application
static const bool Registered = (libecap::RegisterVersionedService(new ExifAdapter::Service));
