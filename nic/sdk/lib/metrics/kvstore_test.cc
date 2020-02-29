// {C} Copyright 2020 Pensando Systems Inc. All rights reserved.
// This file was forked from delphi

#include <unistd.h>
#include <ev++.h>
#include "gtest/gtest.h"
#include "shm.hpp"

namespace {
using namespace std;
using namespace sdk;
using namespace sdk::metrics;

class KvstoreTest : public testing::Test {
protected:
    ShmPtr client_shm_;
public:
    virtual void SetUp() {
        usleep(1000);

        // create the client
        client_shm_ = make_shared<Shm>();
        error err = client_shm_->MemMap(SDK_METRICS_SHM_NAME,
                                        SDK_METRICS_SHM_SIZE);
        assert(err.IsOK());
    }
    virtual void TearDown() {
        usleep(1000);
        client_shm_->MemUnmap();
    }
};

TEST_F(KvstoreTest, BasicKvTest) {
    client_shm_->Kvstore()->DumpInfo();

    // create a table
    TableMgrUptr tbl = client_shm_->Kvstore()->CreateTable("test_kind", 100);
    ASSERT_TRUE(tbl != NULL) << "Failed to create table";

    // create an entry in table
    const char *key = "test_key";
    const char *value = "test_value";
    char *valptr = (char *)tbl->Create(key, strlen(key), (strlen(value) + 1));
    ASSERT_TRUE(valptr != NULL) << "error creating key";

    // copy the data
    strcpy(valptr, value);

    // get the key
    char *gptr = (char *)tbl->Find(key, strlen(key));
    ASSERT_TRUE(gptr != NULL) << "find failed";
    ASSERT_EQ(gptr, valptr) << "value pointers did not match";
    char *gptr2 = (char *)client_shm_->Kvstore()->Table("test_kind")->Find(key, strlen(key));
    ASSERT_TRUE(gptr2 != NULL);
    ASSERT_EQ(gptr2, valptr) << "value pointers did not match";
    ASSERT_EQ(tbl->RefCount(gptr), 3) << "refcount for hash entry is incorrect";

    // dump kvstore info
    client_shm_->DumpMeta();
    tbl->DumpTable();

    // delete the key
    error err = tbl->Delete(key, strlen(key));
    ASSERT_EQ(err, error::OK()) << "Error deleting the key";
    ASSERT_EQ(tbl->RefCount(gptr), 2) << "refcount for hash entry is incorrect";

    // dump kvstore info
    client_shm_->DumpMeta();
    tbl->DumpTable();

    // verify find returns error
    char * gptr3 = (char *)tbl->Find(key, strlen(key));
    ASSERT_TRUE(gptr3 == NULL) << "find suceeded after delete";

    // release memory
    tbl->Release(gptr);
    ASSERT_EQ(tbl->RefCount(gptr), 1) << "refcount for hash entry is incorrect";
    tbl->Release(gptr2);
    client_shm_->DumpMeta();
    tbl->DumpTable();
}

TEST_F(KvstoreTest, TableIteratorTest) {
    // create a table
    TableMgrUptr tbl = client_shm_->Kvstore()->CreateTable("test_kind", 100);
    ASSERT_TRUE(tbl != NULL) << "Failed to create table";

    // create an entry in table
    const char *key1 = "test_key1";
    const char *value1 = "test_value1";
    char *valptr = (char *)tbl->Create(key1, strlen(key1), (strlen(value1) + 1));
    ASSERT_TRUE(valptr != NULL) << "error creating key";
    strcpy(valptr, value1);

    // create second entry in table
    const char *key2 = "test_key2";
    const char *value2 = "test_value2";
    valptr = (char *)tbl->Create(key2, strlen(key2), (strlen(value2) + 1));
    ASSERT_TRUE(valptr != NULL) << "error creating key";
    strcpy(valptr, value2);

    // verify iterator returns two entries
    int exp_count = 0;
    for (TableIterator it = tbl->Iterator(); it.IsNotNil(); it.Next()) {
        // LogInfo("Got key {} value {}", it.Key(), it.Value());
        ASSERT_TRUE((string(it.Key()) == string(key1)) || (string(it.Key()) == string(key2))) << "invalid key";
        ASSERT_TRUE((string(it.Value()) == string(value1)) || (string(it.Value()) == string(value2))) << "invalid value";
        exp_count++;
    }
    ASSERT_TRUE(exp_count == 2) << "unexpected number of keys";

    // verify find works with multiple keys
    char *gptr = (char *)tbl->Find(key1, strlen(key1));
    ASSERT_TRUE(gptr != NULL) << "find failed";
    tbl->Release(gptr);
    tbl->DumpTable();

    // delete one key
    error err = tbl->Delete(key1, strlen(key1));
    ASSERT_EQ(err, error::OK()) << "Error deleting the key";

    exp_count = 0;
    for (TableIterator it = tbl->Iterator(); it.IsNotNil(); it.Next()) {
        // LogInfo("Got key {} value {}", it.Key(), it.Value());
        ASSERT_TRUE((string(it.Key()) == string(key2))) << "invalid key";
        ASSERT_TRUE((string(it.Value()) == string(value2))) << "invalid value";
        exp_count++;
    }
    ASSERT_TRUE(exp_count == 1) << "unexpected number of keys";

    // delete second key
    err = tbl->Delete(key2, strlen(key2));
    ASSERT_EQ(err, error::OK()) << "Error deleting the key";

    exp_count = 0;
    for (TableIterator it = tbl->Iterator(); it.IsNotNil(); it.Next()) {
        // LogInfo("Got key {} value {}", it.Key(), it.Value());
        exp_count++;
    }
    ASSERT_TRUE(exp_count == 0) << "unexpected number of keys";
}

TEST_F(KvstoreTest, HashCollisionTest) {
    int test_count = 400;
    TableMgrUptr tbl = client_shm_->Kvstore()->CreateTable("test_collision_kind", 10);
    ASSERT_TRUE(tbl != NULL) << "Failed to create table";

    // create entries in table
    for (int i = 0; i < test_count; i++) {
        char key[100];
        char value[100];
        sprintf(key, "test_key-%d", i);
        sprintf(value, "test_value-%d", i);

        char *valptr = (char *)tbl->Create(key, (strlen(key)), (strlen(value) + 1));
        ASSERT_TRUE(valptr != NULL) << "error creating key";
        strcpy(valptr, value);
    }

    tbl->DumpTable();

    // verify find works
    for (int i = 0; i < test_count; i++) {
        char key[100];
        char value[100];
        sprintf(key, "test_key-%d", i);
        sprintf(value, "test_value-%d", i);

        char *gptr = (char *)tbl->Find(key, (strlen(key)));
        ASSERT_TRUE(gptr != NULL) << "find failed";
        ASSERT_EQ(string(gptr), string(value)) << "value did not match";
        tbl->Release(gptr);
    }


    // verify iterator find all the entries
    int exp_count = 0;
    for (TableIterator it = tbl->Iterator(); it.IsNotNil(); it.Next()) {
        // LogInfo("Got key {} value {}", it.Key(), it.Value());
        exp_count++;
    }
    ASSERT_TRUE(exp_count == test_count) << "unexpected number of keys";

    // delete all the keys
    for (int i = 0; i < test_count; i++) {
        char key[100];
        char value[100];
        sprintf(key, "test_key-%d", i);
        sprintf(value, "test_value-%d", i);

        error err = tbl->Delete(key, (strlen(key)));
        ASSERT_EQ(err, error::OK()) << "Error deleting the key";
    }

    tbl->DumpTable();
    client_shm_->DumpMeta();
}

TEST_F(KvstoreTest, KvPublishTest) {
    client_shm_->Kvstore()->DumpInfo();

    // create a table
    TableMgrUptr tbl = client_shm_->Kvstore()->CreateTable("test_pub_kind", 100);
    ASSERT_TRUE(tbl != NULL) << "Failed to create table";

    // create an entry in table
    const char *key = "test_key";
    const char *value = "test_value";
    error err = tbl->Publish(key, strlen(key), value, (strlen(value) + 1));
    ASSERT_TRUE(err.IsOK()) << "error publishing key";

    // get the key
    char *gptr = (char *)tbl->Find(key, strlen(key));
    ASSERT_TRUE(gptr != NULL) << "find failed";
    ASSERT_EQ(tbl->RefCount(gptr), 2) << "refcount for hash entry is incorrect";

    // dump kvstore info
    printf("------ after first publish and find -------\n");
    client_shm_->DumpMeta();
    tbl->DumpTable();

    // publish a new value
    const char *value2 = "test_value2";
    err = tbl->Publish(key, strlen(key), value2, (strlen(value2) + 1));
    ASSERT_TRUE(err.IsOK()) << "error publishing new value";

    // dump kvstore info
    printf("------ after second publish -------\n");
    client_shm_->DumpMeta();
    tbl->DumpTable();

    char *gptr2 = (char *)tbl->Find(key, strlen(key));
    ASSERT_TRUE(gptr != NULL) << "find failed after publishing new value";
    printf("Refcounts: old: %d, new: %d\n", tbl->RefCount(gptr), tbl->RefCount(gptr2));
    ASSERT_EQ(tbl->RefCount(gptr), 1) << "refcount for old hash entry is incorrect";
    ASSERT_EQ(tbl->RefCount(gptr2), 2) << "refcount for new hash entry is incorrect";

    // dump kvstore info
    printf("------ after second publish and find -------\n");
    client_shm_->DumpMeta();
    tbl->DumpTable();

    // delete the key
    err = tbl->Delete(key, strlen(key));
    ASSERT_EQ(err, error::OK()) << "Error deleting the key";
    ASSERT_EQ(tbl->RefCount(gptr2), 1) << "refcount for new hash entry is incorrect";

    // dump kvstore info
    printf("------ after delete -------\n");
    client_shm_->DumpMeta();
    tbl->DumpTable();

    // verify find returns error
    char * gptr3 = (char *)tbl->Find(key, strlen(key));
    ASSERT_TRUE(gptr3 == NULL) << "find suceeded after delete";

    // release memory
    tbl->Release(gptr);
    tbl->Release(gptr2);
    printf("------ after release -------\n");
    client_shm_->DumpMeta();
    tbl->DumpTable();
}

TEST_F(KvstoreTest, DpstatsTest) {
    error err;

    // create a table
    TableMgrUptr tbl = client_shm_->Kvstore()->CreateTable("test_dp_kind", 100);
    ASSERT_TRUE(tbl != NULL) << "Failed to create table";
    printf("After creating table:\n");
    client_shm_->Kvstore()->DumpInfo();
    client_shm_->DumpMeta();

    // create a dpstats entry in table
    const char *key = "test_key";
    uint64_t   pal_addr = 0x1234567812345678ll;
    auto valptr = tbl->CreateDpstats(key, strlen(key), pal_addr, 3000);
    ASSERT_TRUE(valptr != NULL) << "find failed";

    // get the key
    uint64_t *gptr = (uint64_t *)tbl->Find(key, strlen(key));
    ASSERT_TRUE(gptr != NULL) << "find failed";
    ASSERT_EQ(*gptr, pal_addr) << "pal address did not match";
    ASSERT_EQ(tbl->RefCount(gptr), 2) << "refcount for hash entry is incorrect";

    // dump kvstore info
    printf("After creating entry:\n");
    tbl->DumpTable();
    tbl->DumpEntry(key, strlen(key));
    client_shm_->DumpMeta();

    // delete the key
    err = tbl->Delete(key, strlen(key));
    ASSERT_EQ(err, error::OK()) << "Error deleting the key";
    ASSERT_EQ(tbl->RefCount(gptr), 1) << "refcount for hash entry is incorrect";

    // dump kvstore info
    printf("After deleting entry:\n");
    tbl->DumpTable();

    // verify find returns error
    char * gptr3 = (char *)tbl->Find(key, strlen(key));
    ASSERT_TRUE(gptr3 == NULL) << "find suceeded after delete";

    // release memory
    tbl->Release(gptr);
    tbl->DumpTable();
}

TEST_F(KvstoreTest, KvstoreBenchmark) {
    // create a table
    TableMgrUptr tbl = client_shm_->Kvstore()->CreateTable("test_bench_kind", 100);
    ASSERT_TRUE(tbl != NULL) << "Failed to create table";

    for (int i = 0; i < (200 * 1000); i++) {
        int32_t key = i + 1;
        char *valptr = (char *)tbl->Create((char *)&key, sizeof(key), 20);
        ASSERT_TRUE(valptr != NULL) << "error creating key";
        error err = tbl->Delete((char *)&key, sizeof(key));
        ASSERT_EQ(err, error::OK()) << "Error deleting the key";
    }
}

TEST_F(KvstoreTest, KvstoreFindBenchmark) {
    int num_entries = 2000;

    // create a table
    TableMgrUptr tbl = client_shm_->Kvstore()->CreateTable("test_bench_kind", 100);
    ASSERT_TRUE(tbl != NULL) << "Failed to create table";

    for (int i = 0; i < num_entries; i++) {
        int32_t key = i + 1;
        char *valptr = (char *)tbl->Create((char *)&key, sizeof(key), 20);
        ASSERT_TRUE(valptr != NULL) << "error creating key";
    }

    for (int i = 0; i < (1000 * 1000); i++) {
        int32_t key = (i  % num_entries) + 1;

        char *gptr = (char *)tbl->Find((char *)&key, sizeof(key));
        ASSERT_TRUE(gptr != NULL) << "Error getting key";
        ASSERT_EQ(tbl->RefCount(gptr), 2) << "incorrect ref count";
        error err = tbl->Release(gptr);
        ASSERT_EQ(err, error::OK()) << "Error releasing the entry";
    }

    for (int i = 0; i < num_entries; i++) {
        int32_t key = i + 1;
        error err = tbl->Delete((char *)&key, sizeof(key));
        ASSERT_EQ(err, error::OK()) << "Error deleting the key";
    }
    client_shm_->DumpMeta();
}

#define NUM_THREADS 25
#define NUM_CREATE_DELETE (10 * 1000)

typedef struct concurrent_test_ctx_ {
    TableMgrUptr tbl;
    int my_id;
    int create_count;
    int delete_count;
    int find_count;
    int release_count;
} concurrent_test_ctx_t;

void * startTestThread(void* targ) {
    concurrent_test_ctx_t *ctx = (concurrent_test_ctx_t *)targ;

    // error checking
    if (ctx->tbl == NULL) {
        pthread_exit(NULL);
        return NULL;
    }

    // repeatedly create/ delete
    for (int i = 0; i < NUM_CREATE_DELETE; i++) {
        int32_t key = (i + 1) * NUM_THREADS + ctx->my_id;
        char *valptr = (char *)ctx->tbl->Create((char *)&key, sizeof(key), 20);
        if (valptr != NULL) ctx->create_count++;

        char *gptr = (char *)ctx->tbl->Find((char *)&key, sizeof(key));
        if (gptr != NULL) {
            ctx->find_count++;
            error err = ctx->tbl->Release(gptr);
            if (err.IsOK()) ctx->release_count++;
        }

        error err = ctx->tbl->Delete((char *)&key, sizeof(key));
        if (err.IsOK()) ctx->delete_count++;
    }

    // LogInfo("Created {} entries", ctx->create_count);
    // LogInfo("Deleted {} entries", ctx->delete_count);

    pthread_exit(NULL);
    return NULL;
}

TEST_F(KvstoreTest, ParallelCreateDeleteTest) {
    pthread_t    threads[NUM_THREADS];
    concurrent_test_ctx_t test_ctx[NUM_THREADS];
    usleep(1000);

    // create a table
    TableMgrUptr tbl = client_shm_->Kvstore()->CreateTable("ptest", 5);
    ASSERT_TRUE(tbl != NULL) << "Failed to create table";

    // setup thread context
    for (int i = 0; i < NUM_THREADS; i++) {
        test_ctx[i].tbl = client_shm_->Kvstore()->Table("ptest");
        test_ctx[i].create_count = 0;
        test_ctx[i].delete_count = 0;
        test_ctx[i].find_count = 0;
        test_ctx[i].release_count= 0;
        test_ctx[i].my_id = i + 1;
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
        ASSERT_EQ(test_ctx[i].create_count, NUM_CREATE_DELETE) << "some creates failed";
        ASSERT_EQ(test_ctx[i].delete_count, NUM_CREATE_DELETE) << "some deletes failed";
        ASSERT_EQ(test_ctx[i].find_count, NUM_CREATE_DELETE) << "some finds failed";
        ASSERT_EQ(test_ctx[i].release_count, NUM_CREATE_DELETE) << "some releases failed";

    }
}

#define NUM_OBJS_TO_PUBLISH 1000
#define NUM_PUBLISH_ITERATIONS 1000
#define NUM_WALK_ITERATIONS 3000
#define PUBLISH_VAL_LEN 18
#define NUM_PUBLISH_THREADS 5
#define NUM_ITERATOR_THREADS 20
#define PUBLISHER_EN 1
#define ITERATOR_EN 1

void * runPublishThread(void* targ) {
    concurrent_test_ctx_t *ctx = (concurrent_test_ctx_t *)targ;
    char val[] = "01234567890123456789" ;

    // repeatedly publish
    for (int iter = 0; iter < NUM_PUBLISH_ITERATIONS; iter++) {
        for (int i = 0; i < NUM_OBJS_TO_PUBLISH; i++) {
            int32_t key = (i + 1) * NUM_THREADS + ctx->my_id;
            auto err = ctx->tbl->Publish((char *)&key, sizeof(key), val, PUBLISH_VAL_LEN);
            if (err.IsOK()) ctx->create_count++;
        }
    }

    // LogInfo("Published {} entries, {} iterations", ctx->create_count, NUM_PUBLISH_ITERATIONS);

    pthread_exit(NULL);
    return NULL;
}

void * runIteratorThread(void* targ) {
    concurrent_test_ctx_t *ctx = (concurrent_test_ctx_t *)targ;

    // repeatedly iterate over the table
    for (int iter = 0; iter < NUM_WALK_ITERATIONS; iter++) {            
        for (TableIterator it = ctx->tbl->Iterator(); it.IsNotNil(); it.Next()) {
            if ((it.Keylen() == sizeof(int32_t)) && (it.ValLen() == PUBLISH_VAL_LEN)) {
                ctx->find_count++;
            } else {
                // LogError("Invalid keylen {} or vallen {}", it.Keylen(), it.ValLen());
            }
        }
    }

    // LogInfo("Iterated {} entries, {} iterations", ctx->find_count, NUM_WALK_ITERATIONS);

    pthread_exit(NULL);
    return NULL;
}

TEST_F(KvstoreTest, ParallelPublishIterateTest) {
    pthread_t    pub_threads[NUM_PUBLISH_THREADS];
    pthread_t    iter_threads[NUM_ITERATOR_THREADS];
    concurrent_test_ctx_t pub_ctx[NUM_PUBLISH_THREADS];
    concurrent_test_ctx_t iter_ctx[NUM_ITERATOR_THREADS];
    usleep(1000);
    const char *names[] = { "ppitest1", "ppitest2", "ppitest3", "ppitest4", "ppitest5" };

    // create tables
    for (int i = 0; i < NUM_PUBLISH_THREADS; i++) {
        TableMgrUptr tbl = client_shm_->Kvstore()->CreateTable(names[i], 2000);
        ASSERT_TRUE(tbl != NULL) << "Failed to create table";
    }
    client_shm_->Kvstore()->DumpInfo();
    client_shm_->DumpMeta();

    // setup thread context
    for (int i = 0; i < NUM_PUBLISH_THREADS; i++) {
        pub_ctx[i].tbl = client_shm_->Kvstore()->Table(names[i]);
        pub_ctx[i].create_count = 0;
        pub_ctx[i].find_count = 0;
        pub_ctx[i].my_id = i + 1;
    }
    for (int i = 0; i < NUM_ITERATOR_THREADS; i++) {
        iter_ctx[i].tbl = client_shm_->Kvstore()->Table(names[i%NUM_PUBLISH_THREADS]);
        iter_ctx[i].create_count = 0;
        iter_ctx[i].find_count = 0;
        iter_ctx[i].my_id = i + 1 + NUM_PUBLISH_THREADS;
    }

    // start the threads in parallel
    pthread_attr_t tattr;
    pthread_attr_init (&tattr);
    pthread_attr_setscope(&tattr, PTHREAD_SCOPE_SYSTEM);
    if (PUBLISHER_EN > 0) {
    for (int i = 0; i < NUM_PUBLISH_THREADS; i++) {
        pthread_create(&pub_threads[i], &tattr, &runPublishThread, (void*)&pub_ctx[i]);
    }
    }
    if (ITERATOR_EN > 0) {
    for (int i = 0; i < NUM_ITERATOR_THREADS; i++) {
        pthread_create(&iter_threads[i], &tattr, &runIteratorThread, (void*)&iter_ctx[i]);
    }
    }

    // wait for the tests to complete
    if (PUBLISHER_EN > 0) {
    for (int i = 0; i < NUM_PUBLISH_THREADS; i++) {
        pthread_join(pub_threads[i], NULL);
    }
    }
    if (ITERATOR_EN > 0) {
    for (int i = 0; i < NUM_ITERATOR_THREADS; i++) {
        pthread_join(iter_threads[i], NULL);
    }
    }
    client_shm_->Kvstore()->DumpInfo();
    client_shm_->DumpMeta();
}

} // namespace

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
