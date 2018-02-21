// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

#ifndef __CAPRI_STATE_HPP__
#define __CAPRI_STATE_HPP__

class capri_state_pd {
public:
    static capri_state_pd *factory(void);
    ~capri_state_pd();

    // get APIs for TXS scheduler related state
    hal::BMAllocator *txs_scheduler_map_idxr(void) { return txs_scheduler_map_idxr_; }
    void set_cfg_path(std::string cfg_path) { cfg_path_ = cfg_path; }
    std::string cfg_path(void) const { return cfg_path_; }

private:
    // TXS scheduler related state
    struct {
        hal::BMAllocator    *txs_scheduler_map_idxr_;
    } __PACK__;
    std::string    cfg_path_;    // HAL config path

private:
    capri_state_pd();
    bool init(void);
    
};

#endif    // __CAPRI_STATE_HPP__

