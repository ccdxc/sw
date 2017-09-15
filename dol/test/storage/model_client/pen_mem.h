#ifndef _PEN_MEM_H_
#define _PEN_MEM_H_
#ifndef SKIP_JUDY
#include <Judy.h>
#else
//#include <vector>
#include <unordered_map>
#endif

#include <assert.h>
#include <iostream>
#include "pknobs.h"

#ifndef NO_THREADS
#include <mutex>
#endif

#ifdef TESTING
#include <benchmark/benchmark_api.h>
#include <map>
#include <vector>
#endif
#include "pen_mem_base.h"

template<unsigned int page_sz_bits = 12>
class pen_mem : public pen_mem_base {
    typedef unsigned long long addr_t;
#ifndef SKIP_JUDY
    Pvoid_t PJLArray = (Pvoid_t) NULL;        // initialize JudyL
                                              // array
    Pvoid_t dirty_bits = (Pvoid_t) NULL;
#else
//  typedef std::pair<addr_t, unsigned char *> mem_db_elem_t;

//  std::vector<mem_db_elem_t> memdb;
    std::unordered_map<addr_t, unsigned char *> memdb;
#endif
    addr_t last_page;
    const unsigned int page_mask;
    const unsigned int page_size;
    std::mutex my_mutex;
    unsigned char * last_page_ptr = nullptr;
    std::string name;
    pknobs::RRKnob addr_knob;
    pknobs::RKnob  data_knob;
    addr_t min;
    addr_t max;
public:
    pen_mem(const char * n, addr_t _min = 0, addr_t _max = (addr_t) (-1ll)) :
        page_mask((1u << page_sz_bits) - 1),
        page_size((1u << page_sz_bits)),
        name(n),
        addr_knob(name+"_aknob",_min, _max),
        data_knob(name+"_dknob"),
        min(_min),
        max(_max)
    {

    }

    bool range_check(addr_t addr) {
        return addr >= min && addr <= max;
    }

    // This returns a new address that has not been allocated.
    addr_t alloc_page() {
        unsigned char * ptr = nullptr;
        int count = 1000;
        do {
            addr_t addr = (addr_knob.eval() & ~page_mask);
            addr_t page_num = get_page_num(addr);
            ptr = get_page_ptr(page_num, false);
            if (ptr == nullptr) {
                ptr = get_page_ptr(addr, true);
                if (ptr == nullptr) {
                    throw std::exception();
                }
                return addr;
            }
            count--;
        } while (ptr == nullptr && count > 0);
        std::cerr << "ERROR: Failed to allocate address" << std::endl;
        return 0;
    }

    bool dealloc_page(addr_t addr) {
        assert(addr&page_mask == 0);
        addr_t page_num = get_page_num(addr);
        void * p = nullptr;
        unsigned char * ptr = nullptr;
#ifndef NO_THREADS
        std::lock_guard<std::mutex> l(my_mutex);
#endif
#ifndef SKIP_JUDY
        // Judy long get
        JLG(p, PJLArray, page_num);
#else
        // auto cmp = [](const mem_db_elem_t &a, const mem_db_elem_t &b) -> bool { return a.first < b.first; };
        // auto it = std::lower_bound(memdb.begin(), memdb.end(), mem_db_elem_t(page_num, 0), cmp);
        auto it = memdb.find(page_num);
        // if (it != memdb.end() && it->first == page_num ) {
        if (it != memdb.end()) {
            p = &(it->second);
        }
#endif
        if (p == nullptr) {
            std::cout << "Could not find the address to dealloc" << std::endl;
            return false;
        }
        ptr = *reinterpret_cast<unsigned char **>(p);
        delete [] ptr;
#ifndef SKIP_JUDY
        Word_t rc;
        JLD(rc,PJLArray, page_num);
#else
        memdb.erase(it);
#endif
        return true;
    }

    inline addr_t get_page_num(addr_t address) const {
        return (address>>page_sz_bits);
    }

    inline addr_t get_page_offset(addr_t address) const {
        return (address & page_mask);
    }

    unsigned int get_max_block_size(addr_t addr, unsigned int size) const {
        if (((addr+size)>>page_sz_bits) == (addr>>page_sz_bits)) {
            return size;
        } else {
            return page_size - get_page_offset(addr);
        }
    }

    unsigned char * get_block(addr_t addr, unsigned int block_sz) {
        addr_t page_num = get_page_num(addr);
        addr_t offset   = get_page_offset(addr);
        assert (page_num == get_page_num(addr+block_sz-1)); // Ensure that the page block is aligned in page.
        assert(range_check(addr));
        unsigned char * ptr = get_page_ptr(page_num, true);
        return ptr+offset;
    }
    unsigned char * get_page_ptr(addr_t page_num, bool insert = false) {
        void * p = nullptr;
        unsigned char *ptr;
#ifndef NO_THREADS
        std::lock_guard<std::mutex> l(my_mutex);
#endif
        if (last_page_ptr && last_page == page_num) {
            ptr = last_page_ptr;
        } else {
#ifndef SKIP_JUDY
            // Judy long get
            JLG(p, PJLArray, page_num);
#else
//      auto cmp = [](const mem_db_elem_t &a, const mem_db_elem_t &b) -> bool { return a.first < b.first; };
//      auto it = std::lower_bound(memdb.begin(), memdb.end(), mem_db_elem_t(page_num, 0), cmp);
            auto it = memdb.find(page_num);
//      if (it != memdb.end() && it->first == page_num ) {
            if (it != memdb.end()) {
                p = &(it->second);
            }
#endif
            if (p == nullptr && !insert) {
                return nullptr;
            } else if (p == nullptr && insert) {
                ptr = new unsigned char[page_size];
                memset(ptr, 0, page_size); ////added this to avoid valgrind complaining, by Changqi
#ifndef SKIP_JUDY
                // Judy long initialize
                JLI(p, PJLArray, page_num);
                *reinterpret_cast<unsigned char **>(p) = ptr;
#else
//	memdb.emplace(it, page_num, ptr);
                memdb[page_num] = ptr;
#endif
            }	else {
                ptr = *reinterpret_cast<unsigned char **>(p);
            }
            last_page = page_num;
            last_page_ptr = ptr;
        }
        return ptr;
    }


    template <typename T>
    T get(addr_t addr) {
        assert(range_check(addr));
        addr_t page_num = get_page_num(addr);
        unsigned char * ptr = get_page_ptr(page_num, false);
        if (ptr) {
            addr_t offset = ~((addr_t)(sizeof(T)) - 1) & get_page_offset(addr);
            return *reinterpret_cast<T *>(ptr+offset);
        } else {
            std::cout << "WARN: Address 0x" << std::hex << addr << std::dec << " is not in memory"  << std::endl;
            return T(0xdeadbeef);
        }
    }

    void set_dirty_bits(addr_t start_addr, uint32_t len)
    {
        for (uint32_t i = 0; i < (len+3)/4; i++) {
            int rc;
            J1S(rc, dirty_bits, (start_addr>>2)+i);
        }
    }



    // TODO: Add unaligned accesses
    template <typename T>
    void set(addr_t addr, T v, bool alloc = false) {
        assert(range_check(addr));
        addr_t page_num = get_page_num(addr);
        unsigned char * ptr = get_page_ptr(page_num, alloc); // Always
        addr_t offset = ~((addr_t)(sizeof(T)) - 1) & get_page_offset(addr);
        if (ptr) {
            *reinterpret_cast<T *>(ptr+offset) = v;
        } else {
            std::cout << "WARN: Address 0x" << std::hex << addr << std::dec << " is not in memory" << std::endl;
        }
        set_dirty_bits(addr, sizeof(T));
    }

    virtual bool burst_read(uint64_t addr, unsigned char * data, unsigned int len, bool secure = false, bool reverse_byte_order = false) {
        unsigned char * p = data;
        addr_t curr_page_num = get_page_num(addr);
        unsigned int curr_len = get_max_block_size(addr, len);
        unsigned int offset = get_page_offset(addr);
        bool retVal = true;
        while (len) {
            addr_t page_num = get_page_num(addr);
            unsigned char * ptr = get_page_ptr(page_num, false);
	    if (ptr == nullptr) {
                retVal = false;
                auto stride = sizeof(decltype(data_knob.eval()));
                for (int i = curr_len; i > 0; i-=stride) {
                    auto wlen = stride > i ? i : stride;
                    auto tmp = data_knob.eval();
                    memcpy(p,&tmp, wlen);
                    p+= wlen;
                }
            } else {
	      memcpy(p, ptr+offset, curr_len);
	    }
            p += curr_len;
	    addr += curr_len;
            len -= curr_len;
            // smaller of remaining len or page_size
            curr_len = len < page_size ? len : page_size;
            offset = 0;
        }
        return retVal;
    }

    virtual bool burst_write(uint64_t addr, const unsigned char * data, unsigned int len, bool secure = false, bool reverse_byte_order = false) {
        const unsigned char * p = data;
        unsigned int curr_len = get_max_block_size(addr, len);
        unsigned int offset = get_page_offset(addr);
        unsigned int orig_len = len;
        while (len) {
            addr_t page_num = get_page_num(addr);
            unsigned char * ptr = get_page_ptr(page_num, true);
            if (ptr == nullptr)
                return false;
            memcpy(ptr+offset, p, curr_len);
            p += curr_len;
	    addr += curr_len;
            len -= curr_len;
            // smaller of remaining len or page_size
            curr_len = len < page_size ? len : page_size;
            offset = 0;
        }
        set_dirty_bits(addr, orig_len);
        return true;
    }

    ~pen_mem() {
#ifndef SKIP_JUDY
        void * p;
        Word_t index = 0;
        // Judy long first
        JLF(p, PJLArray, index);
        int rc;
        while (p) {
            auto ptr = *reinterpret_cast<unsigned char **>(p);
            delete [] ptr;
            // Judy long delete entry
            JLD(rc, PJLArray, index);
            // Judy long next
            JLN(p, PJLArray, index);
        }
        J1FA(rc, dirty_bits);
#else
        //auto del = [](mem_db_elem_t &c) -> void { delete [] c.second; };
        auto del = [](std::unordered_map<addr_t, unsigned char *>::value_type &c) -> void { delete [] c.second; };
        std::for_each(memdb.begin(),memdb.end(),del);
        memdb.clear();
#endif
    }

    class iterator: public std::iterator<
        std::input_iterator_tag,   // iterator_category
        addr_t,                 // value_type
        std::ptrdiff_t,            // difference_type
        addr_t *,                // pointer
        const addr_t &                 // reference
        >{
        int rc;
	Word_t index = 0;
	addr_t addr;
	pen_mem & m;
    public:
        explicit iterator(pen_mem &r, bool last = false)
	  : m(r) {
	  if (last) {
	    index = -1ll;
	    J1L(rc,m.dirty_bits, index);
	    index++;
	  } else {
	    J1F(rc, m.dirty_bits, index);
	  }
	  addr = (addr_t)(index)<<2;
	}
        iterator& operator++() {J1N(rc,m.dirty_bits, index); addr = (addr_t)(index)<<2; return *this;}
        iterator operator++(int) {iterator retval = *this; ++(*this); return retval;}
        bool operator==(iterator other) const {return  index == other.index;}
        bool operator!=(iterator other) const {return !(*this == other);}
        reference operator*() const {return addr; }
    };
    iterator begin() {return iterator(*this);}
    iterator end() {return iterator(*this,true);}
};


#ifdef TESTING
void BM_mem_access(benchmark::State& state) {
    pknobs::RRKnob a("foo",1,0x100000000ll);
    pen_mem<9> m("mymem");
    std::vector<unsigned long long> addrs;
    addrs.resize(50000);
    for (int i = 50000-1; i >= 0; --i) {
        addrs[i] = a.eval();
        m.set(addrs[i], a.eval());
    }
    int idx = 0;
    auto wrapGet = [&] () -> bool { m.get<unsigned long long>(addrs[idx]); idx++; if (idx>=50000) idx = 0; return true; };
    while (state.KeepRunning()) {
        benchmark::DoNotOptimize(wrapGet());
    }
}

BENCHMARK(BM_mem_access);
BENCHMARK_MAIN()
#endif
/*
int main (void) {
    std::map<Word_t, unsigned long long> db;
    pen_mem<9> m;
    pknobs::RRKnob a("foo",1,0x100000000ll);
    std::cout << std::hex ;
    for (int i = 0; i < 20000; i++) {
        auto addr = a.eval();
        auto val  = a.eval();
        std::cout << addr << " " << val << std::endl;
        m.set(addr, val);
        db[addr] = val;
    }
    for (auto it = db.begin(); it != db.end(); it++) {
        auto addr = it->first;
        auto val_e = it->second;
        auto val_r = m.get<decltype(a.eval())>(it->first);
        std::cout << std::hex ;
        std::cout << addr << " " << val_e << " " << val_r << std::endl;
        assert(val_e == val_r);
    }
}
*/

#endif
