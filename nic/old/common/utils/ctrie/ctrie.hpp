#ifndef __CTRIE_HPP__
#define __CTRIE_HPP__

#include <boost/function.hpp>
#include <base.hpp>
#include <slab.hpp>

// no. of blocks per slab
#define CTRIE_DEFAULT_SLAB_SIZE              256

// TODO: thread_safe mode is not supported
class ctrie {
public:
    enum ctrie_ret_t {
        CTRIE_RET_OK,       // success
        CTRIE_RET_FAIL,     // catch-all failure
        CTRIE_RET_OOM,      // out-of-memory
        CTRIE_RET_EINVAL,   // invalid arguments
    };
    ctrie(uint16_t maxkey_len, uint16_t slab_block_size=CTRIE_DEFAULT_SLAB_SIZE, bool thread_safe=false);
    ~ctrie();
    ctrie_ret_t ctrie_add(uint8_t *key, uint16_t keylen, void *ctxt);
    void *ctrie_del(uint8_t *key, uint16_t keylen);
    void *ctrie_lookup(uint8_t *key, uint16_t keylen, bool exact_match=false);
    void ctrie_walk(boost::function<bool (uint8_t *key, uint16_t keylen, void *ctxt)> cb);

private:
    typedef struct ctnode_s {
        struct ctnode_s *left;
        struct ctnode_s *right;
        struct ctnode_s *parent;
        void            *ctxt;
        uint16_t        keylen;        // in bits
        uint8_t         key[0];        // key in big-endian byte order
    } ctnode_t;

    bool            thread_safe_;
    uint16_t        max_keylen_;       // in bits (multiple of 8)
    slab            *node_slab_;
    ctnode_t        *root_;

private:
    ctnode_t *create_ctnode_(uint8_t *key, uint16_t keylen, void *ctxt);
    bool ctrie_del_(ctnode_t *ctnode, uint8_t *key, uint16_t keylen, void **ctxt);
    bool is_bit_set_(uint8_t *bytes, uint16_t posn);
    void split_tree_(ctnode_t *node, uint16_t bposn,
                     uint8_t *key, uint16_t keylen, void *ctxt);
    bool equal_keys_(uint8_t *key1, uint16_t klen1, uint8_t *key2, uint16_t klen2);
    bool equal_keys_(uint8_t *key1, uint8_t *key2, uint16_t klen);
    bool ctrie_walk_(ctnode_t *node, boost::function<bool (uint8_t *key, uint16_t keylen, void *ctxt)> cb);
};

#endif    // __CTRIE_HPP__

