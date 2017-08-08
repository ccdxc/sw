


struct d_struct {
    out_page_pindex : 8;
};

struct k_struct {
    out_page_semaphore : ADDRESS_WIDTH;
}
struct p_struct p       ;
struct k_struct k       ;
struct d_struct d       ;

out_page_alloc_index:
    phvwr p.out_page_pindex, d.out_page_pindex
    phvwri p.table3_valid, 1
    add r1, r0, d.out_page_index
    sll r1, r1, PINDEX_SIZE_SHIFT
    addi r1, r1, OUT_PAGE_PINDEX_BASE_ADDR 
    phvwri p.table3_addr, r1 
    phvwri p.table3_pc, out_page_addr_compute
    phvwri p.table3_size, RAW_TABLE_DEFAULT_SIZE
    phvwri p.table3_rsvd, 0

struct d_struct {
    out_page_addr : ADDRESS_WIDTH
};

struct k_struct {
    out_page_pindex : 8;
};

out_page_addr_compute:
    add r2, r0, k.out_page_pindex
    mincr r2, 1, NIC_PAGE_TABLE_SIZE_SHIFT // ???
    sll r2, r2, NIC_PAGE_ENTRY_SIZE_SHIFT
    addi r3, r0, RNMPR_BASE_ADDR
    add  r3, r2, r3
    phvwr p.out_page_addr, r3
    phvwri p.table3_addr, r3 
    phvwri p.table3_pc, out_page_read_contents
    phvwri p.table3_size, RAW_TABLE_DEFAULT_SIZE
    phvwri p.table3_rsvd, 0
