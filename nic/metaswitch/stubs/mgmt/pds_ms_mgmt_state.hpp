//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// Mgmt stub global states
//--------------------------------------------------------------

#ifndef __PDS_MS_MGMT_STATE_HPP__
#define __PDS_MS_MGMT_STATE_HPP__

#include "nic/sdk/lib/logger/logger.hpp"
#include "nic/sdk/include/sdk/base.hpp"
#include "nic/sdk/lib/thread/thread.hpp"
#include "nic/metaswitch/stubs/common/pdsa_error.hpp"
#include <mutex>
#include <condition_variable>

namespace pds_ms {

// Singleton that holds all global state for the mgmt stub
// Placeholder class currently as we do not have any global
// mgmt stub state yet
class mgmt_state_t {
public:
    struct context_t {
        public:    
            context_t(std::mutex& m, mgmt_state_t* s) : l_(m), state_(s)  {};
            mgmt_state_t* state(void) {return state_;}
            void release(void) {l_.unlock(); state_ = nullptr;}
        private:    
            std::unique_lock<std::mutex> l_;
            mgmt_state_t* state_;
    };
    static void create(void) { 
        SDK_ASSERT (g_state_ == nullptr);
        g_state_ = new mgmt_state_t;
        g_response_ready_ = false;
    }
    static void destroy(void) {
        delete(g_state_); g_state_ = nullptr;
    }

    // Obtain a mutual exclusion context for thread safe access to the 
    // stub state. Automatic release when the context goes 
    // out of scope. Direct external access to Stub state without
    // a context is prohibited.
    // Calling this more than once from the same thread will deadlock.
    static context_t thread_context(void) {
        SDK_ASSERT(g_state_ != nullptr);
        return context_t(g_state_mtx_, g_state_);
    }
    
    // Blocking function call. To be used only from the grpc thread
    // context while waiting for the async response back from nbase
    static void ms_response_wait(void) {
        std::unique_lock<std::mutex> lock(g_cv_mtx_);
        g_cv_resp_.wait(lock, response_ready);
        g_response_ready_ = false;
    }
    
    // Called from the response method in the nbase thread context to
    // unblock the grpc thread
    static void ms_response_ready(void) {
        std::unique_lock<std::mutex> lock(g_cv_mtx_);
        g_response_ready_ = true;
        g_cv_resp_.notify_all();
    }

    void set_nbase_thread(sdk::lib::thread *tptr) {
        nbase_thread_ = tptr;
    }

    sdk::lib::thread *nbase_thread(void) {
        return nbase_thread_;
    }

private:
    static mgmt_state_t* g_state_;
    // Predicate to avoid spurious wake-up calls
    static bool g_response_ready_;
    static std::mutex g_cv_mtx_;
    static std::mutex g_state_mtx_;
    static std::condition_variable g_cv_resp_;

private:
    mgmt_state_t(void) {}
    static bool response_ready() {
        return g_response_ready_;
    }
    // Store the nbase thread ptr. Used to set thread ready upon nbase
    // init completion
    sdk::lib::thread *nbase_thread_;
};

// Function prototypes
bool  mgmt_state_init(void);
void  mgmt_state_destroy(void);

}

#endif    // __PDS_MS_MGMT_STATE_HPP__
