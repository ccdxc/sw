//-----------------------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------
#ifndef __FTL_STATS_HPP__
#define __FTL_STATS_HPP__

namespace sdk {
namespace table {
namespace ftlint {

class apistats {
private:
    uint64_t insert_;
    uint64_t insert_duplicate_;
    uint64_t insert_fail_;
    uint64_t insert_recirc_fail_;
    uint64_t remove_;
    uint64_t remove_not_found_;
    uint64_t remove_fail_;
    uint64_t update_;
    uint64_t update_fail_;
    uint64_t get_;
    uint64_t get_fail_;
    uint64_t release_;
    uint64_t release_fail_;

public:
    apistats() {
        insert_ = 0;
        insert_duplicate_ = 0;
        insert_fail_ = 0;
        remove_ = 0;
        remove_not_found_ = 0;
        remove_fail_ = 0;
        update_ = 0;
        update_fail_ = 0;
        get_ = 0;
        get_fail_ = 0;
        release_ = 0;
        release_fail_ = 0;
    }

    ~apistats() {
    }
    
    void clear() {
        memset(this, 0, sizeof(apistats));
    }
    
    sdk_ret_t insert(sdk_ret_t status) {
        //FTL_TRACE_VERBOSE("Updating insert stats, ret:%d", status);
        if (status == SDK_RET_OK) {
            insert_++;
        } else if (status == SDK_RET_ENTRY_EXISTS) {
            insert_duplicate_++;
        } else if (status == SDK_RET_MAX_RECIRC_EXCEED) {
            insert_recirc_fail_++;
        } else {
            insert_fail_++;
        }
        return SDK_RET_OK;
    }

    sdk_ret_t update(sdk_ret_t status) {
        //FTL_TRACE_VERBOSE("Updating update stats, ret:%d", status);
        if (status == SDK_RET_OK) {
            update_++;
        } else {
            update_fail_++;
        }
        return SDK_RET_OK;
    }

    sdk_ret_t remove(sdk_ret_t status) {
        //FTL_TRACE_VERBOSE("Updating remove stats, ret:%d", status);
        if (status == SDK_RET_OK) {
            remove_++;
        } else if (status == SDK_RET_ENTRY_NOT_FOUND) {
            remove_not_found_++;
        } else {
            remove_fail_++;
        }
        return SDK_RET_OK;
    }

    sdk_ret_t release(sdk_ret_t status) {
        //FTL_TRACE_VERBOSE("Updating release stats, ret:%d", status);
        if (status == SDK_RET_OK) {
            release_++;
        } else {
            release_fail_++;
        }
        return SDK_RET_OK;
    }

    sdk_ret_t get(sdk_ret_t status) {
        //FTL_TRACE_VERBOSE("Updating get stats, ret:%d", status);
        if (status == SDK_RET_OK) {
            get_++;
        } else {
            get_fail_++;
        }
        return SDK_RET_OK;
    }

    sdk_ret_t get(sdk_table_api_stats_t *stats) {
        stats->insert = insert_;
        stats->insert_duplicate = insert_duplicate_;
        stats->insert_fail = insert_fail_;
        stats->insert_recirc_fail = insert_recirc_fail_;
        stats->remove = remove_;
        stats->remove_not_found = remove_not_found_;
        stats->remove_fail = remove_fail_;
        stats->update = update_;
        stats->update_fail = update_fail_;
        stats->get = get_;
        stats->get_fail = get_fail_;
        stats->release = release_;
        stats->release_fail = release_fail_;
        stats->reserve = 0;
        stats->reserve_fail = 0;
        return SDK_RET_OK;
    }


};

class tablestats {
private:
    uint64_t    insert_; //Total entries inserted
    uint64_t    remove_; //Total entries removed
    uint64_t    read_; //Total entries read
    uint64_t    write_; //Total entries written
    uint64_t    entries_; //Total entries in the Table
    uint64_t    collisions_; //Total collisions incountred while insert
    uint64_t    insert_lvl_[SDK_TABLE_MAX_RECIRC]; //Entries inserted in level
    uint64_t    remove_lvl_[SDK_TABLE_MAX_RECIRC]; //Entries inserted in level

public:
    tablestats() {
        insert_ = remove_ = 0;
        read_ = write_ = 0;
        memset(&insert_lvl_, 0, sizeof(insert_lvl_));
        memset(&insert_lvl_, 0, sizeof(remove_lvl_));
    }

    ~tablestats() {
    }

    void clear() {
        memset(this, 0, sizeof(tablestats));
    }

    sdk_ret_t inc_entries() {
        entries_ ++;
        return SDK_RET_OK;
    }

    sdk_ret_t inc_collisions() {
        collisions_ ++;
        return SDK_RET_OK;
    }

    sdk_ret_t dec_entries() {
        entries_ --;
        return SDK_RET_OK;
    }

    sdk_ret_t dec_collisions() {
        collisions_ --;
        return SDK_RET_OK;
    }

    sdk_ret_t insert(uint32_t lvl) {
        insert_++;
        insert_lvl_[lvl]++;
        return SDK_RET_OK;
    }

    sdk_ret_t remove(uint32_t lvl) {
        remove_++;
        remove_lvl_[lvl]++;
        return SDK_RET_OK;
    }

    sdk_ret_t read(uint32_t lvl) {
        read_++;
        return SDK_RET_OK;
    }

    sdk_ret_t write(uint32_t lvl) {
        write_++;
        return SDK_RET_OK;
    }

    sdk_ret_t get(sdk_table_stats_t *stats) {
        stats->insert = insert_;
        stats->remove = remove_;
        for (int i = 0; i < SDK_TABLE_MAX_RECIRC; i++) {
            stats->insert_lvl[i] = insert_lvl_[i];
            stats->remove_lvl[i] = remove_lvl_[i];
        }
        stats->read = read_;
        stats->write = write_;
        stats->entries = entries_;
        stats->collisions = collisions_;
        return SDK_RET_OK;
    }
};

} // namespace ftlint
} // namespace table
} // namespace sdk

#endif // __FTL_STATS_HPP__
