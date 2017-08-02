#ifndef __ACTION_HPP__
#define __ACTION_HPP__

#include <base.hpp>

// forward declaration
typedef struct pkt_meta_s pkt_meta_t;

//------------------------------------------------------------------------------
// possible return values from action handlers
//------------------------------------------------------------------------------
typedef enum act_ret_e {
    ACT_RET_OK,                   // proceed to the next stage
    ACT_RET_DROP,                 // drop packet without walking chain further
    ACT_RET_JUMP_TO_FAST_PATH,    // jump to forward action
    ACT_RET_SKIP_NEXT,            // skip next action
} act_ret_t;

//------------------------------------------------------------------------------
// action handler function type
//------------------------------------------------------------------------------
typedef act_ret_t (*action_func)(pkt_meta_t *pkt_meta, uint8_t *pkt);

//------------------------------------------------------------------------------
// global/common action initialization routine
//------------------------------------------------------------------------------
int action_init(action_func flow_lookup, action_func flow_setup,
                action_func fast_path);

//------------------------------------------------------------------------------
// top level packet action driver executed for every packet that is sent to
// compute cores
//------------------------------------------------------------------------------
int process_packet(uint8_t *pkt);

#endif    // __ACTION_HPP__

