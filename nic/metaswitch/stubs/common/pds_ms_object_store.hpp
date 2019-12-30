//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// PDS-MS object-store base
//--------------------------------------------------------------

#ifndef __PDS_MS_OBJ_STORE_HPP__
#define __PDS_MS_OBJ_STORE_HPP__

#include <functional>
#include <unordered_map>
#include <memory>
#include <string>

namespace pds_ms {

//------------------------------------------------------------------------------------
// Base class template for all PDS-MS object stores.
// Stores with KEY that is not a built-in type need to provide a Hash function object
//-------------------------------------------------------------------------------------
template <typename KEY, typename OBJECT, typename HASH = std::hash<KEY>> 
class obj_store_t {
public:
    // Object allocated from slab elsewhere
    // Just copy the pointer into the store
    void add_upd(const KEY& key, OBJECT* obj) {
        store_[key] = std::unique_ptr<OBJECT>(obj);
    }
    void add_upd(const KEY& key, std::unique_ptr<OBJECT>&& obj) {
        store_[key] = std::move(obj);
    }
    void erase(const KEY& key) {
        store_.erase(key);
    }

    OBJECT* get(const KEY& k) {
        auto it = store_.find(k);
        if (it == store_.end()) {return nullptr;}
        return it->second.get();
    };

    bool get_copy(const KEY& k, OBJECT* copy_out) {
        auto it = store_.find(k);
        if (it == store_.end()) {return false;}
        *copy_out = *(it->second);
        return true;
    };

    // Takes a function or lambda callback as argument. 
    // Return false from callback to abort walk
    void walk(const std::function<bool(const KEY&, OBJECT&)>& cb_fn) {
        for (auto& pair: store_) {
            if (!cb_fn (pair.first, *pair.second.get())) {break;}
        }
    } 
private:
    std::unordered_map<KEY, std::unique_ptr<OBJECT>, HASH> store_;
};

class state_t;

class base_obj_t {
public:
    virtual ~base_obj_t(void) {};
    virtual void update_store(state_t* state, bool op_delete) = 0;
    virtual void print_debug_str(void) = 0;
};

}

#endif
