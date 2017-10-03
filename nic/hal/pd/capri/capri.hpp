#ifndef __CAPRI_HPP__
#define __CAPRI_HPP__

typedef struct capri_cfg_s {
    std::string      loader_info_file;
} capri_cfg_t;


hal_ret_t capri_init(capri_cfg_t *hal_cfg);
hal_ret_t capri_p4_asm_init();
hal_ret_t capri_p4p_asm_init();
hal_ret_t capri_p4_pgm_init();
hal_ret_t capri_hbm_regions_init();
hal_ret_t capri_repl_init();

#endif    // __CAPRI_HPP__
