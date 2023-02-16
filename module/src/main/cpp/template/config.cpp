#include "config.hpp"

namespace Module {
    const int moduleVersionCode = ${MODULE_VERSION};
    const char *const moduleVersionName = "${MODULE_VERSION_NAME}";
}

#ifdef RIRU_MODULE
namespace riru {
    const int moduleApiVersion = ${RIRU_MODULE_API_VERSION};
    const int moduleMinApiVersion = ${RIRU_MODULE_MIN_API_VERSION};
}
#endif
