#ifndef __CAPRI_STATE_HPP__
#define __CAPRI_STATE_HPP__

class capri_state_pd {
public:
    static capri_state_pd *factory(void);

    ~capri_state_pd();

    // get APIs for TXS scheduler related state
    hal::BMAllocator *txs_scheduler_map_idxr(void) { return txs_scheduler_map_idxr_; }

private:
    capri_state_pd();
    
    bool init(void);
    
    // TXS scheduler related state
    struct {
        hal::BMAllocator    *txs_scheduler_map_idxr_;
    } __PACK__;
};

#endif
