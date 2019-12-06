//-----------------------------------------------------------------------------
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved
//-----------------------------------------------------------------------------

#include "nic/hal/plugins/network/net_plugin.hpp"
#include "nic/hal/plugins/cfg/nw/interface_api.hpp"
#include "nic/include/pd_api.hpp"

namespace hal {
namespace plugins {
namespace network {


static inline hal_ret_t
update_flow_from_qos_spec(fte::ctx_t& ctx)
{
    hal_ret_t ret = HAL_RET_OK;
    hal::qos_class_t *qos_class = NULL;
    hal::qos_class_t *tx_qos_class = NULL;
    hal::qos_class_t *rx_qos_class = NULL;
    hal::qos_group_t qos_group;
    bool qos_class_en = false;
    uint32_t qos_class_id = 0;
    hal::if_t *dif = ctx.dif();
    hal::if_t *sif = ctx.sif();
    bool proxy_enabled;
    bool proxy_flow;
    static hal::qos_class_t *def_qos = find_qos_class_by_group(hal::QOS_GROUP_DEFAULT);

    proxy_enabled = ctx.is_proxy_enabled();
    proxy_flow = ctx.is_proxy_flow();

    HAL_TRACE_VERBOSE("proxy_enabled {} proxy_flow {} "
                      "enic_dif {} enic_sif {}",
                      proxy_enabled, proxy_flow,
                      dif && (dif->if_type == intf::IF_TYPE_ENIC),
                      sif && (sif->if_type == intf::IF_TYPE_ENIC));

    if (dif && (dif->if_type == intf::IF_TYPE_ENIC)) {
        hal::lif_t *dlif = if_get_lif(dif);
        rx_qos_class = lif_get_rx_qos_class(dlif);
    } else if (sif && (sif->if_type == intf::IF_TYPE_ENIC)) {
        hal::lif_t *slif = if_get_lif(sif);
        if (slif) tx_qos_class = lif_get_tx_qos_class(slif);
    }

    if (proxy_enabled) {
        if (proxy_flow) {
            if (dif && (dif->if_type == intf::IF_TYPE_ENIC)) {
                // The flow is coming from uplink towards the p4plus.
                // Use the proxy classes
                if (rx_qos_class && rx_qos_class->no_drop) {
                    qos_group = QOS_GROUP_RX_PROXY_NO_DROP;
                } else {
                    qos_group = QOS_GROUP_RX_PROXY_DROP;
                }
                qos_class = find_qos_class_by_group(qos_group);
            } else if (sif && (sif->if_type == intf::IF_TYPE_ENIC)) {
                // The flow is the proxy flow towards the uplink. Use the
                // qos-class of the src lif
                qos_class = tx_qos_class;
            }
        } else {
            if (dif && (dif->if_type == intf::IF_TYPE_ENIC)) {
                // The flow is towards the host
                // Use the host rx class
                qos_class = rx_qos_class;
            } else if (sif && (sif->if_type == intf::IF_TYPE_ENIC)) {
                // The flow is coming from host towards the p4plus.
                // Use the proxy classes
                if (tx_qos_class && tx_qos_class->no_drop) {
                    qos_group = hal::QOS_GROUP_TX_PROXY_NO_DROP;
                } else {
                    qos_group = hal::QOS_GROUP_TX_PROXY_DROP;
                }
                qos_class = find_qos_class_by_group(qos_group);
            }
        }
    } else {
        if (dif && dif->if_type == intf::IF_TYPE_ENIC) {
            // The flow is towards the host
            // Use the host rx class
            qos_class = rx_qos_class;
        }
    }

    if (!rx_qos_class && !tx_qos_class) {
        qos_class = def_qos;
    }

    if (qos_class) {
        qos_class_en = true;
        pd::pd_qos_class_get_qos_class_id_args_t args;
        pd::pd_func_args_t pd_func_args = {0};
        args.qos_class = qos_class;
        args.dest_if = dif;
        args.qos_class_id = &qos_class_id;
        pd_func_args.pd_qos_class_get_qos_class_id = &args;
        ret = hal::pd::hal_pd_call(hal::pd::PD_FUNC_ID_GET_QOS_CLASSID, &pd_func_args);
        // ret = hal::pd::qos_class_get_qos_class_id(qos_class, dif, &qos_class_id);
        if (ret != HAL_RET_OK) {
            return ret;
        }
        HAL_TRACE_VERBOSE("qos_class_en {} qos_class {} qos_class_id {}",
                          qos_class_en, qos_class->key, qos_class_id);
    }

    fte::flow_update_t flowupd = {type: fte::FLOWUPD_QOS_INFO};
    flowupd.qos_info.qos_class_en = qos_class_en;
    flowupd.qos_info.qos_class_id = qos_class_id;
    return ctx.update_flow(flowupd);
}

fte::pipeline_action_t
qos_exec(fte::ctx_t& ctx)
{
    hal_ret_t ret = HAL_RET_OK;

    ret = update_flow_from_qos_spec(ctx);
    if (ret != HAL_RET_OK) {
        ctx.set_feature_status(ret);
        return fte::PIPELINE_END;
    }

    return fte::PIPELINE_CONTINUE;
}

} // namespace network
} // namespace plugins
} // namespace hal
