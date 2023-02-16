#pragma once

namespace Module {
    extern const int moduleVersionCode;
    extern const char *const moduleVersionName;
}

#ifdef RIRU_MODULE
namespace riru {
    extern const int moduleApiVersion;
    extern const int moduleMinApiVersion;
}
#endif
