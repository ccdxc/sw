
/*
 * {C} Copyright 2020 Pensando Systems Inc. All rights reserved
 *
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/time.h>

#define PRINT_TTIME_STR_LEN 1024
#define USEC_PER_SEC 1000000L

void myprintf(int level, char *format, ...)
{
    return;
    char       logbuf[1024];
    va_list    args;

    va_start(args, format);                                                                                           
    vsnprintf(logbuf, sizeof(logbuf), format, args);                                                                  
    printf("%s\n", logbuf);
}

typedef struct ttime_ {
    uint32_t       count;
    struct timeval time_taken;
} ttime_t;

#define GEN_ENUM_ENTRY(enum_entry, val, func) \
    enum_entry=val,

#define TEST_ENUM_CASE_FUNC(enum_entry, val, func) \
     case   enum_entry : \
         return test_##func ; \
     break;

#define TEST_ENUM_CASE_STR(enum_entry, val, func) \
     case   enum_entry : \
         return #func ; \

#define test_entries(entry) \
    entry(VLAN_TO_VNIC_MAP_CREATE_ENUM, 1, pds_vlan_to_vnic_map_create) \
    entry(VLAN_TO_VNIC_MAP_UPDATE_ENUM, 2, pds_vlan_to_vnic_map_update) \
    entry(VLAN_TO_VNIC_MAP_READ_ENUM,   3, pds_vlan_to_vnic_map_read) \
    entry(VLAN_TO_VNIC_MAP_DELETE_ENUM, 4, pds_vlan_to_vnic_map_delete) \
    entry(MPLS_LABEL_TO_VNIC_MAP_CREATE_ENU, 5, pds_mpls_label_to_vnic_map_create) \
    entry(MPLS_LABEL_TO_VNIC_MAP_UPDATE_ENUM, 6,pds_mpls_label_to_vnic_map_update) \
    entry(MPLS_LABEL_TO_VNIC_MAP_READ_ENUM,   7, pds_mpls_label_to_vnic_map_read) \
    entry(MPLS_LABEL_TO_VNIC_MAP_DELETE_ENUM, 8, pds_mpls_label_to_vnic_map_delete) \
    entry(FLOW_CACHE_ENTRY_CREATE_ENUM, 9, pds_flow_cache_entry_create) \
    entry(FLOW_CACHE_ENTRY_UPDATE_ENUM, 10, pds_flow_cache_entry_update) \
    entry(FLOW_CACHE_ENTRY_READ_ENUM, 11, pds_flow_cache_entry_read) \
    entry(FLOW_CACHE_ENTRY_DELETE_ENUM, 12, pds_flow_cache_entry_delete) \
    entry(SESSION_INFO_ENTRY_CREATE_ENUM, 13, pds_flow_session_info_create) \
    entry(SESSION_INFO_ENTRY_UPDATE_ENUM, 14, pds_flow_session_info_update) \
    entry(SESSION_INFO_ENTRY_READ_ENUM, 15, pds_flow_session_info_read) \
    entry(SESSION_INFO_ENTRY_DELETE_ENUM, 16, pds_flow_session_info_delete) \
    entry(SESSION_REWRITE_ENTRY_CREATE_ENUM, 17, pds_flow_session_rewrite_create) \
    entry(SESSION_REWRITE_ENTRY_UPDATE_ENUM, 18, pds_flow_session_rewrite_update) \
    entry(SESSION_REWRITE_ENTRY_READ_ENUM, 19, pds_flow_session_rewrite_read) \
    entry(SESSION_REWRITE_ENTRY_DELETE_ENUM, 20, pds_flow_session_rewrite_delete) 

#define GEN_ENUM(name) \
    enum { \
        test_entries(GEN_ENUM_ENTRY) \
        MAX_CURD_ENUM, \
    }name##_t;


#define DECL_TEST_FUNC(enum_entry, val, func) \
    inline void test_##func (uint32_t);
    
#define TEST_ENUM_TO_FUNC(_typ)                          \
    void* test_enum_to_func(_typ c)                           \
    {                                                                   \
        switch (c) {                                                    \
            test_entries(TEST_ENUM_CASE_FUNC);                               \
        }                                                               \
        return NULL;                                                \
    }

#define TEST_ENUM_TO_STR(_typ )                          \
    char* test_enum_to_str(_typ c)                           \
    {                                                                   \
        switch (c) {                                                    \
            test_entries(TEST_ENUM_CASE_STR);                               \
        }                                                               \
        return NULL;                                                \
    }

GEN_ENUM(api_enum)
//test_entries(DECL_TEST_FUNC)
TEST_ENUM_TO_FUNC(int) 
TEST_ENUM_TO_STR(int) 
