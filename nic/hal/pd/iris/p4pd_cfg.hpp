// {C} Copyright 2019 Pensando Systems Inc. All rights reserved

#ifndef __P4PD_CFG_HPP__
#define __P4PD_CFG_HPP__

//----------------------------------------------------------------------------
// initing p4pd cfg attributes
//----------------------------------------------------------------------------
static inline sdk_ret_t
pipeline_cfg_init (p4pd_cfg_t *p4pd_cfg, p4pd_cfg_t *p4pd_rxdma_cfg,
                   p4pd_cfg_t *p4pd_txdma_cfg)
 {
     if (p4pd_cfg) {
         p4pd_cfg->table_map_cfg_file  = "iris/p4_table_map.json";
         p4pd_cfg->p4pd_pgm_name       = "iris";
         p4pd_cfg->p4pd_rxdma_pgm_name = "p4plus";
         p4pd_cfg->p4pd_txdma_pgm_name = "p4plus";
         p4pd_cfg->cfg_path = std::getenv("HAL_CONFIG_PATH");
     }

     if (p4pd_rxdma_cfg) {
         p4pd_rxdma_cfg->table_map_cfg_file  = "iris/p4_rxdma_table_map.json";
         p4pd_rxdma_cfg->p4pd_pgm_name       = "iris_p4";
         p4pd_rxdma_cfg->p4pd_rxdma_pgm_name = "iris_rxdma";
         p4pd_rxdma_cfg->p4pd_txdma_pgm_name = "iris_txdma";
         p4pd_rxdma_cfg->cfg_path = std::getenv("HAL_CONFIG_PATH");
     }

     if (p4pd_txdma_cfg) {
         p4pd_txdma_cfg->table_map_cfg_file  = "iris/p4_txdma_table_map.json";
         p4pd_txdma_cfg->p4pd_pgm_name       = "iris_p4";
         p4pd_txdma_cfg->p4pd_rxdma_pgm_name = "iris_rxdma";
         p4pd_txdma_cfg->p4pd_txdma_pgm_name = "iris_txdma";
         p4pd_txdma_cfg->cfg_path = std::getenv("HAL_CONFIG_PATH");
     }

     return SDK_RET_OK;
 }

#endif    // __P4PD_CFG_HPP__

