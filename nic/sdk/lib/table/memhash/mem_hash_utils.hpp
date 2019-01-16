//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#ifndef __MEM_HASH_UTILS_HPP__
#define __MEM_HASH_UTILS_HPP__

#define MEM_HASH_TRACE_API_BEGIN(_name) {\
        const char *marker = "-------------------------"; \
        SDK_TRACE_DEBUG("%s Api Begin For Table: %s %s",\
                        marker, _name, marker);\
}
#define MEM_HASH_TRACE_API_END(_name) {\
        const char *marker = "-------------------------"; \
        SDK_TRACE_DEBUG("%s Api End For Table: %s %s",\
                        marker, _name, marker);\
}

char*
mem_hash_utils_rawstr(void *data, uint32_t len);

#endif // __MEM_HASH_UTILS_HPP__
