


struct d_struct {
    out_desc_pindex : 8;
};

struct k_struct {
    out_desc_semaphore : ADDRESS_WIDTH;
}
struct p_struct p       ;
struct k_struct k       ;
struct d_struct d       ;

allocate_output_desc_semaphore:
    phvwr p.out_desc_pindex, d.out_desc_pindex
    // calculate raw-table address based on pindex 
    add r1, r0, d.out_desc_index
    sll r1, r1, PINDEX_SIZE_SHIFT
    addi r1, r1, OUT_DESC_PINDEX_BASE_ADDR
    phvwri p.table1_valid, 1
    phvwri p.table1_addr, r1 
    phvwri p.table1_pc, allocate_output_desc_addr
    phvwri p.table1_size, RAW_TABLE_DEFAULT_SIZE
    phvwri p.table1_rsvd, 0




struct d_struct {
    out_desc_addr : ADDRESS_WIDTH;
};

struct k_struct {
    out_desc_pindex : 8;
}

struct p_struct p       ;
struct k_struct k       ;
struct d_struct d       ;

allocate_output_desc_addr:
    add r2, r0, k.out_desc_pindex 
    mincr r2, 1, NIC_DESC_TABLE_SIZE_SHIFT // ???
    sll r2, r2, NIC_DESC_ENTRY_SIZE_SHIFT // sizeof(p4plus_ipsec_internal_t)+sizeof(nic_desc_entry_t) 
    addi r3, r0, RNMDR_BASE_ADDR
    add  r3, r2, r3  // add base+(out_desc_pindex*descriptor_struct) to get current desc addr.
    phvwr p.out_desc_addr, r3
out_desc_load_desc_contents:
    add r1, r0, d.out_desc_addr
    phvwr p.table_size0, 8
    phvwri p.table1_valid, 1
    phvwri p.table1_addr, r1 
    phvwri p.table1_pc, read_output_desc_contents
    phvwri p.table1_size, RAW_TABLE_DEFAULT_SIZE
    phvwri p.table1_rsvd, 0
