//---------------------------------------------------------------
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
// PDSA object-store base
//--------------------------------------------------------------

#ifndef __PDSA_OBJ_STORE_HPP__
#define __PDSA_OBJ_STORE_HPP__

#include <functional>
#include <unordered_map>
#include <memory>

namespace pdsa_stub {

//------------------------------------------------------------------------------------
// Base class template for all PDSA object stores.
// Stores with KEY that is not a built-in type need to provide a Hash function object
//-------------------------------------------------------------------------------------
template <typename KEY, typename OBJECT, typename HASH = std::hash<KEY>> 
class obj_store_t {
public:
    void add_upd(const KEY& key, const OBJECT& obj) {
        store_.insert(std::make_pair(key, std::unique_ptr<OBJECT>(new OBJECT(obj))));
    }
    void add_upd(const KEY& key, OBJECT&& obj) {
        store_.insert(std::make_pair(key, std::unique_ptr<OBJECT>(new OBJECT(std::move(obj)))));
    }
    // Object allocated from slab elsewhere
    void add_upd(const KEY& key, std::unique_ptr<OBJECT>&& obj) {
        store_.insert(std::make_pair(key, std::unique_ptr<OBJECT>(std::move(obj))));
    }
    void erase(const KEY& key) {
        store_.erase(key);
    }

    OBJECT* get(const KEY& k) {
        auto it = store_.find(k);
        if (it = store_.end()) {return nullptr;}
        return it->second.get();
    };

    bool get_copy(const KEY& k, OBJECT* copy_out) {
        auto it = store_.find(k);
        if (it = store_.end()) {return false;}
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

}

#endif