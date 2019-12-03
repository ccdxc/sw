#pragma once

#include "fte_ctx.hpp"
#include "nic/hal/plugins/cfg/nw/session.hpp"
#include "nic/include/pd_api.hpp"

namespace fte {

class flow_t {
public:
    void from_config(const hal::flow_cfg_t &config, const hal::flow_pgm_attrs_t &attrs, const hal::session_t *session);
    hal_ret_t to_config(hal::flow_cfg_t &config, hal::flow_pgm_attrs_t &attrs) const;
    hal_ret_t merge_flow(const flow_t& flow);
    hal_ret_t header_pop(const header_pop_info_t &header_pop);
    hal_ret_t header_push(const header_push_info_t &header_push);
    hal_ret_t header_rewrite(const header_rewrite_info_t &header_rewrite);

    void init(ctx_t *ctx) {
        bzero(this, sizeof(*this));
        num_header_updates_ = 0;
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
        if (action_ == action) {
            return HAL_RET_ENTRY_EXISTS;
        }
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
        if (!memcmp(&fwding_, &fwding, sizeof(fwding_info_t))) {
            return HAL_RET_ENTRY_EXISTS;
        }

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

    hal_ret_t set_mcast_info(const mcast_info_t& mcast_info) {
        mcast_info_ = mcast_info;
        valid_.mcast_info = true;
        return HAL_RET_OK;
    }
    hal_ret_t merge_mcast_info(const mcast_info_t& mcast_info) {

        // Merge the argument mcast_info; the final evaluation of which
        // mcast ptr is ultimately applicable will be done in fte::to_config()

        if (!memcmp(&mcast_info, &mcast_info_, sizeof(mcast_info_t))) {
            return HAL_RET_ENTRY_EXISTS;
        }

        if (mcast_info.mcast_en) {
            if (mcast_info.mcast_ptr) {
                mcast_info_.mcast_ptr = mcast_info.mcast_ptr;
            }
            if (mcast_info.proxy_mcast_ptr) {
                mcast_info_.proxy_mcast_ptr = mcast_info.proxy_mcast_ptr;
            }
        } else {
            if (mcast_info.mcast_ptr) {
                mcast_info_.mcast_ptr = 0;
            }
            if (mcast_info.proxy_mcast_ptr) {
                mcast_info_.proxy_mcast_ptr = 0;
            }
        }

        mcast_info_.mcast_en = mcast_info_.mcast_ptr || mcast_info_.proxy_mcast_ptr;
        valid_.mcast_info = true;
        return HAL_RET_OK;
    }
    const mcast_info_t& mcast_info() const {
        return mcast_info_;
    }
    bool valid_mcast_info() const {
        return valid_.mcast_info;
    }

    hal_ret_t set_ingress_info(const ingress_info_t& ingress_info) {
        hal::pd::pd_if_get_hw_lif_id_args_t args;
        hal::pd::pd_func_args_t pd_func_args = {0};

        args.pi_if = ingress_info.expected_sif;
        pd_func_args.pd_if_get_hw_lif_id = &args;
        hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_IF_GET_HW_LIF_ID, &pd_func_args);
        if (args.hw_lif_id == ingress_info_.hw_lif_id) {
            return HAL_RET_ENTRY_EXISTS;
        }

        ingress_info_ = ingress_info;
        valid_.ingress_info = true;
        ingress_info_.hw_lif_id = args.hw_lif_id;

        return HAL_RET_OK;
    }
    const ingress_info_t& ingress_info() const {
        return ingress_info_;
    }
    bool valid_ingress_info() const {
        return valid_.ingress_info;
    }

    hal_ret_t set_lkp_info(const lkp_info_t& lkp_info) {
        lkp_info_ = lkp_info;
        valid_.lkp_info = true;
        return HAL_RET_OK;
    }
    const lkp_info_t& lkp_info() const {
        return lkp_info_;
    }
    bool valid_lkp_info() const {
        return valid_.lkp_info;
    }

    hal_ret_t merge_mirror_info(const mirror_info_t& mirror_info) {
        if (!memcmp(&mirror_info, &mirror_info_, sizeof(mirror_info_t))) {
            return HAL_RET_ENTRY_EXISTS;
        }

        mirror_info_.mirror_en = mirror_info.mirror_en;
        mirror_info_.egr_mirror_session = mirror_info.egr_mirror_session;
        mirror_info_.ing_mirror_session = mirror_info.ing_mirror_session;
        mirror_info_.proxy_egr_mirror_session = mirror_info.proxy_egr_mirror_session;
        mirror_info_.proxy_ing_mirror_session = mirror_info.proxy_ing_mirror_session;
        valid_.mirror_info = true;
        return HAL_RET_OK;
    }
    const mirror_info_t& mirror_info() const {
        return mirror_info_;
    }
    bool valid_mirror_info() const {
        return valid_.mirror_info;
    }

    hal_ret_t merge_export_info(const export_info_t& export_info) {
        if (!memcmp(&export_info, &export_info_, sizeof(export_info_t))) {
            return HAL_RET_ENTRY_EXISTS;
        }
        export_info_.export_en = export_info.export_en;
        export_info_.export_id1 = (export_info.export_en & (1 << 0)) ?
                                   export_info.export_id1 : 0;
        export_info_.export_id2 = (export_info.export_en & (1 << 1)) ?
                                   export_info.export_id2 : 0;
        export_info_.export_id3 = (export_info.export_en & (1 << 2)) ?
                                   export_info.export_id3 : 0;
        export_info_.export_id4 = (export_info.export_en & (1 << 3)) ?
                                   export_info.export_id4 : 0;
        valid_.export_info = true;
        return HAL_RET_OK;
    }
    const export_info_t& export_info() const {
        return export_info_;
    }
    bool valid_export_info() const {
        return valid_.export_info;
    }

    hal_ret_t set_qos_info(const qos_info_t& qos_info) {
        if (!memcmp(&qos_info, &qos_info_, sizeof(qos_info_t))) {
            return HAL_RET_ENTRY_EXISTS;
        }

        qos_info_ = qos_info;
        valid_.qos_info = true;
        return HAL_RET_OK;
    }
    const qos_info_t& qos_info() const {
        return qos_info_;
    }
    bool valid_qos_info() const {
        return valid_.qos_info;
    }

    bool is_proxy_enabled() const {
        return is_proxy_enabled_;
    }

    header_rewrite_info_t header_rewrite_info() const {
        header_rewrite_info_t info = {};

        for (int i = 0; i < num_header_updates_; i++) {
            const header_update_t *entry = &header_updates_[i];
            if (entry->type == HEADER_REWRITE) {
                return (entry->header_rewrite);
            }
        }
        return info;
    }

    header_push_info_t header_push_info() const {
       header_push_info_t info = {};

       for (int i = 0; i < num_header_updates_; i++) {
            const header_update_t *entry = &header_updates_[i];
            if (entry->type == HEADER_PUSH) {
                return (entry->header_push);
            }
        }
        return info;
    }

    hal_ret_t set_sfw_info(const sfw_flow_info_t sfw_info) {
        sfw_info_ = sfw_info;
        valid_.sfw_info = true;
        return HAL_RET_OK;
    }

    bool valid_sfw_info() const {
        return valid_.sfw_info;
    }

    const sfw_flow_info_t& sfw_info() const {
        return sfw_info_;
    }

    hal_ret_t set_aging_info(const aging_info_t& aging_info) {
        aging_info_ = aging_info;
        valid_.aging_info = true;
        return HAL_RET_OK;
    }
    const aging_info_t& aging_info() const {
        return aging_info_;
    }
    bool valid_aging_info() const {
        return valid_.aging_info;
    }

    // Store the mac for remote EPs. We might need those
    // to construct the keepalives in case of TCP
    hal_ret_t set_l2_info(const hal::flow_key_t &key) {
        l2_info_ = key;
        valid_.l2_info = true;
        return HAL_RET_OK;
    }
    const hal::flow_key_t& l2_info() const {
        return l2_info_;
    };
    bool valid_l2_info() const {
        return valid_.l2_info;
    };

private:
    // Max header updates we track per flow
    // This is enough to pop the outer, rewrite inner and push new outer
    static const uint8_t MAX_HEADER_UPDATES = 3;

    ctx_t                    *ctx_;

    struct {
        uint16_t key:1;
        uint16_t attrs:1;
        uint16_t action:1;
        uint16_t flow_state:1;
        uint16_t fwding:1;
        uint16_t mcast_info:1;
        uint16_t ingress_info:1;
        uint16_t mirror_info:1;
        uint16_t qos_info:1;
        uint16_t lkp_info:1;
        uint16_t export_info:1;
        uint16_t aging_info:1;
        uint16_t l2_info:1;
        uint16_t sfw_info:1;
     } valid_;

    hal::flow_key_t           key_;                 // flow's key
    hal::flow_pgm_attrs_t     attrs_;               // Restored flow attrs
    hal::flow_key_t           l2_info_;             // flow's l2 info

    session::FlowAction       action_;              // firwall action
    flow_state_t              flow_state_;          // connection tracking
    fwding_info_t             fwding_;              // Fwding info
    mcast_info_t              mcast_info_;          // Mulitcast Enable
    ingress_info_t            ingress_info_;        // Ingress info (src if check)
    mirror_info_t             mirror_info_;         // Mirror info
    qos_info_t                qos_info_;            // Qos Info
    lkp_info_t                lkp_info_;            // Flow lookup info
    export_info_t             export_info_;         // Flow Export info
    aging_info_t              aging_info_;          // Aging info
    sfw_flow_info_t           sfw_info_;            // SFW info

    uint8_t                   num_header_updates_; // no.of valid updates
    header_update_t           header_updates_[MAX_HEADER_UPDATES];
    bool                      is_proxy_enabled_;     // used only on existing sessions

    static hal_ret_t merge_header_rewrite(header_rewrite_info_t &dst,
                                          const  header_rewrite_info_t &src);
    static hal_ret_t merge_header_rewrite_with_push(header_push_info_t &dst,
                                                    const  header_rewrite_info_t &src);
    static hal::rewrite_actions_enum nat_rewrite_action(header_type_t l3_type,
                                                        header_type_t l4_type,
                                                        session::NatType nat_type);
    hal_ret_t build_rewrite_config(hal::flow_cfg_t &config,
                                          hal::flow_pgm_attrs_t &attrs,
                                          const header_rewrite_info_t &rewrite) const;
    hal_ret_t build_push_header_config(hal::flow_pgm_attrs_t &attrs,
                                       const header_push_info_t &header) const;

    hal_ret_t get_rewrite_config(const hal::flow_cfg_t &flow_cfg,
                                 const hal::flow_pgm_attrs_t  &attrs,
                                 header_rewrite_info_t *rewrite);
};

} // namespace fte
