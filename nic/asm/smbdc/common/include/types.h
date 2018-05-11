#ifndef __TYPES_H
#define __TYPES_H

#define CB_UNIT_SIZE_BYTES 64
#define SQCB_ADDR_SHIFT    9
#define RQCB_ADDR_SHIFT    9

#define SQCB0_ADDR_GET(_r) \
    sll     _r, k.{phv_global_common_cb_addr_sbit0_ebit1...phv_global_common_cb_addr_sbit18_ebit24}, SQCB_ADDR_SHIFT;

#define SQCB1_ADDR_GET(_r) \
    add     _r, CB_UNIT_SIZE_BYTES, k.{phv_global_common_cb_addr_sbit0_ebit1...phv_global_common_cb_addr_sbit18_ebit24}, SQCB_ADDR_SHIFT;

#define SQCB2_ADDR_GET(_r) \
    add     _r, (2 * CB_UNIT_SIZE_BYTES), k.{phv_global_common_cb_addr_sbit0_ebit1...phv_global_common_cb_addr_sbit18_ebit24}, SQCB_ADDR_SHIFT;

#define SQCB3_ADDR_GET(_r) \
    add     _r, (3 * CB_UNIT_SIZE_BYTES), k.{phv_global_common_cb_addr_sbit0_ebit1...phv_global_common_cb_addr_sbit18_ebit24}, SQCB_ADDR_SHIFT;

//RQCB organization
//0-256B RQCB0..RQCB3
//256-320B RESP_RX_STATS
//320-376B RESP_TX_STATS
#define RQCB0_ADDR_GET(_r) \
    sll     _r, k.{phv_global_common_cb_addr_sbit0_ebit1...phv_global_common_cb_addr_sbit18_ebit24}, RQCB_ADDR_SHIFT;

#define RQCB1_ADDR_GET(_r) \
    add     _r, CB_UNIT_SIZE_BYTES, k.{phv_global_common_cb_addr_sbit0_ebit1...phv_global_common_cb_addr_sbit18_ebit24}, RQCB_ADDR_SHIFT;

#define RQCB2_ADDR_GET(_r) \
    add     _r, (2 * CB_UNIT_SIZE_BYTES), k.{phv_global_common_cb_addr_sbit0_ebit1...phv_global_common_cb_addr_sbit18_ebit24}, RQCB_ADDR_SHIFT;

    
#define ARE_ALL_FLAGS_SET(_c, _flags_r, _flags_test) \
    smeqh   _c, _flags_r, _flags_test, _flags_test
#define IS_ANY_FLAG_SET(_c, _flags_r, _flags_test) \
    smneh   _c, _flags_r, _flags_test, 0
#define IS_FLAG_NOT_SET(_c, _flags_r, _flags_test) \
    smeqh   _c, _flags_r, _flags_test, 0
#define IS_FLAG_NOT_SET_C(_c1, _flags_r, _flags_test, _c2) \
    smeqh._c2   _c1, _flags_r, _flags_test, 0

#define ARE_ALL_FLAGS_SET_B(_c, _flags, _flags_test) \
    smeqb   _c, _flags, _flags_test, _flags_test
#define IS_ANY_FLAG_SET_B(_c, _flags, _flags_test) \
    smneb   _c, _flags, _flags_test, 0

#define IS_MASKED_VAL_EQUAL(_c, _cmp_r, _mask, _val) \
    smeqh   _c, _cmp_r, _mask, _val

#define IS_MASKED_VAL_EQUAL_B(_c, _cmp_r, _mask, _val) \
    smeqb   _c, _cmp_r, _mask, _val



//smbdc_sq_wqe format

struct smbdc_sg_list_t {
    base_addr: 64;
    num_pages: 16;
};


#define SMBDC_MR_DIR_READ  0
#define SMBDC_MR_DIR_WRITE 1

//this is the wqe_base (16 Bytes), 
//followed by num_sg_lists number of smbdc_sg_list_t structs
//Each sg_list is a contiguous list of pages and maps to one RDMA MR
//@begin_offset: first page of first sg-list can have an offset to start at
//#end_offset  : last page of last sg-list can have an offset to stop at
struct smbdc_sqwqe_mr_t {
    wrid:64;
    direction: 1; //write or read
    need_invalidate: 1; 
    pad: 14;
    num_sg_lists: 8;
    begin_offset: 16;
    end_offset: 16;
    //... followed by sg list elements
    struct smbdc_sg_list_t sg0;
    struct smbdc_sg_list_t sg1;
    struct smbdc_sg_list_t sg2;
    struct smbdc_sg_list_t sg3;
    pad2: 64;
};

#define SMBDC_NUM_SGES_PER_CACHELINE 4

struct smbdc_sge_t {
    va: 64;
    len: 32;
    pad: 32;
};

struct smbdc_sqwqe_send_t {
    wrid: 64;
    total_len: 32;
    pad: 16;
    num_sges: 8;
    
    struct smbdc_sge_t sg0;   
    struct smbdc_sge_t sg1;   
    struct smbdc_sge_t sg2;   
};

#define SMBDC_SQWQE_OP_TYPE_NONE 0
#define SMBDC_SQWQE_OP_TYPE_MR   1
#define SMBDC_SQWQE_OP_TYPE_SEND 2


struct smbdc_sqwqe_t {
    type: 8;
    union {
        struct smbdc_sqwqe_mr_t mr;
        struct smbdc_sqwqe_send_t send;
    };
};


struct smbdc_sqsge_t {
    struct smbdc_sge_t sge0;
    struct smbdc_sge_t sge1;
    struct smbdc_sge_t sge2;
    struct smbdc_sge_t sge3;
};

//smbdc_cqe format

#define SMBDC_CQ_STATUS_SUCCESS 0
#define SMBDC_CQ_STATUS_FAILURE 1

struct smbdc_cqe_t {
    wrid:64;
    status:8;
    mr_id1:24;
    mr_id2:24;
    mr_id3:24;
    mr_id4:24;
    color:1;
    pad:87;
};




#endif //__TYPES_H
