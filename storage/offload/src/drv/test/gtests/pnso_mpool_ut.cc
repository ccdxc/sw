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

#define PNSO_UT_MPOOL_NUM_OBJECTS 16

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

	OSAL_LOG_DEBUG("enter ...");

	/* use this same setup across UTs */
	num_objects = PNSO_UT_MPOOL_NUM_OBJECTS;
	object_size = PNSO_UT_OBJECT_SIZE;
	align_size = PNSO_UT_ALIGN_SIZE;

	OSAL_LOG_INFO("=== verify invalid pool type");
	mpool_type = MPOOL_TYPE_NONE;
	err = mpool_create(mpool_type, num_objects, object_size,
			align_size, &mpool);
	EXPECT_EQ(err, EINVAL);

	mpool_type = MPOOL_TYPE_MAX;
	err = mpool_create(mpool_type, num_objects, object_size,
			align_size, &mpool);
	EXPECT_EQ(err, EINVAL);

	OSAL_LOG_INFO("=== verify valid pool type - cpdc");
	mpool = NULL;
	mpool_type = MPOOL_TYPE_CPDC_DESC;
	err = mpool_create(mpool_type, num_objects, object_size,
			align_size, &mpool);
	EXPECT_NE(err, EINVAL);
	EXPECT_NE(mpool, nullptr);
	EXPECT_EQ(mpool->mp_config.mpc_type, MPOOL_TYPE_CPDC_DESC);
	mpool_destroy(&mpool);
	EXPECT_EQ(mpool, nullptr);

	OSAL_LOG_INFO("=== verify valid pool type - sgl");
	mpool_type = MPOOL_TYPE_CPDC_SGL;
	err = mpool_create(mpool_type, num_objects, object_size,
			align_size, &mpool);
	EXPECT_NE(err, EINVAL);
	EXPECT_NE(mpool, nullptr);
	EXPECT_EQ(mpool->mp_config.mpc_type, MPOOL_TYPE_CPDC_SGL);
	mpool_destroy(&mpool);
	EXPECT_EQ(mpool, nullptr);

	OSAL_LOG_INFO("=== verify valid pool type - cpdc status");
	mpool_type = MPOOL_TYPE_CPDC_STATUS_DESC;
	err = mpool_create(mpool_type, num_objects, object_size,
			align_size, &mpool);
	EXPECT_NE(err, EINVAL);
	EXPECT_NE(mpool, nullptr);
	EXPECT_EQ(mpool->mp_config.mpc_type, MPOOL_TYPE_CPDC_STATUS_DESC);
	mpool_destroy(&mpool);
	EXPECT_EQ(mpool, nullptr);

	OSAL_LOG_INFO("=== verify valid pool type - xts");
	mpool_type = MPOOL_TYPE_XTS_DESC;
	err = mpool_create(mpool_type, num_objects, object_size,
			align_size, &mpool);
	EXPECT_NE(err, EINVAL);
	EXPECT_NE(mpool, nullptr);
	EXPECT_EQ(mpool->mp_config.mpc_type, MPOOL_TYPE_XTS_DESC);
	mpool_destroy(&mpool);
	EXPECT_EQ(mpool, nullptr);

	OSAL_LOG_INFO("=== verify valid pool type - aol");
	mpool_type = MPOOL_TYPE_XTS_AOL;
	err = mpool_create(mpool_type, num_objects, object_size,
			align_size, &mpool);
	EXPECT_NE(err, EINVAL);
	EXPECT_NE(mpool, nullptr);
	EXPECT_EQ(mpool->mp_config.mpc_type, MPOOL_TYPE_XTS_AOL);
	mpool_destroy(&mpool);
	EXPECT_EQ(mpool, nullptr);

	OSAL_LOG_INFO("=== verify valid pool type - service chain");
	mpool_type = MPOOL_TYPE_SERVICE_CHAIN;
	err = mpool_create(mpool_type, num_objects, object_size,
			align_size, &mpool);
	EXPECT_NE(err, EINVAL);
	EXPECT_NE(mpool, nullptr);
	EXPECT_EQ(mpool->mp_config.mpc_type, MPOOL_TYPE_SERVICE_CHAIN);
	mpool_destroy(&mpool);
	EXPECT_EQ(mpool, nullptr);

	OSAL_LOG_INFO("=== verify valid pool type - service chain entry");
	mpool_type = MPOOL_TYPE_SERVICE_CHAIN_ENTRY;
	err = mpool_create(mpool_type, num_objects, object_size,
			align_size, &mpool);
	EXPECT_NE(err, EINVAL);
	EXPECT_NE(mpool, nullptr);
	EXPECT_EQ(mpool->mp_config.mpc_type, MPOOL_TYPE_SERVICE_CHAIN_ENTRY);
	mpool_destroy(&mpool);
	EXPECT_EQ(mpool, nullptr);

	OSAL_LOG_DEBUG("exit!");
}

static void
ut_mpool_create_num_objects_cpdc(void)
{
	pnso_error_t err;
	enum mem_pool_type mpool_type;
	uint32_t num_objects, object_size, align_size;
	struct mem_pool *mpool;

	OSAL_LOG_DEBUG("enter ...");

	/* use this same setup across UTs */
	mpool_type = MPOOL_TYPE_CPDC_DESC;
	object_size = PNSO_UT_OBJECT_SIZE;
	align_size = PNSO_UT_ALIGN_SIZE;
	mpool = NULL;

	OSAL_LOG_INFO("=== verify valid case");
	num_objects = PNSO_UT_MPOOL_NUM_OBJECTS;
	err = mpool_create(mpool_type, num_objects, object_size,
			align_size, &mpool);
	EXPECT_NE(err, EINVAL);
	EXPECT_NE(mpool, nullptr);
	EXPECT_EQ(mpool->mp_config.mpc_type, MPOOL_TYPE_CPDC_DESC);
	EXPECT_EQ(mpool->mp_config.mpc_num_objects, num_objects);
	mpool_destroy(&mpool);
	EXPECT_EQ(mpool, nullptr);

	OSAL_LOG_DEBUG("exit!");
}

static void
ut_mpool_create_num_objects_cpdc_status(void)
{
	pnso_error_t err;
	enum mem_pool_type mpool_type;
	uint32_t num_objects, object_size, align_size;
	struct mem_pool *mpool;

	OSAL_LOG_DEBUG("enter ...");

	/* use this same setup across UTs */
	mpool_type = MPOOL_TYPE_CPDC_STATUS_DESC;
	object_size = PNSO_UT_OBJECT_SIZE;
	align_size = PNSO_UT_ALIGN_SIZE;
	mpool = NULL;

	OSAL_LOG_INFO("=== verify lower limit");
	num_objects = PNSO_UT_MPOOL_NUM_OBJECTS - 1;
	err = mpool_create(mpool_type, num_objects, object_size,
			align_size, &mpool);
	EXPECT_EQ(err, 0);

	OSAL_LOG_INFO("=== verify upper limit");
	num_objects = PNSO_UT_MPOOL_NUM_OBJECTS + 1;
	err = mpool_create(mpool_type, num_objects, object_size,
			align_size, &mpool);
	EXPECT_EQ(err, 0);

	OSAL_LOG_INFO("=== verify valid case");
	num_objects = PNSO_UT_MPOOL_NUM_OBJECTS;
	err = mpool_create(mpool_type, num_objects, object_size,
			align_size, &mpool);
	EXPECT_NE(err, EINVAL);
	EXPECT_NE(mpool, nullptr);
	EXPECT_EQ(mpool->mp_config.mpc_type, MPOOL_TYPE_CPDC_STATUS_DESC);
	EXPECT_EQ(mpool->mp_config.mpc_num_objects, num_objects);
	mpool_destroy(&mpool);
	EXPECT_EQ(mpool, nullptr);

	OSAL_LOG_DEBUG("exit!");
}

static void
ut_mpool_create_num_objects_cpdc_sgl(void)
{
	pnso_error_t err;
	enum mem_pool_type mpool_type;
	uint32_t num_objects, object_size, align_size;
	struct mem_pool *mpool;

	OSAL_LOG_DEBUG("enter ...");

	/* use this same setup across UTs */
	mpool_type = MPOOL_TYPE_CPDC_SGL;
	object_size = PNSO_UT_OBJECT_SIZE;
	align_size = PNSO_UT_ALIGN_SIZE;
	mpool = NULL;

	OSAL_LOG_INFO("=== verify lower limit");
	num_objects = PNSO_UT_MPOOL_NUM_OBJECTS - 1;
	err = mpool_create(mpool_type, num_objects, object_size,
			align_size, &mpool);
	EXPECT_EQ(err, 0);

	OSAL_LOG_INFO("=== verify upper limit");
	num_objects = (PNSO_UT_MPOOL_NUM_OBJECTS * 2) + 1;
	err = mpool_create(mpool_type, num_objects, object_size,
			align_size, &mpool);
	EXPECT_EQ(err, 0);

	OSAL_LOG_INFO("=== verify valid case");
	num_objects = PNSO_UT_MPOOL_NUM_OBJECTS;
	err = mpool_create(mpool_type, num_objects, object_size,
			align_size, &mpool);
	EXPECT_NE(err, EINVAL);
	EXPECT_NE(mpool, nullptr);
	EXPECT_EQ(mpool->mp_config.mpc_type, MPOOL_TYPE_CPDC_SGL);
	EXPECT_EQ(mpool->mp_config.mpc_num_objects, num_objects);
	mpool_destroy(&mpool);
	EXPECT_EQ(mpool, nullptr);

	OSAL_LOG_DEBUG("exit!");
}

static void
ut_mpool_create_num_objects_xts(void)
{
	pnso_error_t err;
	enum mem_pool_type mpool_type;
	uint32_t num_objects, object_size, align_size;
	struct mem_pool *mpool;

	OSAL_LOG_DEBUG("enter ...");

	/* use this same setup across UTs */
	mpool_type = MPOOL_TYPE_XTS_DESC;
	object_size = PNSO_UT_OBJECT_SIZE;
	align_size = PNSO_UT_ALIGN_SIZE;
	mpool = NULL;

	OSAL_LOG_INFO("=== verify lower limit");
	num_objects = PNSO_UT_MPOOL_NUM_OBJECTS - 1;
	err = mpool_create(mpool_type, num_objects, object_size,
			align_size, &mpool);
	EXPECT_EQ(err, 0);

	OSAL_LOG_INFO("=== verify upper limit");
	num_objects = PNSO_UT_MPOOL_NUM_OBJECTS + 1;
	err = mpool_create(mpool_type, num_objects, object_size,
			align_size, &mpool);
	EXPECT_EQ(err, 0);

	OSAL_LOG_INFO("=== verify valid case");
	num_objects = PNSO_UT_MPOOL_NUM_OBJECTS;
	err = mpool_create(mpool_type, num_objects, object_size,
			align_size, &mpool);
	EXPECT_NE(err, EINVAL);
	EXPECT_NE(mpool, nullptr);
	EXPECT_EQ(mpool->mp_config.mpc_type, MPOOL_TYPE_XTS_DESC);
	EXPECT_EQ(mpool->mp_config.mpc_num_objects, num_objects);
	mpool_destroy(&mpool);
	EXPECT_EQ(mpool, nullptr);

	OSAL_LOG_DEBUG("exit!");
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

	OSAL_LOG_DEBUG("enter ...");

	/* use this same setup across UTs */
	mpool_type = MPOOL_TYPE_CPDC_DESC;
	num_objects = PNSO_UT_MPOOL_NUM_OBJECTS;
	object_size = PNSO_UT_OBJECT_SIZE;
	mpool = NULL;

	OSAL_LOG_INFO("=== verify invalid size (1)");
	align_size = 0;
	err = mpool_create(mpool_type, num_objects, object_size,
			align_size, &mpool);
	EXPECT_EQ(err, EINVAL);

	OSAL_LOG_INFO("=== verify invalid size (2)");
	align_size = 7;
	err = mpool_create(mpool_type, num_objects, object_size,
			align_size, &mpool);
	EXPECT_EQ(err, EINVAL);

	OSAL_LOG_INFO("=== verify byte aligned");
	align_size = 1;
	err = mpool_create(mpool_type, num_objects, object_size,
			align_size, &mpool);
	EXPECT_NE(err, EINVAL);
	EXPECT_NE(mpool, nullptr);
	EXPECT_EQ(mpool->mp_config.mpc_type, MPOOL_TYPE_CPDC_DESC);
	EXPECT_EQ(mpool->mp_config.mpc_align_size, align_size);
	EXPECT_EQ(mpool->mp_config.mpc_pad_size, 0);
	mpool_destroy(&mpool);
	EXPECT_EQ(mpool, nullptr);

	OSAL_LOG_INFO("=== verify 4K page aligned");
	align_size = PNSO_MEM_ALIGN_PAGE;
	err = mpool_create(mpool_type, num_objects, object_size,
			align_size, &mpool);
	EXPECT_NE(err, EINVAL);
	EXPECT_NE(mpool, nullptr);
	EXPECT_EQ(mpool->mp_config.mpc_type, MPOOL_TYPE_CPDC_DESC);
	EXPECT_EQ(mpool->mp_config.mpc_align_size, align_size);
	EXPECT_EQ((mpool->mp_config.mpc_object_size +
			mpool->mp_config.mpc_pad_size), align_size);
	mpool_destroy(&mpool);
	EXPECT_EQ(mpool, nullptr);

	OSAL_LOG_INFO("=== verify 4K page aligned with odd sized object");
	object_size = PNSO_UT_OBJECT_SIZE - 1;
	align_size = PNSO_MEM_ALIGN_PAGE;
	err = mpool_create(mpool_type, num_objects, object_size,
			align_size, &mpool);
	EXPECT_NE(err, EINVAL);
	EXPECT_NE(mpool, nullptr);
	EXPECT_EQ(mpool->mp_config.mpc_type, MPOOL_TYPE_CPDC_DESC);
	EXPECT_EQ(mpool->mp_config.mpc_align_size, align_size);
	EXPECT_EQ((mpool->mp_config.mpc_object_size +
			mpool->mp_config.mpc_pad_size), align_size);
	mpool_destroy(&mpool);
	EXPECT_EQ(mpool, nullptr);

	OSAL_LOG_DEBUG("exit!");
}

static void
ut_pool_create_object_size(void)
{
	pnso_error_t err;
	enum mem_pool_type mpool_type;
	uint32_t num_objects, object_size, align_size;
	struct mem_pool *mpool;

	OSAL_LOG_DEBUG("enter ...");

	/* use this same setup across UTs */
	mpool_type = MPOOL_TYPE_CPDC_DESC;
	num_objects = PNSO_UT_MPOOL_NUM_OBJECTS;
	align_size = PNSO_UT_ALIGN_SIZE;
	mpool = NULL;

	OSAL_LOG_INFO("=== verify invalid size");
	object_size = 0;
	err = mpool_create(mpool_type, num_objects, object_size,
			align_size, &mpool);
	EXPECT_EQ(err, EINVAL);

	OSAL_LOG_INFO("=== verify byte aligned");
	object_size = 1;
	err = mpool_create(mpool_type, num_objects, object_size,
			align_size, &mpool);
	EXPECT_NE(err, EINVAL);
	EXPECT_NE(mpool, nullptr);
	EXPECT_EQ(mpool->mp_config.mpc_type, MPOOL_TYPE_CPDC_DESC);
	EXPECT_EQ(mpool->mp_config.mpc_object_size, object_size);
	mpool_destroy(&mpool);
	EXPECT_EQ(mpool, nullptr);

	OSAL_LOG_DEBUG("exit!");
}

static void
ut_pool_create_null_pool(void)
{
	pnso_error_t err;
	enum mem_pool_type mpool_type;
	uint32_t num_objects, object_size, align_size;
	struct mem_pool *mpool;

	OSAL_LOG_DEBUG("enter ...");

	/* use this same setup across UTs */
	mpool_type = MPOOL_TYPE_CPDC_DESC;
	num_objects = PNSO_UT_MPOOL_NUM_OBJECTS;
	object_size = 1;
	align_size = PNSO_UT_ALIGN_SIZE;

	OSAL_LOG_INFO("=== verify with NULL pointer for mpool");
	err = mpool_create(mpool_type, num_objects, object_size,
			align_size, NULL);
	EXPECT_EQ(err, EINVAL);

	OSAL_LOG_INFO("=== verify with non-NULL pointer for mpool");
	err = mpool_create(mpool_type, num_objects, object_size,
			align_size, &mpool);
	EXPECT_NE(err, EINVAL);
	EXPECT_NE(mpool, nullptr);
	EXPECT_EQ(mpool->mp_config.mpc_type, MPOOL_TYPE_CPDC_DESC);
	EXPECT_EQ(mpool->mp_config.mpc_object_size, object_size);
	mpool_destroy(&mpool);
	EXPECT_EQ(mpool, nullptr);

	OSAL_LOG_DEBUG("exit!");
}

static void
ut_pool_create_pool_size(void)
{
	pnso_error_t err;
	enum mem_pool_type mpool_type;
	uint32_t num_objects, object_size, align_size, pad_size;
	struct mem_pool *mpool;
	size_t pool_size;

	OSAL_LOG_DEBUG("enter ...");

	/* use this same setup across UTs */
	mpool_type = MPOOL_TYPE_CPDC_DESC;
	num_objects = PNSO_UT_MPOOL_NUM_OBJECTS;

	OSAL_LOG_INFO("=== verify object and alignment size is same");
	object_size = PNSO_UT_OBJECT_SIZE;
	align_size = PNSO_UT_ALIGN_SIZE;
	err = mpool_create(mpool_type, num_objects, object_size,
			align_size, &mpool);
	EXPECT_NE(err, EINVAL);
	EXPECT_NE(mpool, nullptr);
	EXPECT_EQ(mpool->mp_config.mpc_type, MPOOL_TYPE_CPDC_DESC);
	pool_size = num_objects * object_size;
	EXPECT_EQ(mpool->mp_config.mpc_pool_size, pool_size);
	mpool_destroy(&mpool);
	EXPECT_EQ(mpool, nullptr);

	OSAL_LOG_INFO("=== verify object is less than and alignment size");
	object_size = PNSO_UT_OBJECT_SIZE - 1;
	align_size = PNSO_UT_ALIGN_SIZE;
	err = mpool_create(mpool_type, num_objects, object_size,
			align_size, &mpool);
	EXPECT_NE(err, EINVAL);
	EXPECT_NE(mpool, nullptr);
	EXPECT_EQ(mpool->mp_config.mpc_type, MPOOL_TYPE_CPDC_DESC);
	pool_size = num_objects * align_size;
	EXPECT_EQ(mpool->mp_config.mpc_pool_size, pool_size);
	mpool_destroy(&mpool);
	EXPECT_EQ(mpool, nullptr);

	OSAL_LOG_INFO("=== verify object is greater than and alignment size");
	object_size = PNSO_UT_OBJECT_SIZE - 1;
	align_size = PNSO_UT_ALIGN_SIZE;
	err = mpool_create(mpool_type, num_objects, object_size,
			align_size, &mpool);
	EXPECT_NE(err, EINVAL);
	EXPECT_NE(mpool, nullptr);
	EXPECT_EQ(mpool->mp_config.mpc_type, MPOOL_TYPE_CPDC_DESC);
	pad_size = (num_objects * (align_size - (object_size % align_size)));
	pool_size = (num_objects * object_size) + pad_size;
	EXPECT_EQ(mpool->mp_config.mpc_pool_size, pool_size);
	mpool_destroy(&mpool);
	EXPECT_EQ(mpool, nullptr);

	OSAL_LOG_DEBUG("exit!");
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

	OSAL_LOG_DEBUG("enter ...");

	/* use this same setup across UTs */
	mpool_type = MPOOL_TYPE_CPDC_DESC;
	num_objects = PNSO_UT_MPOOL_NUM_OBJECTS;
	object_size = PNSO_UT_OBJECT_SIZE;
	align_size = PNSO_UT_ALIGN_SIZE;

	OSAL_LOG_INFO("=== verify 'destroy' with NULL mpool and "
			"then with a valid one");
	err = mpool_create(mpool_type, num_objects, object_size,
			align_size, &mpool);
	EXPECT_NE(err, EINVAL);
	EXPECT_NE(mpool, nullptr);
	EXPECT_EQ(mpool->mp_config.mpc_type, MPOOL_TYPE_CPDC_DESC);
	mpool_destroy(NULL);
	mpool_destroy(&mpool);
	EXPECT_EQ(mpool, nullptr);

	OSAL_LOG_DEBUG("exit!");
}

TEST_F(pnso_mpool_test, ut_mpool_get_object) {
	pnso_error_t err;
	enum mem_pool_type mpool_type;
	uint32_t num_objects, object_size, align_size, i;
	struct mem_pool *mpool;
	void *p, *q;

	OSAL_LOG_DEBUG("enter ...");

	/* use this same setup across UTs */
	mpool_type = MPOOL_TYPE_CPDC_DESC;
	num_objects = PNSO_UT_MPOOL_NUM_OBJECTS;

	object_size = PNSO_UT_OBJECT_SIZE;
	align_size = PNSO_UT_ALIGN_SIZE;

	OSAL_LOG_INFO("=== verify 'get' with NULL object");
	err = mpool_create(mpool_type, num_objects, object_size,
			align_size, &mpool);
	EXPECT_NE(err, EINVAL);
	EXPECT_NE(mpool, nullptr);
	EXPECT_EQ(mpool->mp_config.mpc_type, MPOOL_TYPE_CPDC_DESC);

	p = mpool_get_object(NULL);
	EXPECT_EQ(p, nullptr);
	mpool_destroy(&mpool);
	EXPECT_EQ(mpool, nullptr);

	OSAL_LOG_INFO("=== verify 'get' with non-NULL object");
	err = mpool_create(mpool_type, num_objects, object_size,
			align_size, &mpool);
	EXPECT_NE(err, EINVAL);
	EXPECT_NE(mpool, nullptr);
	EXPECT_EQ(mpool->mp_config.mpc_type, MPOOL_TYPE_CPDC_DESC);

	p = mpool_get_object(mpool);
	EXPECT_NE(p, nullptr);
	mpool_destroy(&mpool);
	EXPECT_EQ(mpool, nullptr);

	OSAL_LOG_INFO("=== verify 'get' and check for alignment (1)");
	object_size = PNSO_UT_OBJECT_SIZE;
	align_size = PNSO_UT_ALIGN_SIZE;
	err = mpool_create(mpool_type, num_objects, object_size,
			align_size, &mpool);
	EXPECT_NE(err, EINVAL);
	EXPECT_NE(mpool, nullptr);
	EXPECT_EQ(mpool->mp_config.mpc_type, MPOOL_TYPE_CPDC_DESC);

	p = mpool_get_object(mpool);
	EXPECT_NE(p, nullptr);
	EXPECT_EQ(((uintptr_t) p % PNSO_UT_ALIGN_SIZE), 0);
	q = mpool_get_object(mpool);
	EXPECT_NE(q, nullptr);
	EXPECT_EQ(((uintptr_t) q % PNSO_UT_ALIGN_SIZE), 0);

	mpool_destroy(&mpool);
	EXPECT_EQ(mpool, nullptr);

	OSAL_LOG_INFO("=== verify 'get' and check for alignment (2)");
	object_size = PNSO_UT_OBJECT_SIZE - 1;
	align_size = PNSO_UT_ALIGN_SIZE;
	err = mpool_create(mpool_type, num_objects, object_size,
			align_size, &mpool);
	EXPECT_NE(err, EINVAL);
	EXPECT_NE(mpool, nullptr);
	EXPECT_EQ(mpool->mp_config.mpc_type, MPOOL_TYPE_CPDC_DESC);

	p = mpool_get_object(mpool);
	EXPECT_NE(p, nullptr);
	EXPECT_EQ(((uintptr_t) p % PNSO_UT_ALIGN_SIZE), 0);
	q = mpool_get_object(mpool);
	EXPECT_NE(q, nullptr);
	EXPECT_EQ(((uintptr_t) q % PNSO_UT_ALIGN_SIZE), 0);

	mpool_destroy(&mpool);
	EXPECT_EQ(mpool, nullptr);

	OSAL_LOG_INFO("=== verify 'get' and check for alignment (3)");
	object_size = PNSO_UT_OBJECT_SIZE + 1;
	align_size = PNSO_UT_ALIGN_SIZE;
	err = mpool_create(mpool_type, num_objects, object_size,
			align_size, &mpool);
	EXPECT_NE(err, EINVAL);
	EXPECT_NE(mpool, nullptr);
	EXPECT_EQ(mpool->mp_config.mpc_type, MPOOL_TYPE_CPDC_DESC);

	p = mpool_get_object(mpool);
	EXPECT_NE(p, nullptr);
	EXPECT_EQ(((uintptr_t) p % PNSO_UT_ALIGN_SIZE), 0);
	q = mpool_get_object(mpool);
	EXPECT_NE(q, nullptr);
	EXPECT_EQ(((uintptr_t) q % PNSO_UT_ALIGN_SIZE), 0);

	mpool_destroy(&mpool);
	EXPECT_EQ(mpool, nullptr);

	OSAL_LOG_INFO("=== verify 'get, put, get' with non-NULL object");
	err = mpool_create(mpool_type, num_objects, object_size,
			align_size, &mpool);
	EXPECT_NE(err, EINVAL);
	EXPECT_NE(mpool, nullptr);
	EXPECT_EQ(mpool->mp_config.mpc_type, MPOOL_TYPE_CPDC_DESC);

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

	OSAL_LOG_INFO("=== verify pop all and then a 'get'; "
			"push the spare one, then 'get'");
	err = mpool_create(mpool_type, num_objects, object_size,
			align_size, &mpool);
	EXPECT_NE(err, EINVAL);
	EXPECT_NE(mpool, nullptr);
	EXPECT_EQ(mpool->mp_config.mpc_type, MPOOL_TYPE_CPDC_DESC);

	for (i = 0; i < num_objects; i++) {
		p = mpool_get_object(mpool);
		EXPECT_NE(p, nullptr);
		q = p;
	}
	p = mpool_get_object(mpool);
	EXPECT_EQ(p, nullptr);

	err = mpool_put_object(mpool, q);
	EXPECT_EQ(err, PNSO_OK);
	p = mpool_get_object(mpool);
	EXPECT_NE(p, nullptr);

	mpool_destroy(&mpool);
	EXPECT_EQ(mpool, nullptr);

	OSAL_LOG_DEBUG("exit!");
}

TEST_F(pnso_mpool_test, ut_mpool_put_object) {
	pnso_error_t err;
	enum mem_pool_type mpool_type;
	uint32_t num_objects, object_size, align_size;
	struct mem_pool *mpool;
	void *p;

	OSAL_LOG_DEBUG("enter ...");

	/* use this same setup across UTs */
	mpool_type = MPOOL_TYPE_CPDC_DESC;
	num_objects = PNSO_UT_MPOOL_NUM_OBJECTS;
	object_size = PNSO_UT_OBJECT_SIZE;
	align_size = PNSO_UT_ALIGN_SIZE;

	OSAL_LOG_INFO("=== verify 'put' with NULL pool and then NULL object");
	err = mpool_create(mpool_type, num_objects, object_size,
			align_size, &mpool);
	EXPECT_NE(err, EINVAL);
	EXPECT_NE(mpool, nullptr);
	EXPECT_EQ(mpool->mp_config.mpc_type, MPOOL_TYPE_CPDC_DESC);
	p = mpool_get_object(NULL);
	EXPECT_EQ(p, nullptr);

	err = mpool_put_object(NULL, p);
	EXPECT_EQ(err, EINVAL);
	err = mpool_put_object(mpool, NULL);
	EXPECT_EQ(err, EINVAL);

	mpool_destroy(&mpool);
	EXPECT_EQ(mpool, nullptr);

	OSAL_LOG_INFO("=== verify 'get, put' with non-NULL object");
	err = mpool_create(mpool_type, num_objects, object_size,
			align_size, &mpool);
	EXPECT_NE(err, EINVAL);
	EXPECT_NE(mpool, nullptr);
	EXPECT_EQ(mpool->mp_config.mpc_type, MPOOL_TYPE_CPDC_DESC);

	p = mpool_get_object(mpool);
	EXPECT_NE(p, nullptr);
	err = mpool_put_object(mpool, p);
	EXPECT_EQ(err, PNSO_OK);

	mpool_destroy(&mpool);
	EXPECT_EQ(mpool, nullptr);

	OSAL_LOG_INFO("=== verify 'put' with non-NULL object on a full stack");
	err = mpool_create(mpool_type, num_objects, object_size,
			align_size, &mpool);
	EXPECT_NE(err, EINVAL);
	EXPECT_NE(mpool, nullptr);
	EXPECT_EQ(mpool->mp_config.mpc_type, MPOOL_TYPE_CPDC_DESC);

	err = mpool_put_object(mpool, p);
	EXPECT_EQ(err, ENOTEMPTY);

	mpool_destroy(&mpool);
	EXPECT_EQ(mpool, nullptr);

	OSAL_LOG_DEBUG("exit!");
}

int main(int argc, char **argv) {
    int ret;
    osal_log_init(OSAL_LOG_LEVEL_INFO);
    ::testing::InitGoogleTest(&argc, argv);
    ret = RUN_ALL_TESTS();
    osal_log_deinit();
    return ret;
}
