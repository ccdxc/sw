#ifndef __FSM__
#define __FSM__

#include "nic/include/base.h"
#include <string.h>

using std::string;

namespace hal {
namespace utils {

typedef uint64_t* fsm_state_ctx;
typedef uint64_t* fsm_event_data;
typedef uint64_t* fsm_state_timer_ctx;

typedef std::function<bool(fsm_state_ctx, fsm_event_data data)>
    fsm_transition_func;
typedef std::function<void(fsm_state_ctx)> fsm_state_func;
typedef std::function<uint32_t(fsm_state_ctx)> fsm_timeout_func;

class fsm_state_t {

    string name_;

   public:
    uint32_t state_id;
    uint32_t timeout;
    fsm_state_func entry_func;
    fsm_state_func exit_func;

    fsm_state_t(uint32_t id, string name = "", uint32_t timeout = 0,
                fsm_state_func entry_func = NULL,
                fsm_state_func exit_func = NULL) {
        this->state_id = id;
        this->name_ = name;
        this->timeout = timeout;
        this->entry_func = entry_func;
        this->exit_func = exit_func;
    }

    bool operator<(const fsm_state_t& s) const {
        if (state_id < s.state_id)
            return 1;
        else
            return 0;
  }
  static void state_entry(fsm_state_ctx ctx) {}

  static void state_exit(fsm_state_ctx ctx) {}
};

class fsm_transition_t {
public:
  uint32_t event;
  fsm_transition_func func;
  uint32_t next_state;

  fsm_transition_t(uint32_t event, fsm_transition_func function,
                   uint32_t next_state);
};

typedef std::map<fsm_state_t, std::vector<fsm_transition_t>>
    fsm_state_machine_def_t;

class fsm_state_machine_t;//forward declaration.
class fsm_timer_t {
    uint32_t timer_id_;

   public:
    fsm_timer_t(uint32_t id) { this->timer_id_ = id; }
    uint32_t get_timer_id() { return this->timer_id_; }
    virtual fsm_state_timer_ctx add_timer(uint64_t timeout,
                                          fsm_state_machine_t* ctx,
                                          bool periodic = false) = 0;
    virtual void delete_timer(fsm_state_timer_ctx) = 0;

    virtual ~fsm_timer_t(){};
};

typedef std::function<fsm_state_machine_def_t*()> get_sm_func;
typedef std::function<fsm_timer_t*()> get_timer_func;

class fsm_state_machine_t {
    get_sm_func sm_get_func_;
    uint32_t current_state_;
    uint32_t init_state_;
    uint32_t end_state_;
    uint32_t timeout_;
    uint32_t timeout_event_;
    fsm_state_timer_ctx cur_state_time_ctx_;
    get_timer_func timer_get_func_;
    fsm_state_ctx ctx_;
    std::tuple<uint32_t, fsm_event_data> next_event_;

    void _reset_next_event() {
        this->next_event_ = std::make_tuple(UINT32_MAX, nullptr);
  }

  bool _next_event_set() {
      return std::get<0>(this->next_event_) != UINT32_MAX;
  }

  void _set_next_event(uint32_t event, fsm_event_data data) {
      this->next_event_ = std::make_tuple(event, data);
  }

  void _process_event_internal(uint32_t event, fsm_event_data data);

 public:
  fsm_state_machine_t(get_sm_func sm_func, uint32_t init_state,
                    uint32_t end_state, uint32_t timeout_event,
                    fsm_state_ctx ctx = NULL,
                    get_timer_func timer_func = NULL);
  uint32_t get_state() { return this->current_state_; }
  void set_state(uint32_t state) { this->current_state_ = state; }
  fsm_state_machine_def_t* get_def() { return this->sm_get_func_(); }
  void process_event(uint32_t event, fsm_event_data data);
  void stop_state_timer();
  void reset_timer();
  // Ability to set dynamic timeout, however this should be called only during
  // entry function of the state.
  void set_current_state_timeout(uint32_t);
  uint32_t get_current_state_timeout();
  uint32_t get_timeout_event();
  void throw_event(uint32_t event, fsm_event_data data);
  bool state_machine_competed() {
      return this->current_state_ == this->end_state_;
 }
 fsm_state_ctx get_ctx() { return this->ctx_; }
};

#define FSM_TRANSITION(event, function, next_state) \
    fsm_transition_t(event, function, next_state),

#define FSM_STATE_BEGIN(state_id, timer, entry_func, exit_func) \
    {                                                           \
        fsm_state_t(state_id, #state_id, timer, entry_func, exit_func), {
#define FSM_STATE_END \
  }                   \
  }                   \
  ,

#define SM_BIND_NON_STATIC(class_name, func) \
    std::bind(&class_name::func, this, _1, _2)

#define SM_BIND_NON_STATIC_ARGS_1(class_name, func) \
    std::bind(&class_name::func, this, _1)

#define FSM_SM_BEGIN(sm_name) fsm_state_machine_def_t sm_name = {
#define FSM_SM_END \
  }                \
  ;

}  // namespace utils
}  // namespace hal
#endif
