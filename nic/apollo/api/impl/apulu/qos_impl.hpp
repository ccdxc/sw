//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//----------------------------------------------------------------------------
///
/// \file
/// datapath implementation of qos
///
//----------------------------------------------------------------------------

#include "include/sdk/base.hpp"
#include "include/sdk/qos.hpp"
#include "nic/sdk/platform/capri/capri_qos.hpp"
#include "nic/apollo/framework/impl_base.hpp"

// default network control traffic dscp value (CS6)
#define QOS_CONTROL_DEFAULT_DSCP 63 

namespace api {
namespace impl {

#define QOS_GROUPS(ENTRY)                                       \
    ENTRY(QOS_GROUP_DEFAULT,             0, "default")          \
    ENTRY(QOS_GROUP_CONTROL,             1, "control")          \
    ENTRY(QOS_GROUP_SPAN,                2, "span")             \
    ENTRY(QOS_GROUP_CPU_COPY,            3, "cpu-copy")         \
    ENTRY(NUM_QOS_GROUPS,                4, "num-qos-groups")

SDK_DEFINE_ENUM(qos_group_t, QOS_GROUPS)
#undef QOS_GROUPS

// pi struct
typedef struct qos_class_s {
    qos_group_t qos_group;  ///< qos class unique identifier
    qos_cmap_t cmap;        ///< classfication map - PCP/DSCP
    bool no_drop;           ///< qos class is no_drop or not
    uint32_t mtu;           ///< max packet size
} __PACK__ qos_class_t;

/// \defgroup PDS_QOS_IMPL - qos functionality
/// \ingroup PDS_QOS
/// @{

/// \brief qos implementation
class qos_impl : public impl_base {
public:
    // \brief  initialize and create qos classes
    //         TODO temporary method until the objects
    //         are created from agent
    // \return #SDK_RET_OK on success, failures status code on error
    static sdk_ret_t qos_init(void);

private:
    /// \brief constructor
    qos_impl() {}

    /// \brief destructor
    ~qos_impl() {}

    // \brief     return dot1q_pcp based on qos group
    // \param[in] qos class representing the qos group
    // \return    dot1q_pcp value
    static uint8_t qos_class_group_get_dot1q_pcp_(qos_class_t *qos_class);

    // \brief     program uplink params for traffic manager
    // \param[in] tm_uplink_q_params_t uplink queue params
    // \return    #SDK_RET_OK on success, failure status code on errro
    static sdk_ret_t qos_program_tm_uplink_params_(
                         tm_uplink_q_params_t *q_params);

    // \brief      allocate queue count based on qos class or group
    // \param[in]  qos_group    qos group representing qos class
    // \param[in]  no_drop      qos class is drop or no-drop
    // \param[out] alloc_params allocated queue counts
    // \return     #SDK_RET_OK on success, failure status code on error
    static sdk_ret_t qos_get_alloc_q_count_(qos_group_t qos_group, bool no_drop,
                                            qos_q_alloc_params_t *alloc_params);

    // \brief      allocate queue index for qos class or group
    // \param[in]  qos_group qos group representing qos class
    // \param[in]  no_drop   qos class is drop or no-drop
    // \param[out] qos_queue allocated queue indices
    // \return     #SDK_RET_OK on success, failure status code on error
    static sdk_ret_t qos_class_alloc_queues_(qos_group_t qos_group,
                                             bool no_drop,
                                             qos_queues_t *qos_queue);

    // \brief       populates queue params based on qos_class and
    //              allocated queues
    // \param[in]   qos_class qos class structure
    // \param[in]   qos_queues allocated queue indices
    // \params[out] q_params queue params to be programmed by HW
    // \return      #SDK_RET_OK on success, failure status code on error
    static sdk_ret_t qos_populate_q_params_(tm_uplink_q_params_t *q_params,
                                            qos_class_t *qos_class,
                                            qos_queues_t *qos_queues);
    // \brief     create a qos class
    // \param[in] qos_class qos class structure
    // \return    #SDK_RET_OK on success, failure status code on error
    static sdk_ret_t qos_class_create_(qos_class_t *qos_class);
};

}    // namespace impl
}    // namespace api
