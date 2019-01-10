//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#ifndef __MEM_HASH_UTILS_HPP__
#define __MEM_HASH_UTILS_HPP__

#define MEM_HASH_TRACE_API_BEGIN() {\
        const char *marker = "-------------------------"; \
        SDK_TRACE_DEBUG("%s API BEGIN %s", marker, marker);\
}
#define MEM_HASH_TRACE_API_END() {\
        const char *marker = "-------------------------"; \
        SDK_TRACE_DEBUG("%s API END %s", marker, marker);\
}

#endif // __MEM_HASH_UTILS_HPP__
