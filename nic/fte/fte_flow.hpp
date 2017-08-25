#pragma once

#include "fte_ctx.hpp"

namespace fte {

class flow_t {
public:
    void from_config(const hal::flow_cfg_t &flow_cfg);
    hal_ret_t to_config(hal::flow_cfg_t &flow_cfg) const;
    hal_ret_t merge_flow(const flow_t& flow);
    hal_ret_t header_pop(const header_pop_info_t &header_pop);
    hal_ret_t header_push(const header_push_info_t &header_push);
    hal_ret_t header_rewrite(const header_rewrite_info_t &header_rewrite);

    hal_ret_t set_key(const hal::flow_key_t &key) {
        key_ = key;
        valid_.key = true;
        return HAL_RET_OK;
    } 
    const hal::flow_key_t& key() const {
        return key_;
    };
    bool valid_key() const {
        return valid_.key;
    };

    hal_ret_t set_action(const action_info_t& action) {
        action_ = action;
        valid_.action = true;
        return HAL_RET_OK;
    }
    const action_info_t& action() const {
        return action_;
    };
    bool valid_action() const {
        return valid_.action;
    };

    hal_ret_t set_conn_track(const conn_track_info_t& conn_track) {
        conn_track_ = conn_track;
        valid_.conn_track = true;
        return HAL_RET_OK;
    }
    const conn_track_info_t& conn_track() const {
        return conn_track_;
    };
    bool valid_conn_track() const {
        return valid_.conn_track;
    };

    hal_ret_t set_fwding(const fwding_info_t& fwding) {
        fwding_ = fwding;
        valid_.fwding = true;
        return HAL_RET_OK;
    }
    const fwding_info_t& fwding() const {
        return fwding_;
    };
    bool valid_fwding() const {
        return valid_.fwding;
    };

private:
    // Max header updates we track per flow
    // This is enough to pop the outer, rewrite inner and push new outer
    static const uint8_t MAX_HEADER_UPDATES = 3;

    struct {
        uint8_t key:1;
        uint8_t action:1;
        uint8_t conn_track:1;
        uint8_t fwding:1;
    } valid_;

    hal::flow_key_t           key_;                  // flow's key
    action_info_t             action_;              // firwall action
    conn_track_info_t         conn_track_;          // connection tracking 
    fwding_info_t             fwding_;              // Fwding info

    uint8_t                   num_header_updates_; // no.of valid updates
    header_update_t           header_updates_[MAX_HEADER_UPDATES];

    static hal_ret_t merge_header_rewrite(header_rewrite_info_t &dst,
                                          const  header_rewrite_info_t &src);
    static hal_ret_t merge_header_rewrite_with_push(header_push_info_t &dst,
                                                    const  header_rewrite_info_t &src);
    static rewrite_actions_enum nat_rewrite_action(header_type_t l3_type,
                                                   header_type_t l4_type, int nat_type);
    static hal_ret_t build_rewrite_config(hal::flow_cfg_t &flow_cfg,
                                   const header_rewrite_info_t &rewrite);
};

} // namespace fte
