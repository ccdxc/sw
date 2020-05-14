// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

#include "include/sdk/types.hpp"
#include "platform/utils/qstate_mgr.hpp"

#define JSON_KEY_LIFS            "lifs"
#define JSON_KEY_LIF_ID          "lif_id"
#define JSON_KEY_HW_LIF_ID       "hw_lif_id"
#define JSON_KEY_QSTATES         "qstates"
#define JSON_KEY_QTYPE           "qtype"
#define JSON_KEY_QSIZE           "qsize"
#define JSON_KEY_ADDRESS         "qaddr"

namespace pt = boost::property_tree;

namespace sdk {
namespace platform {
namespace utils {

bool
qstate_mgr::init(const char *qstate_info_file, shmmgr *mmgr)
{
    pt::ptree        json_pt;

    mmgr_ = mmgr;
    std::ifstream qstateinfo(qstate_info_file);
    read_json(qstateinfo, json_pt);

    BOOST_FOREACH(pt::ptree::value_type& lif, json_pt.get_child(JSON_KEY_LIFS)) {
        std::string lifid_s = lif.second.get<std::string>(JSON_KEY_LIF_ID);
        std::string hwlifid_s = lif.second.get<std::string>(JSON_KEY_HW_LIF_ID);
        uint64_t lifid = (uint64_t)std::stol(lifid_s);
        uint64_t hwlifid = (uint64_t)std::stol(hwlifid_s);
        qstate_info_t    qsinfo = {0};

        if (lifid2qsinfo_.find(lifid) != lifid2qsinfo_.end()) {
            SDK_TRACE_ERR("lifid %lu info exists already !! duplicate ??", lifid);
            return false;
        }
        if (hwlifid2qsinfo_.find(hwlifid) != hwlifid2qsinfo_.end()) {
            SDK_TRACE_ERR("hwlifid %lu info exists already !! duplicate ??", hwlifid);
            return false;
        }

        // iterate through all the qstates now
        pt::ptree& qstates = lif.second.get_child(JSON_KEY_QSTATES);
        BOOST_FOREACH(pt::ptree::value_type& qs, qstates) {
            std::string qtype_s = qs.second.get<std::string>(JSON_KEY_QTYPE);
            std::string qsize_s = qs.second.get<std::string>(JSON_KEY_QSIZE);
            std::string qaddr_s = qs.second.get<std::string>(JSON_KEY_ADDRESS);
            auto qtype = (uint8_t) std::stol(qtype_s);
            qsinfo.qsize[qtype] = (uint32_t) std::stol(qsize_s);
            qsinfo.qaddr[qtype] = (uint64_t) std::stol(qaddr_s);
            SDK_TRACE_DEBUG("added lifid %s hwlifid %s, qtype %s qsize %s addr %s",
                            lifid_s.c_str(), hwlifid_s.c_str(), qtype_s.c_str(),
                            qsize_s.c_str(), qaddr_s.c_str());
        }

        lifid2qsinfo_[lifid] = qsinfo;
        hwlifid2qsinfo_[hwlifid] = qsinfo;
    }

    return true;
}

qstate_mgr *
qstate_mgr::factory(const char *qstate_info_file, shmmgr *mmgr)
{
    void          *mem;
    qstate_mgr    *new_qstate_mgr;

    if (qstate_info_file == nullptr) {
        return NULL;
    }

    if (access(qstate_info_file, R_OK) < 0) {
        SDK_TRACE_ERR("File %s doesn't exist or not accessible",
                      qstate_info_file);
        return NULL;
    }

    if (mmgr) {
        mem = mmgr->alloc(sizeof(qstate_mgr), 4, true);
    } else {
        mem = SDK_CALLOC(SDK_MEM_ALLOC_LIB_PLATFORM, sizeof(qstate_mgr));
    }
    if (mem == NULL) {
        SDK_TRACE_ERR("Failed to create program info instance for %s",
                      qstate_info_file);
        return NULL;
    }

    new_qstate_mgr = new (mem) qstate_mgr();
    if (new_qstate_mgr->init(qstate_info_file) == false) {
        SDK_TRACE_ERR("Failed to initialize program info for %s",
                      qstate_info_file);
        new_qstate_mgr->~qstate_mgr();
        if (mmgr) {
            mmgr->free(mem);
        } else {
            SDK_FREE(SDK_MEM_ALLOC_LIB_PLATFORM, mem);
        }
        return NULL;
    }
    return new_qstate_mgr;
}

qstate_mgr::~qstate_mgr()
{
}

void
qstate_mgr::destroy(qstate_mgr *qsm)
{
    shmmgr    *mmgr;

    if (!qsm) {
        return;
    }

    mmgr = qsm->mmgr_;
    qsm->~qstate_mgr();
    if (mmgr) {
        mmgr->free(qsm);
    } else {
        SDK_FREE(SDK_MEM_ALLOC_LIB_PLATFORM, qsm);
    }
}

mem_addr_t
qstate_mgr::get_lifid_qstate_address(uint64_t lifid, uint8_t qtype,
                                     uint32_t qid) const
{
    auto it = lifid2qsinfo_.find(lifid);
    if (it == lifid2qsinfo_.end()) {
        return SDK_INVALID_HBM_ADDRESS;
    }
    if (qtype >= NUM_QUEUE_TYPES || it->second.qsize[qtype] == 0) {
        return SDK_INVALID_HBM_ADDRESS;
    }
    return (it->second.qaddr[qtype] + (qid * it->second.qsize[qtype]));
}

mem_addr_t
qstate_mgr::get_hwlifid_qstate_address(uint64_t hwlifid, uint8_t qtype,
                                       uint32_t qid) const
{
    auto it = hwlifid2qsinfo_.find(hwlifid);
    if (it == hwlifid2qsinfo_.end()) {
        return SDK_INVALID_HBM_ADDRESS;
    }
    if (qtype >= NUM_QUEUE_TYPES || it->second.qsize[qtype] == 0) {
        return SDK_INVALID_HBM_ADDRESS;
    }
    return (it->second.qaddr[qtype] + (qid * it->second.qsize[qtype]));
}

}    // namespace utils
}    // namespace platform
}    // namespace sdk
