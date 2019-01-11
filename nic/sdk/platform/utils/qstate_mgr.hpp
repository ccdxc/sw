// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

#ifndef __SDK_PLATFORM_PQSTATE_HPP__
#define __SDK_PLATFORM_PQSTATE_HPP__

#include <string>
#include <map>
#include "include/sdk/base.hpp"
#include "include/sdk/mem.hpp"
#include "lib/shmmgr/shmmgr.hpp"
#include "include/sdk/types.hpp"
#include "boost/foreach.hpp"
#include "boost/optional.hpp"
#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/json_parser.hpp"
#include "include/sdk/platform.hpp"

#define NUM_QUEUE_TYPES 8

namespace sdk {
namespace platform {
namespace utils {

    typedef struct qstate_info_s {
        uint32_t qsize[NUM_QUEUE_TYPES];
        hbm_addr_t qaddr[NUM_QUEUE_TYPES];
    } __PACK__ qstate_info_t;

    class qstate_mgr {
    public:
        // constructor to build qstate_mgr instance given full path of the json
        // file containing nicmgr generated information
        static qstate_mgr *factory(const char *qstate_info_file,
                                   shmmgr *mmgr = NULL);

        static void destroy(qstate_mgr *qsm);

        hbm_addr_t get_lifid_qstate_address(uint64_t lifid, uint8_t qtype,
                                            uint32_t qid = 0) const;

        hbm_addr_t get_hwlifid_qstate_address(uint64_t hwlifid, uint8_t qtype,
                                              uint32_t qid = 0) const;

    private:
        shmmgr *mmgr_;
        std::map<uint64_t, qstate_info_t> lifid2qsinfo_;
        std::map<uint64_t, qstate_info_t> hwlifid2qsinfo_;

    private:
        qstate_mgr() {};

        ~qstate_mgr();

        bool init(const char *qstate_info_file, shmmgr *mmgr = NULL);
    };

}    // namespace utils
}    // namespace platform
}    // namespace sdk

#endif // __SDK_PLATFORM_PQSTATE_HPP__
