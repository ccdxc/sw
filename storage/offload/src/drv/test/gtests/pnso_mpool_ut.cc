/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#include <gtest/gtest.h>
#include <vector>
#include <unistd.h>

#include "osal_logger.h"
#include "storage/offload/src/drv/pnso_mpool.h"

using namespace std;

#define PNSO_UT_OBJECT_SIZE	64
#define PNSO_UT_ALIGN_SIZE	64

class pnso_mpool_test : public ::testing::Test {
public:

protected:

    pnso_mpool_test() {
    }

    virtual ~pnso_mpool_test() {
    }

    // will be called immediately after the constructor before each test
    virtual void SetUp() {
    }

    // will be called immediately after each test before the destructor
    virtual void TearDown() {
    }
};

static void
ut_mpool_create_type(void)
{
	pnso_error_t err;
	enum mem_pool_type mpool_type;
	uint32_t num_objects, object_size, align_size;
	struct mem_pool *mpool;

	/* use this same setup across UTs */
	num_objects = PNSO_MIN_NUM_POOL_OBJECTS;
	object_size = PNSO_UT_OBJECT_SIZE;
	align_size = PNSO_UT_ALIGN_SIZE;

	/* ------------------------------------------------------------------ */
	mpool_type = MPOOL_TYPE_NONE;
	err = mpool_create(mpool_type, num_objects, object_size,
			align_size, &mpool);
	EXPECT_EQ(err, -EINVAL);

	mpool_type = MPOOL_TYPE_MAX;
	err = mpool_create(mpool_type, num_objects, object_size,
			align_size, &mpool);
	EXPECT_EQ(err, -EINVAL);

	/* ------------------------------------------------------------------ */
	mpool = NULL;
	mpool_type = MPOOL_TYPE_CPDC;
	err = mpool_create(mpool_type, num_objects, object_size,
			align_size, &mpool);
	EXPECT_NE(err, -EINVAL);
	EXPECT_NE(mpool, nullptr);
	EXPECT_EQ(mpool->mp_config.mpc_type, MPOOL_TYPE_CPDC);
	mpool_destroy(&mpool);
	EXPECT_EQ(mpool, nullptr);

	/* ------------------------------------------------------------------ */
	mpool_type = MPOOL_TYPE_CPDC_SGL;
	err = mpool_create(mpool_type, num_objects, object_size,
			align_size, &mpool);
	EXPECT_NE(err, -EINVAL);
	EXPECT_NE(mpool, nullptr);
	EXPECT_EQ(mpool->mp_config.mpc_type, MPOOL_TYPE_CPDC_SGL);
	mpool_destroy(&mpool);
	EXPECT_EQ(mpool, nullptr);

	/* ------------------------------------------------------------------ */
	mpool_type = MPOOL_TYPE_CPDC_STATUS;
	err = mpool_create(mpool_type, num_objects, object_size,
			align_size, &mpool);
	EXPECT_NE(err, -EINVAL);
	EXPECT_NE(mpool, nullptr);
	EXPECT_EQ(mpool->mp_config.mpc_type, MPOOL_TYPE_CPDC_STATUS);
	mpool_destroy(&mpool);
	EXPECT_EQ(mpool, nullptr);

	/* ------------------------------------------------------------------ */
	mpool_type = MPOOL_TYPE_XTS;
	err = mpool_create(mpool_type, num_objects, object_size,
			align_size, &mpool);
	EXPECT_NE(err, -EINVAL);
	EXPECT_NE(mpool, nullptr);
	EXPECT_EQ(mpool->mp_config.mpc_type, MPOOL_TYPE_XTS);
	mpool_destroy(&mpool);
	EXPECT_EQ(mpool, nullptr);

	/* ------------------------------------------------------------------ */
	mpool_type = MPOOL_TYPE_AOL;
	err = mpool_create(mpool_type, num_objects, object_size,
			align_size, &mpool);
	EXPECT_NE(err, -EINVAL);
	EXPECT_NE(mpool, nullptr);
	EXPECT_EQ(mpool->mp_config.mpc_type, MPOOL_TYPE_AOL);
	mpool_destroy(&mpool);
	EXPECT_EQ(mpool, nullptr);

	/* ------------------------------------------------------------------ */
	mpool_type = MPOOL_TYPE_CHAIN_ENTRY;
	err = mpool_create(mpool_type, num_objects, object_size,
			align_size, &mpool);
	EXPECT_NE(err, -EINVAL);
	EXPECT_NE(mpool, nullptr);
	EXPECT_EQ(mpool->mp_config.mpc_type, MPOOL_TYPE_CHAIN_ENTRY);
	mpool_destroy(&mpool);
	EXPECT_EQ(mpool, nullptr);
}

static void
ut_mpool_create_num_objects_cpdc(void)
{
	pnso_error_t err;
	enum mem_pool_type mpool_type;
	uint32_t num_objects, object_size, align_size;
	struct mem_pool *mpool;

	/* use this same setup across UTs */
	mpool_type = MPOOL_TYPE_CPDC;
	object_size = PNSO_UT_OBJECT_SIZE;
	align_size = PNSO_UT_ALIGN_SIZE;
	mpool = NULL;

	/* ------------------------------------------------------------------ */
	/* verify lower limit */
	num_objects = PNSO_MIN_NUM_CPDC_DESC - 1;
	err = mpool_create(mpool_type, num_objects, object_size,
			align_size, &mpool);
	EXPECT_EQ(err, -EINVAL);

	/* ------------------------------------------------------------------ */
	/* verify upper limit */
	num_objects = PNSO_MAX_NUM_CPDC_DESC + 1;
	err = mpool_create(mpool_type, num_objects, object_size,
			align_size, &mpool);
	EXPECT_EQ(err, -EINVAL);

	/* ------------------------------------------------------------------ */
	/* verify a valid case */
	num_objects = PNSO_MIN_NUM_CPDC_DESC;
	err = mpool_create(mpool_type, num_objects, object_size,
			align_size, &mpool);
	EXPECT_NE(err, -EINVAL);
	EXPECT_NE(mpool, nullptr);
	EXPECT_EQ(mpool->mp_config.mpc_type, MPOOL_TYPE_CPDC);
	EXPECT_EQ(mpool->mp_config.mpc_num_objects, num_objects);
	mpool_destroy(&mpool);
	EXPECT_EQ(mpool, nullptr);
}

static void
ut_mpool_create_num_objects_cpdc_status(void)
{
	pnso_error_t err;
	enum mem_pool_type mpool_type;
	uint32_t num_objects, object_size, align_size;
	struct mem_pool *mpool;

	/* use this same setup across UTs */
	mpool_type = MPOOL_TYPE_CPDC_STATUS;
	object_size = PNSO_UT_OBJECT_SIZE;
	align_size = PNSO_UT_ALIGN_SIZE;
	mpool = NULL;

	/* ------------------------------------------------------------------ */
	/* verify lower limit */
	num_objects = PNSO_MIN_NUM_CPDC_STATUS_DESC - 1;
	err = mpool_create(mpool_type, num_objects, object_size,
			align_size, &mpool);
	EXPECT_EQ(err, -EINVAL);

	/* ------------------------------------------------------------------ */
	/* verify upper limit */
	num_objects = PNSO_MAX_NUM_CPDC_STATUS_DESC + 1;
	err = mpool_create(mpool_type, num_objects, object_size,
			align_size, &mpool);
	EXPECT_EQ(err, -EINVAL);

	/* ------------------------------------------------------------------ */
	/* verify a valid case */
	num_objects = PNSO_MIN_NUM_CPDC_STATUS_DESC;
	err = mpool_create(mpool_type, num_objects, object_size,
			align_size, &mpool);
	EXPECT_NE(err, -EINVAL);
	EXPECT_NE(mpool, nullptr);
	EXPECT_EQ(mpool->mp_config.mpc_type, MPOOL_TYPE_CPDC_STATUS);
	EXPECT_EQ(mpool->mp_config.mpc_num_objects, num_objects);
	mpool_destroy(&mpool);
	EXPECT_EQ(mpool, nullptr);
}

static void
ut_mpool_create_num_objects_cpdc_sgl(void)
{
	pnso_error_t err;
	enum mem_pool_type mpool_type;
	uint32_t num_objects, object_size, align_size;
	struct mem_pool *mpool;

	/* use this same setup across UTs */
	mpool_type = MPOOL_TYPE_CPDC_SGL;
	object_size = PNSO_UT_OBJECT_SIZE;
	align_size = PNSO_UT_ALIGN_SIZE;
	mpool = NULL;

	/* ------------------------------------------------------------------ */
	/* verify lower limit */
	num_objects = PNSO_MIN_NUM_SGL_DESC - 1;
	err = mpool_create(mpool_type, num_objects, object_size,
			align_size, &mpool);
	EXPECT_EQ(err, -EINVAL);

	/* ------------------------------------------------------------------ */
	/* verify upper limit */
	num_objects = (PNSO_MAX_NUM_SGL_DESC * 2) + 1;
	err = mpool_create(mpool_type, num_objects, object_size,
			align_size, &mpool);
	EXPECT_EQ(err, -EINVAL);

	/* ------------------------------------------------------------------ */
	/* verify a valid case */
	num_objects = PNSO_MIN_NUM_SGL_DESC;
	err = mpool_create(mpool_type, num_objects, object_size,
			align_size, &mpool);
	EXPECT_NE(err, -EINVAL);
	EXPECT_NE(mpool, nullptr);
	EXPECT_EQ(mpool->mp_config.mpc_type, MPOOL_TYPE_CPDC_SGL);
	EXPECT_EQ(mpool->mp_config.mpc_num_objects, num_objects);
	mpool_destroy(&mpool);
	EXPECT_EQ(mpool, nullptr);
}

static void
ut_mpool_create_num_objects_xts(void)
{
	pnso_error_t err;
	enum mem_pool_type mpool_type;
	uint32_t num_objects, object_size, align_size;
	struct mem_pool *mpool;

	/* use this same setup across UTs */
	mpool_type = MPOOL_TYPE_XTS;
	object_size = PNSO_UT_OBJECT_SIZE;
	align_size = PNSO_UT_ALIGN_SIZE;
	mpool = NULL;

	/* ------------------------------------------------------------------ */
	/* verify lower limit */
	num_objects = PNSO_MIN_NUM_XTS_DESC - 1;
	err = mpool_create(mpool_type, num_objects, object_size,
			align_size, &mpool);
	EXPECT_EQ(err, -EINVAL);

	/* ------------------------------------------------------------------ */
	/* verify upper limit */
	num_objects = PNSO_MAX_NUM_XTS_DESC + 1;
	err = mpool_create(mpool_type, num_objects, object_size,
			align_size, &mpool);
	EXPECT_EQ(err, -EINVAL);

	/* ------------------------------------------------------------------ */
	/* verify a valid case */
	num_objects = PNSO_MIN_NUM_XTS_DESC;
	err = mpool_create(mpool_type, num_objects, object_size,
			align_size, &mpool);
	EXPECT_NE(err, -EINVAL);
	EXPECT_NE(mpool, nullptr);
	EXPECT_EQ(mpool->mp_config.mpc_type, MPOOL_TYPE_XTS);
	EXPECT_EQ(mpool->mp_config.mpc_num_objects, num_objects);
	mpool_destroy(&mpool);
	EXPECT_EQ(mpool, nullptr);
}

static void
ut_mpool_create_num_objects_aol(void)
{
	/* TODO-mpool: ... */
}

static void
ut_mpool_create_num_objects_chain_entry(void)
{
	/* TODO-mpool: ... */
}

static void
ut_mpool_create_num_objects(void)
{
	ut_mpool_create_num_objects_cpdc();
	ut_mpool_create_num_objects_cpdc_status();
	ut_mpool_create_num_objects_cpdc_sgl();
	ut_mpool_create_num_objects_xts();
	ut_mpool_create_num_objects_aol();
	ut_mpool_create_num_objects_chain_entry();
}

static void
ut_pool_create_align_size(void)
{
	pnso_error_t err;
	enum mem_pool_type mpool_type;
	uint32_t num_objects, object_size, align_size;
	struct mem_pool *mpool;

	/* use this same setup across UTs */
	mpool_type = MPOOL_TYPE_CPDC;
	num_objects = PNSO_MIN_NUM_POOL_OBJECTS;
	object_size = PNSO_UT_OBJECT_SIZE;
	mpool = NULL;

	/* ------------------------------------------------------------------ */
	/* verify invalid size */
	align_size = 0;
	err = mpool_create(mpool_type, num_objects, object_size,
			align_size, &mpool);
	EXPECT_EQ(err, -EINVAL);

	/* ------------------------------------------------------------------ */
	align_size = 7;
	err = mpool_create(mpool_type, num_objects, object_size,
			align_size, &mpool);
	EXPECT_EQ(err, -EINVAL);

	/* ------------------------------------------------------------------ */
	/* verify byte aligned ...  */
	align_size = 1;
	err = mpool_create(mpool_type, num_objects, object_size,
			align_size, &mpool);
	EXPECT_NE(err, -EINVAL);
	EXPECT_NE(mpool, nullptr);
	EXPECT_EQ(mpool->mp_config.mpc_type, MPOOL_TYPE_CPDC);
	EXPECT_EQ(mpool->mp_config.mpc_align_size, align_size);
	mpool_destroy(&mpool);
	EXPECT_EQ(mpool, nullptr);

	/* ------------------------------------------------------------------ */
	/* verify 4K page aligned ...  */
	align_size = PNSO_MEM_ALIGN_PAGE;
	err = mpool_create(mpool_type, num_objects, object_size,
			align_size, &mpool);
	EXPECT_NE(err, -EINVAL);
	EXPECT_NE(mpool, nullptr);
	EXPECT_EQ(mpool->mp_config.mpc_type, MPOOL_TYPE_CPDC);
	EXPECT_EQ(mpool->mp_config.mpc_align_size, align_size);
	mpool_destroy(&mpool);
	EXPECT_EQ(mpool, nullptr);
}

static void
ut_pool_create_object_size(void)
{
	pnso_error_t err;
	enum mem_pool_type mpool_type;
	uint32_t num_objects, object_size, align_size;
	struct mem_pool *mpool;

	/* use this same setup across UTs */
	mpool_type = MPOOL_TYPE_CPDC;
	num_objects = PNSO_MIN_NUM_POOL_OBJECTS;
	align_size = PNSO_UT_ALIGN_SIZE;
	mpool = NULL;

	/* ------------------------------------------------------------------ */
	/* verify invalid size */
	object_size = 0;
	err = mpool_create(mpool_type, num_objects, object_size,
			align_size, &mpool);
	EXPECT_EQ(err, -EINVAL);

	/* ------------------------------------------------------------------ */
	/* verify byte aligned ...  */
	object_size = 1;
	err = mpool_create(mpool_type, num_objects, object_size,
			align_size, &mpool);
	EXPECT_NE(err, -EINVAL);
	EXPECT_NE(mpool, nullptr);
	EXPECT_EQ(mpool->mp_config.mpc_type, MPOOL_TYPE_CPDC);
	EXPECT_EQ(mpool->mp_config.mpc_object_size, object_size);
	mpool_destroy(&mpool);
	EXPECT_EQ(mpool, nullptr);
}

static void
ut_pool_create_null_pool(void)
{
	pnso_error_t err;
	enum mem_pool_type mpool_type;
	uint32_t num_objects, object_size, align_size;
	struct mem_pool *mpool;

	/* use this same setup across UTs */
	mpool_type = MPOOL_TYPE_CPDC;
	num_objects = PNSO_MIN_NUM_POOL_OBJECTS;
	object_size = 1;
	align_size = PNSO_UT_ALIGN_SIZE;

	/* ------------------------------------------------------------------ */
	/* verify with NULL pointer for mpool */
	err = mpool_create(mpool_type, num_objects, object_size,
			align_size, NULL);
	EXPECT_EQ(err, -EINVAL);

	/* ------------------------------------------------------------------ */
	/* verify with non-NULL pointer for mpool */
	err = mpool_create(mpool_type, num_objects, object_size,
			align_size, &mpool);
	EXPECT_NE(err, -EINVAL);
	EXPECT_NE(mpool, nullptr);
	EXPECT_EQ(mpool->mp_config.mpc_type, MPOOL_TYPE_CPDC);
	EXPECT_EQ(mpool->mp_config.mpc_object_size, object_size);
	mpool_destroy(&mpool);
	EXPECT_EQ(mpool, nullptr);
}

static void
ut_pool_create_pool_size(void)
{
	pnso_error_t err;
	enum mem_pool_type mpool_type;
	uint32_t num_objects, object_size, align_size, pad_size;
	struct mem_pool *mpool;
	size_t pool_size;

	/* use this same setup across UTs */
	mpool_type = MPOOL_TYPE_CPDC;
	num_objects = PNSO_MIN_NUM_POOL_OBJECTS;

	/* ------------------------------------------------------------------ */
	/* verify the case of object and alignment size is same ... */
	object_size = PNSO_UT_OBJECT_SIZE;
	align_size = PNSO_UT_ALIGN_SIZE;
	err = mpool_create(mpool_type, num_objects, object_size,
			align_size, &mpool);
	EXPECT_NE(err, -EINVAL);
	EXPECT_NE(mpool, nullptr);
	EXPECT_EQ(mpool->mp_config.mpc_type, MPOOL_TYPE_CPDC);
	pool_size = num_objects * object_size;
	EXPECT_EQ(mpool->mp_config.mpc_pool_size, pool_size);
	mpool_destroy(&mpool);
	EXPECT_EQ(mpool, nullptr);

	/* ------------------------------------------------------------------ */
	/* verify the case of size of object is less than alignment size */
	object_size = PNSO_UT_OBJECT_SIZE - 1;
	align_size = PNSO_UT_ALIGN_SIZE;
	err = mpool_create(mpool_type, num_objects, object_size,
			align_size, &mpool);
	EXPECT_NE(err, -EINVAL);
	EXPECT_NE(mpool, nullptr);
	EXPECT_EQ(mpool->mp_config.mpc_type, MPOOL_TYPE_CPDC);
	pool_size = num_objects * align_size;
	EXPECT_EQ(mpool->mp_config.mpc_pool_size, pool_size);
	mpool_destroy(&mpool);
	EXPECT_EQ(mpool, nullptr);

	/* ------------------------------------------------------------------ */
	/* verify the case of size of object is greater than alignment size */
	object_size = PNSO_UT_OBJECT_SIZE - 1;
	align_size = PNSO_UT_ALIGN_SIZE;
	err = mpool_create(mpool_type, num_objects, object_size,
			align_size, &mpool);
	EXPECT_NE(err, -EINVAL);
	EXPECT_NE(mpool, nullptr);
	EXPECT_EQ(mpool->mp_config.mpc_type, MPOOL_TYPE_CPDC);
	pad_size = (num_objects * (align_size - (object_size % align_size)));
	pool_size = (num_objects * object_size) + pad_size;
	EXPECT_EQ(mpool->mp_config.mpc_pool_size, pool_size);
	mpool_destroy(&mpool);
	EXPECT_EQ(mpool, nullptr);
}

TEST_F(pnso_mpool_test, ut_mpool_create) {
	/* input parameter checks */
	ut_mpool_create_type();
	ut_mpool_create_num_objects();
	ut_pool_create_align_size();
	ut_pool_create_object_size();
	ut_pool_create_null_pool();

	/* sanity checks */
	ut_pool_create_pool_size();

	/*
	 * TODO-mpool:
	 * 	Cannot mimic memory allocation failure checks via osal calls,
	 * 	at this time. One approach of getting through it could be to
	 * 	define a function table, and tweak it witha dedicated tables
	 * 	for production and UTs.
	 *
	 */
}

TEST_F(pnso_mpool_test, ut_mpool_destroy) {
	pnso_error_t err;
	enum mem_pool_type mpool_type;
	uint32_t num_objects, object_size, align_size;
	struct mem_pool *mpool;

	/* use this same setup across UTs */
	mpool_type = MPOOL_TYPE_CPDC;
	num_objects = PNSO_MIN_NUM_POOL_OBJECTS;
	object_size = PNSO_UT_OBJECT_SIZE;
	align_size = PNSO_UT_ALIGN_SIZE;

	/* ------------------------------------------------------------------ */
	/* attempt a 'destroy' with NULL mpool and then with a valid one */
	err = mpool_create(mpool_type, num_objects, object_size,
			align_size, &mpool);
	EXPECT_NE(err, -EINVAL);
	EXPECT_NE(mpool, nullptr);
	EXPECT_EQ(mpool->mp_config.mpc_type, MPOOL_TYPE_CPDC);
	mpool_destroy(NULL);
	mpool_destroy(&mpool);
	EXPECT_EQ(mpool, nullptr);
}

TEST_F(pnso_mpool_test, ut_mpool_get_object) {
	pnso_error_t err;
	enum mem_pool_type mpool_type;
	uint32_t num_objects, object_size, align_size, i;
	struct mem_pool *mpool;
	void *p, *q;

	/* use this same setup across UTs */
	mpool_type = MPOOL_TYPE_CPDC;
	num_objects = PNSO_MIN_NUM_POOL_OBJECTS;
	object_size = PNSO_UT_OBJECT_SIZE;
	align_size = PNSO_UT_ALIGN_SIZE;

	/* ------------------------------------------------------------------ */
	err = mpool_create(mpool_type, num_objects, object_size,
			align_size, &mpool);
	EXPECT_NE(err, -EINVAL);
	EXPECT_NE(mpool, nullptr);
	EXPECT_EQ(mpool->mp_config.mpc_type, MPOOL_TYPE_CPDC);

	/* attempt a 'get' with NULL object */
	p = mpool_get_object(NULL);
	EXPECT_EQ(p, nullptr);
	mpool_destroy(&mpool);
	EXPECT_EQ(mpool, nullptr);

	/* ------------------------------------------------------------------ */
	err = mpool_create(mpool_type, num_objects, object_size,
			align_size, &mpool);
	EXPECT_NE(err, -EINVAL);
	EXPECT_NE(mpool, nullptr);
	EXPECT_EQ(mpool->mp_config.mpc_type, MPOOL_TYPE_CPDC);

	/* attempt a 'get' with non-NULL object */
	p = mpool_get_object(mpool);
	EXPECT_NE(p, nullptr);
	mpool_destroy(&mpool);
	EXPECT_EQ(mpool, nullptr);

	/* ------------------------------------------------------------------ */
	err = mpool_create(mpool_type, num_objects, object_size,
			align_size, &mpool);
	EXPECT_NE(err, -EINVAL);
	EXPECT_NE(mpool, nullptr);
	EXPECT_EQ(mpool->mp_config.mpc_type, MPOOL_TYPE_CPDC);

	/* attempt a 'get, put, get' with non-NULL object */
	p = mpool_get_object(mpool);
	EXPECT_NE(p, nullptr);
	q = p;
	err = mpool_put_object(mpool, p);
	EXPECT_EQ(err, PNSO_OK);
	p = mpool_get_object(mpool);
	EXPECT_NE(p, nullptr);
	EXPECT_EQ(p, q);

	mpool_destroy(&mpool);
	EXPECT_EQ(mpool, nullptr);

	/* ------------------------------------------------------------------ */
	err = mpool_create(mpool_type, num_objects, object_size,
			align_size, &mpool);
	EXPECT_NE(err, -EINVAL);
	EXPECT_NE(mpool, nullptr);
	EXPECT_EQ(mpool->mp_config.mpc_type, MPOOL_TYPE_CPDC);

	/* pop all and then attempt a 'get' */
	for (i = 0; i < num_objects; i++) {
		p = mpool_get_object(mpool);
		EXPECT_NE(p, nullptr);
		q = p;
	}
	p = mpool_get_object(mpool);
	EXPECT_EQ(p, nullptr);

	/* push the spare one, and attempt a 'get' */
	err = mpool_put_object(mpool, q);
	EXPECT_EQ(err, PNSO_OK);
	p = mpool_get_object(mpool);
	EXPECT_NE(p, nullptr);

	mpool_destroy(&mpool);
	EXPECT_EQ(mpool, nullptr);
}

TEST_F(pnso_mpool_test, ut_mpool_put_object) {
	pnso_error_t err;
	enum mem_pool_type mpool_type;
	uint32_t num_objects, object_size, align_size;
	struct mem_pool *mpool;
	void *p;

	/* use this same setup across UTs */
	mpool_type = MPOOL_TYPE_CPDC;
	num_objects = PNSO_MIN_NUM_POOL_OBJECTS;
	object_size = PNSO_UT_OBJECT_SIZE;
	align_size = PNSO_UT_ALIGN_SIZE;

	/* ------------------------------------------------------------------ */
	err = mpool_create(mpool_type, num_objects, object_size,
			align_size, &mpool);
	EXPECT_NE(err, -EINVAL);
	EXPECT_NE(mpool, nullptr);
	EXPECT_EQ(mpool->mp_config.mpc_type, MPOOL_TYPE_CPDC);
	p = mpool_get_object(NULL);
	EXPECT_EQ(p, nullptr);

	/* attempt a 'put' with NULL pool */
	err = mpool_put_object(NULL, p);
	EXPECT_EQ(err, -EINVAL);

	/* attempt a 'put' with NULL object */
	err = mpool_put_object(mpool, NULL);
	EXPECT_EQ(err, -EINVAL);

	mpool_destroy(&mpool);
	EXPECT_EQ(mpool, nullptr);

	/* ------------------------------------------------------------------ */
	err = mpool_create(mpool_type, num_objects, object_size,
			align_size, &mpool);
	EXPECT_NE(err, -EINVAL);
	EXPECT_NE(mpool, nullptr);
	EXPECT_EQ(mpool->mp_config.mpc_type, MPOOL_TYPE_CPDC);

	/* attempt a 'get, put' with non-NULL object */
	p = mpool_get_object(mpool);
	EXPECT_NE(p, nullptr);
	err = mpool_put_object(mpool, p);
	EXPECT_EQ(err, PNSO_OK);

	mpool_destroy(&mpool);
	EXPECT_EQ(mpool, nullptr);

	/* ------------------------------------------------------------------ */
	err = mpool_create(mpool_type, num_objects, object_size,
			align_size, &mpool);
	EXPECT_NE(err, -EINVAL);
	EXPECT_NE(mpool, nullptr);
	EXPECT_EQ(mpool->mp_config.mpc_type, MPOOL_TYPE_CPDC);

	/* attempt a 'put' with non-NULL object on a full stack */
	err = mpool_put_object(mpool, p);
	EXPECT_EQ(err, -ENOTEMPTY);

	mpool_destroy(&mpool);
	EXPECT_EQ(mpool, nullptr);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
