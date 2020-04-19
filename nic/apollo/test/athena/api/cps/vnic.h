/*
 * {C} Copyright 2020 Pensando Systems Inc. All rights reserved
 *
 * athena vnic test implementation
 *
 */

#include "pds_vnic.h"

#define IDX_TO_VLAN(id) id+1
#define IDX_TO_MPLS_LABEL(id) id+1
#define IDX_TO_VNIC(id) id%512
#define IDX_TO_VNIC1(id) (id+1)%512

#define VLAN_2_VNIC_VALIDATE_RET(id, ret)  \
    { \
        if (id > 0 && id < 4095) { \
            assert(ret == PDS_RET_OK); \
        } else { \
            assert(ret != PDS_RET_OK); \
        } \
    }

#define VLAN_2_VNIC_VALIDATE_READ_INFO(id, info) \
    {\
        if (id > 0 && id < 4095) {\
            assert(info.spec.data.vnic_type == VNIC_TYPE_L3);\
            assert((info.spec.data.vnic_id == id%512) || \
                    (info.spec.data.vnic_id == (id+1)%512));\
        }\
    }

#define IDX_TO_MPLS_LABEL(id) id+1
#define MPLS_LABEL_2_VNIC_VALIDATE_RET(id, ret)  \
    { \
        if (id > 0 && id < 1048576) { \
            assert(ret == PDS_RET_OK); \
        } else { \
            assert(ret != PDS_RET_OK); \
        } \
    }

#define MPLS_LABEL_2_VNIC_VALIDATE_READ_INFO(id, info) \
    {\
        if (id >= 0 && id < 1048576 ) { \
            assert(info.spec.data.vnic_type == VNIC_TYPE_L3);\
            assert((info.spec.data.vnic_id == id%512) || \
                    (info.spec.data.vnic_id == (id+1)%512));\
        }\
    }

static inline void
test_pds_vlan_to_vnic_map_create (uint32_t idx)
{

    pds_ret_t                   ret = PDS_RET_OK;
    pds_vlan_to_vnic_map_spec_t spec = {0} ;

    spec.key.vlan_id = IDX_TO_VLAN(idx);
    spec.data.vnic_type = VNIC_TYPE_L3 ;
    spec.data.vnic_id = IDX_TO_VNIC(idx);

    ret = pds_vlan_to_vnic_map_create(&spec); 

    VLAN_2_VNIC_VALIDATE_RET(idx, ret);
}

static inline void
test_pds_vlan_to_vnic_map_update (uint32_t idx)
{

    pds_ret_t                   ret = PDS_RET_OK;
    pds_vlan_to_vnic_map_spec_t spec = {0} ;

    spec.key.vlan_id = IDX_TO_VLAN(idx);
    spec.data.vnic_type =  VNIC_TYPE_L3;
    spec.data.vnic_id = IDX_TO_VNIC1(idx);

    ret = pds_vlan_to_vnic_map_create(&spec); 

    VLAN_2_VNIC_VALIDATE_RET(idx, ret);
}

static inline void
test_pds_vlan_to_vnic_map_read (uint32_t idx)
{
    pds_ret_t                   ret = PDS_RET_OK;
    pds_vlan_to_vnic_map_key_t  key = {0};
    pds_vlan_to_vnic_map_info_t info = {0};

    key.vlan_id = IDX_TO_VLAN(idx);

    ret = pds_vlan_to_vnic_map_read(&key, &info); 

    VLAN_2_VNIC_VALIDATE_RET(idx, ret);

    VLAN_2_VNIC_VALIDATE_READ_INFO(idx, info);
}

static inline void
test_pds_vlan_to_vnic_map_delete (uint32_t idx)
{
    pds_ret_t                   ret = PDS_RET_OK;
    pds_vlan_to_vnic_map_key_t  key = {0} ;

    key.vlan_id = IDX_TO_VLAN(idx);

    ret = pds_vlan_to_vnic_map_delete(&key); 

    VLAN_2_VNIC_VALIDATE_RET(idx, ret);
}

static inline void
test_pds_mpls_label_to_vnic_map_create (uint32_t idx)
{

    pds_ret_t                   ret = PDS_RET_OK;
    pds_mpls_label_to_vnic_map_spec_t spec = {0} ;

    spec.key.mpls_label = IDX_TO_MPLS_LABEL(idx);
    spec.data.vnic_type = VNIC_TYPE_L3 ;
    spec.data.vnic_id = IDX_TO_VNIC(idx);

    ret = pds_mpls_label_to_vnic_map_create(&spec); 

    MPLS_LABEL_2_VNIC_VALIDATE_RET(idx, ret);
}

static inline void
test_pds_mpls_label_to_vnic_map_update (uint32_t idx)
{

    pds_ret_t                   ret = PDS_RET_OK;
    pds_mpls_label_to_vnic_map_spec_t spec = {0} ;

    spec.key.mpls_label = IDX_TO_MPLS_LABEL(idx);
    spec.data.vnic_type = VNIC_TYPE_L3;
    spec.data.vnic_id = IDX_TO_VNIC1(idx);

    ret = pds_mpls_label_to_vnic_map_create(&spec); 

    MPLS_LABEL_2_VNIC_VALIDATE_RET(idx, ret);
}

static inline void
test_pds_mpls_label_to_vnic_map_read (uint32_t idx)
{
    pds_ret_t                   ret = PDS_RET_OK;
    pds_mpls_label_to_vnic_map_key_t  key = {0};
    pds_mpls_label_to_vnic_map_info_t info = {0};

    key.mpls_label = IDX_TO_MPLS_LABEL(idx);

    ret = pds_mpls_label_to_vnic_map_read(&key, &info); 

    MPLS_LABEL_2_VNIC_VALIDATE_RET(idx, ret);

    MPLS_LABEL_2_VNIC_VALIDATE_READ_INFO(idx, info);
}

static inline void
test_pds_mpls_label_to_vnic_map_delete (uint32_t idx)
{
    pds_ret_t                      ret = PDS_RET_OK;
    pds_mpls_label_to_vnic_map_key_t  key = {0} ;

    key.mpls_label = IDX_TO_MPLS_LABEL(idx);

    ret = pds_mpls_label_to_vnic_map_delete(&key); 

    MPLS_LABEL_2_VNIC_VALIDATE_RET(idx, ret);
}
