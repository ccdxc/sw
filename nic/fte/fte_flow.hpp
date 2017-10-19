#pragma once

#include "fte_ctx.hpp"

namespace fte {

class flow_t {
public:
    void from_config(const hal::flow_cfg_t &config, const hal::flow_pgm_attrs_t &attrs);
    hal_ret_t to_config(hal::flow_cfg_t &config, hal::flow_pgm_attrs_t &attrs) const;
    hal_ret_t merge_flow(const flow_t& flow);
    hal_ret_t header_pop(const header_pop_info_t &header_pop);
    hal_ret_t header_push(const header_push_info_t &header_push);
    hal_ret_t header_rewrite(const header_rewrite_info_t &header_rewrite);

    void init(ctx_t *ctx) {
        *this = {};
        ctx_ = ctx;
    }

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

    hal_ret_t set_action(session::FlowAction action) {
        action_ = action;
        valid_.action = true;
        return HAL_RET_OK;
    }
    session::FlowAction action() const {
        return action_;
    };
    bool valid_action() const {
        return valid_.action;
    };

    hal_ret_t set_flow_state(const flow_state_t& flow_state) {
        flow_state_ = flow_state;
        valid_.flow_state = true;
        return HAL_RET_OK;
    }
    const flow_state_t& flow_state() const {
        return flow_state_;
    };
    bool valid_flow_state() const {
        return valid_.flow_state;
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

    hal_ret_t set_mcast_copy(const uint8_t& copy_en) {
        mcast_copy_en_ = copy_en;
        valid_.mcast_copy = true;
        return HAL_RET_OK;
    }
    const uint8_t& mcast_copy() const {
        return mcast_copy_en_;
    } 
    bool valid_mcast_copy() const {
        return valid_.mcast_copy;
    }

private:
    // Max header updates we track per flow
    // This is enough to pop the outer, rewrite inner and push new outer
    static const uint8_t MAX_HEADER_UPDATES = 3;

    ctx_t                    *ctx_;

    struct {
        uint8_t key:1;
        uint8_t action:1;
        uint8_t flow_state:1;
        uint8_t fwding:1;
        uint8_t mcast_copy:1;
    } valid_;

    hal::flow_key_t           key_;                 // flow's key
    session::FlowAction       action_;              // firwall action
    flow_state_t              flow_state_;          // connection tracking 
    fwding_info_t             fwding_;              // Fwding info
    uint8_t                   mcast_copy_en_;             // Flow copy Enable 

    uint8_t                   num_header_updates_; // no.of valid updates
    header_update_t           header_updates_[MAX_HEADER_UPDATES];

    static hal_ret_t merge_header_rewrite(header_rewrite_info_t &dst,
                                          const  header_rewrite_info_t &src);
    static hal_ret_t merge_header_rewrite_with_push(header_push_info_t &dst,
                                                    const  header_rewrite_info_t &src);
    static rewrite_actions_enum nat_rewrite_action(header_type_t l3_type,
                                                   header_type_t l4_type,
                                                   session::NatType nat_type);
    static hal_ret_t build_rewrite_config(hal::flow_pgm_attrs_t &attrs,
                                          const header_rewrite_info_t &rewrite);
    static hal_ret_t build_push_header_config(hal::flow_pgm_attrs_t &attrs,
                                              const header_push_info_t &header);
};

} // namespace fte
