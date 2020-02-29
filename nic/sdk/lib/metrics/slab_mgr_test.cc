// {C} Copyright 2020 Pensando Systems Inc. All rights reserved.
// This file was forked from delphi

#include <unistd.h>
#include <ev++.h>
#include "gtest/gtest.h"
#include "slab_mgr.hpp"

namespace {
using namespace std;
using namespace sdk::metrics;

const int ktestMemSize = (NUM_SLAB_LAYERS  * SHM_PAGE_SIZE);

class SlabMgrTest : public testing::Test {
protected:
    SlabAllocatorUptr slab_mgr_;
    uint8_t           *mem_;
public:
    virtual void SetUp() {
        // allocate memory
        mem_ = (uint8_t *)malloc(ktestMemSize);

        // initialize slab mgr
        slab_mgr_ = unique_ptr<SlabAllocator>(new SlabAllocator((SlabPool_t *)mem_, mem_, 1));
        error err = slab_mgr_->InitMem(ktestMemSize);
        assert(err.IsOK());
    }
    virtual void TearDown() {
        // nothing to do??
    }
};

TEST_F(SlabMgrTest, BasicAllocTest) {
    // allocate some memory and free it
    uint8_t *ptr = slab_mgr_->Alloc(1);
    ASSERT_TRUE((ptr != NULL)) << "alloc failed";
    error err = slab_mgr_->Free(ptr);
    ASSERT_EQ(err, error::OK()) << "Error freeing memory";

    // try allocating bigger than allowed chunk size
    uint8_t *inv_ptr = slab_mgr_->Alloc(SHM_PAGE_SIZE);
    ASSERT_TRUE((inv_ptr == NULL)) << "alloc failed";

    // try doing double free
    err = slab_mgr_->Free(ptr);
    ASSERT_NE(err, error::OK()) << "double freeing suceeded";

    // try freeing invalid memory addresses
    err = slab_mgr_->Free(0);
    ASSERT_NE(err, error::OK()) << "freeing invalid address suceeded";
    err = slab_mgr_->Free(((uint8_t *)ptr) + ktestMemSize);
    ASSERT_NE(err, error::OK()) << "freeing invalid address suceeded";

    // try to exhaust all the chunks
    int num_pages  = (ktestMemSize / SHM_PAGE_SIZE) - 1;
    int max_chunks = CHUNKS_PER_PAGE(BASE_CHUNK_SIZE) * num_pages;
    uint8_t **ptrs = (uint8_t**)malloc(max_chunks * sizeof(ptr));
    for (int i = 0; i < max_chunks; i++) {
        ptrs[i] = slab_mgr_->Alloc(1);
        ASSERT_TRUE((ptrs[i] != NULL)) << "alloc failed";
    }
    slab_mgr_->Print();

    // try to allocate one more and it should fail
    ptr = slab_mgr_->Alloc(1);
    ASSERT_TRUE((ptr == NULL)) << "alloc suceeded when there are no chunks available";

    // free all the memory
    for (int i = 0; i < max_chunks; i++) {
        err = slab_mgr_->Free(ptrs[i]);
        ASSERT_EQ(err, error::OK()) << "free failed";
    }
    slab_mgr_->Print();

    // try freeing already freed memory and see what happens
    err = slab_mgr_->Free(ptrs[0]);
    ASSERT_NE(err, error::OK()) << "duplicate free suceeded";

    // try to allocate one objet from each chunk size
    int chunk_size;
    int idx;
    for (idx = 0; idx < (NUM_SLAB_LAYERS - 1); idx++) {
        chunk_size = (BASE_CHUNK_SIZE * (1 << idx));
        ptrs[idx] = slab_mgr_->Alloc(chunk_size);
        ASSERT_TRUE((ptrs[idx] != NULL)) << "alloc failed";
    }
    slab_mgr_->Print();

    // try to allocate one more and it should fail
    chunk_size = (BASE_CHUNK_SIZE * (1 << idx));
    ptr = slab_mgr_->Alloc(chunk_size);
    ASSERT_TRUE((ptr == NULL)) << "alloc suceeded when there are no chunks available";

    // free all the pointers
    for (idx = 0; idx < (NUM_SLAB_LAYERS - 1); idx++) {
        err = slab_mgr_->Free(ptrs[idx]);
        ASSERT_TRUE(err.IsOK()) << "free failed";
    }

    free(ptrs);
}

#define NUM_THREADS 5
#define NUM_ALLOCS_FREES (200 * 1000)

typedef struct concurrent_test_ctx_ {
    SlabAllocator *slab_mgr;
    int max_chunks;
    int alloc_count;
    int free_count;
    int my_id;
} concurrent_test_ctx_t;

void * startTestThread(void* targ) {
    concurrent_test_ctx_t *ctx = (concurrent_test_ctx_t *)targ;

    // try to exhaust all the chunks
    uint8_t **ptrs = (uint8_t**)malloc(ctx->max_chunks * sizeof(uint8_t *));
    for (int i = 0; i < ctx->max_chunks; i++) {
        ptrs[i] = ctx->slab_mgr->Alloc(1);
        if (ptrs[i] != NULL) {
            ctx->alloc_count++;
        } else {
            //LogError("Error allocating memory in thread {}", ctx->my_id);
        }
    }

    //LogInfo("Allocated {} chunks", ctx->alloc_count);
    sleep(1);

    // free all the memory
    for (int i = 0; i < ctx->max_chunks; i++) {
        error err = ctx->slab_mgr->Free(ptrs[i]);
        if (err.IsOK()) ctx->free_count++;
    }

    //LogInfo("Freed {} chunks", ctx->free_count);

    free(ptrs);
    pthread_exit(NULL);
    return NULL;
}

void * startAfTestThread(void* targ) {
    concurrent_test_ctx_t *ctx = (concurrent_test_ctx_t *)targ;

    // allocate and free a million alloc/frees
    for (int i = 0; i < NUM_ALLOCS_FREES; i++) {
        int len = (rand() & 0x3FFF) + 1;
        uint8_t *ptr = ctx->slab_mgr->Alloc(len);
        if (ptr != NULL) {
            ctx->alloc_count++;
        } else {
            //LogError("Error allocating memory of size {}. thread {}", len, ctx->my_id);
        }
        error err = ctx->slab_mgr->Free(ptr);
        if (err.IsOK()) ctx->free_count++;
    }
    //LogInfo("Allocated {} chunks", ctx->alloc_count);
    //LogInfo("Freed {} chunks", ctx->free_count);

    pthread_exit(NULL);
    return NULL;
}

TEST_F(SlabMgrTest, ParallelAllocTest) {
    pthread_t    threads[NUM_THREADS];
    concurrent_test_ctx_t test_ctx[NUM_THREADS];

    // setup thread context
    for (int i = 0; i < NUM_THREADS; i++) {
        test_ctx[i].slab_mgr = new SlabAllocator((SlabPool_t *)mem_, mem_, (i + 1));
        test_ctx[i].alloc_count = 0;
        test_ctx[i].free_count = 0;
        int num_pages  = (ktestMemSize / SHM_PAGE_SIZE) - 1;
        test_ctx[i].max_chunks = (CHUNKS_PER_PAGE(BASE_CHUNK_SIZE) * num_pages) / NUM_THREADS;
    }

    // start the threads in parallel
    pthread_attr_t tattr;
    pthread_attr_init (&tattr);
    pthread_attr_setscope(&tattr, PTHREAD_SCOPE_SYSTEM);
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], &tattr, &startTestThread, (void*)&test_ctx[i]);
    }

    // wait for the tests to complete
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
        ASSERT_EQ(test_ctx[i].alloc_count, (test_ctx[i].max_chunks)) << "some allocs failed";
        ASSERT_EQ(test_ctx[i].free_count, (test_ctx[i].max_chunks)) << "some frees failed";
    }

}

TEST_F(SlabMgrTest, AllocFreeBenchmarkTest) {
    pthread_t    threads[NUM_THREADS];
    concurrent_test_ctx_t test_ctx[NUM_THREADS];

    // setup thread context
    for (int i = 0; i < NUM_THREADS; i++) {
        test_ctx[i].slab_mgr = new SlabAllocator((SlabPool_t *)mem_, mem_, (i + 1));
        test_ctx[i].alloc_count = 0;
        test_ctx[i].free_count = 0;
        int num_pages  = (ktestMemSize / SHM_PAGE_SIZE) - 1;
        test_ctx[i].max_chunks = (CHUNKS_PER_PAGE(BASE_CHUNK_SIZE) * num_pages) / NUM_THREADS;
    }

    // start the threads in parallel
    pthread_attr_t tattr;
    pthread_attr_init (&tattr);
    pthread_attr_setscope(&tattr, PTHREAD_SCOPE_SYSTEM);
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], &tattr, &startAfTestThread, (void*)&test_ctx[i]);
    }

    // wait for the tests to complete
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
        ASSERT_EQ(test_ctx[i].alloc_count, NUM_ALLOCS_FREES) << "some allocs failed";
        ASSERT_EQ(test_ctx[i].free_count, NUM_ALLOCS_FREES) << "some frees failed";
    }
}

} // namespace

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
