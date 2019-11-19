
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/module.h>
#include "pnso_test.h"
#include "pnso_test_ctx.h"
#include "pnso_test_sysfs.h"
#include "osal_atomic.h"
#include "pnso_test_dev.h"

static const char *ctl_state_strs[CTL_STATE_MAX+1] = {
	[CTL_STATE_READ] = "read",
	[CTL_STATE_START] = "start",
	[CTL_STATE_REPEAT] = "repeat",
	[CTL_STATE_STOP] = "stop",
	[CTL_STATE_MAX] = "invalid"
};

static osal_atomic_int_t ctl_state;
static bool kobj_pencake_created;
static struct kobject *kobj_pencake;
static struct kobject *kobj_pencake_cfg;
static struct kobject *kobj_pencake_status;
static osal_atomic_int_t cfg_lock;
static osal_atomic_int_t status_lock;

#define MAX_SYSFS_DATA_LEN 4096
struct pencake_sysfs_data_node {
	struct kobj_attribute kattr;
	uint32_t id;
	osal_atomic_int_t *osal_lock;
	uint32_t data_len;
	char data[MAX_SYSFS_DATA_LEN];
};

struct pencake_sysfs_stat_node {
	struct kobj_attribute kattr;
	osal_atomic_int_t stat;
};

#define MAX_SYSFS_CFG_NODES 10
#define MAX_SYSFS_STATUS_NODES 10
static struct pencake_sysfs_data_node cfg_nodes[MAX_SYSFS_CFG_NODES];
static struct pencake_sysfs_data_node status_nodes[MAX_SYSFS_STATUS_NODES];
static struct attribute *cfg_attrs[MAX_SYSFS_CFG_NODES+1];
static struct attribute *status_attrs[MAX_SYSFS_STATUS_NODES+1];
//ATTRIBUTE_GROUPS(cfg);
//ATTRIBUTE_GROUPS(status);
static struct attribute_group cfg_group = {
	.attrs = cfg_attrs
};
static struct attribute_group status_group = {
	.attrs = status_attrs
};

static struct pencake_sysfs_stat_node status_cur_node;
static struct pencake_sysfs_stat_node status_success_node;
static struct pencake_sysfs_stat_node status_fail_node;
static struct pencake_sysfs_stat_node status_fail_id_node;

/* null separated list of sysfs node names, for basic string storage */
static size_t node_names_i;
static char node_names[1024];

#define PENCAKE_INIT_DATA_NODE(_node, _id, _mode, _show_fn, _store_fn, _lock) \
	(_node)->kattr.attr.name = node_names + node_names_i; \
	if (_id < 10) \
		node_names[node_names_i++] = '0' + _id; \
	else \
		node_names[node_names_i++] = 'a' + _id; \
	node_names[node_names_i++] = '\0'; \
	(_node)->kattr.attr.mode = _mode;   \
	(_node)->kattr.show = _show_fn;	   \
	(_node)->kattr.store = _store_fn; \
	(_node)->id = _id; \
	(_node)->data_len = 0; \
	(_node)->osal_lock = _lock

#define PENCAKE_INIT_STAT_NODE(_node, _name, _mode, _show_fn, _store_fn)	\
	(_node)->kattr.attr.name = node_names + node_names_i;			\
	node_names_i += 1 + safe_strcpy(node_names + node_names_i, \
			_name, sizeof(node_names) - node_names_i); \
	(_node)->kattr.attr.mode = _mode;			   \
	(_node)->kattr.show = _show_fn; \
	(_node)->kattr.store = _store_fn; \
	osal_atomic_init(&(_node)->stat, 0)


static uint32_t read_sysfs_data(struct pencake_sysfs_data_node *node, char *dst)
{
	uint32_t ret = 0;

	osal_atomic_lock(node->osal_lock);
	ret = node->data_len;
	if (node->data_len) {
		memcpy(dst, node->data, node->data_len);
		ret = node->data_len;
	}
	osal_atomic_unlock(node->osal_lock);

	return ret;
}

static void write_sysfs_data(struct pencake_sysfs_data_node *node, const char *src,
			     uint32_t len)
{
	if (len > MAX_SYSFS_DATA_LEN)
		len = MAX_SYSFS_DATA_LEN;

	osal_atomic_lock(node->osal_lock);
	if (len)
		memcpy(node->data, src, len);
	node->data_len = len;
	osal_atomic_unlock(node->osal_lock);
}

static void write_testcase_summary(uint32_t testcase_id, bool success)
{
	if (success) {
		osal_atomic_fetch_add(&status_success_node.stat, 1);
		pnso_test_inc_success_cnt();
	} else {
		osal_atomic_fetch_add(&status_fail_node.stat, 1);
		osal_atomic_set(&status_fail_id_node.stat, (int) testcase_id);
		pnso_test_inc_fail_cnt();
	}
}

void pnso_test_sysfs_write_status_data(const char *src, uint32_t len, void *opaque)
{
	struct pencake_sysfs_data_node *node;
	struct testcase_context *test_ctx = (struct testcase_context *) opaque;
	uint32_t test_id = test_ctx ? test_ctx->testcase->node.idx : 0;
	uint32_t cur;

	cur = test_id % MAX_SYSFS_STATUS_NODES;
	osal_atomic_set(&status_cur_node.stat, (int) cur);
	node = &status_nodes[cur];

	write_sysfs_data(node, src, len);
	if (test_ctx) {
		write_testcase_summary(test_ctx->testcase->node.idx,
			test_ctx->stats.agg_stats.validation_failures == 0);
	}
}

static ssize_t data_sysfs_show(struct kobject *kobj,
				 struct kobj_attribute *attr,
				 char *buf)
{
	struct pencake_sysfs_data_node *node = container_of(attr,
			struct pencake_sysfs_data_node, kattr);
	ssize_t ret;

	ret = (ssize_t) read_sysfs_data(node, buf);
	PNSO_LOG_DEBUG("Sysfs show %u bytes from '%s'\n",
		       (uint32_t) ret, attr->attr.name);

	return ret;
}

static ssize_t data_sysfs_store(struct kobject *kobj,
				  struct kobj_attribute *attr,
				  const char *buf,
				  size_t count)
{
	struct pencake_sysfs_data_node *node = container_of(attr,
			struct pencake_sysfs_data_node, kattr);

	write_sysfs_data(node, buf, count);
	PNSO_LOG_DEBUG("Sysfs store %u bytes to '%s'\n",
		       (uint32_t) count, attr->attr.name);
	return count;
}


static ssize_t ctl_sysfs_show(struct kobject *kobj,
			      struct kobj_attribute *attr,
			      char *buf)
{
	ssize_t ret;
	int state;

	state = osal_atomic_read(&ctl_state);
	if (state < 0 || state > CTL_STATE_MAX)
		state = CTL_STATE_MAX;

	ret = (ssize_t) safe_strcpy(buf, ctl_state_strs[state], 16);

	PNSO_LOG_DEBUG("Sysfs show ctl_state %s\n", buf);

	return ret;
}

static ssize_t ctl_sysfs_store(struct kobject *kobj,
			       struct kobj_attribute *attr,
			       const char *buf,
			       size_t count)
{
	int i;

	for (i = 0; i < CTL_STATE_MAX; i++) {
		if (0 == safe_strncmp(buf, ctl_state_strs[i],
				      strlen(ctl_state_strs[i])))
			break;
	}
	osal_atomic_set(&ctl_state, i);

	PNSO_LOG_DEBUG("Sysfs store ctl_state %s\n", ctl_state_strs[i]);
	return count;
}

static ssize_t stat_sysfs_show(struct kobject *kobj,
			       struct kobj_attribute *attr,
			       char *buf)
{
	struct pencake_sysfs_stat_node *node = container_of(attr,
			struct pencake_sysfs_stat_node, kattr);
	ssize_t ret;
	int val;

	val = osal_atomic_read(&node->stat);

	ret = (ssize_t) safe_itoa(buf, MAX_SYSFS_DATA_LEN, val);
	buf[ret++] = '\n';
	buf[ret] = '\0';

	PNSO_LOG_DEBUG("Sysfs show stat %s = %s\n",
		       attr->attr.name, buf);

	return ret;
}

static struct kobj_attribute ctl_attr = __ATTR(ctl, 0644, ctl_sysfs_show, ctl_sysfs_store);

int pnso_test_sysfs_init(void)
{
	int err = EINVAL;
	uint32_t i;

	osal_atomic_init(&cfg_lock, 0);
	osal_atomic_init(&status_lock, 0);

	/* Get module root */
#ifdef MODULE
	kobj_pencake = &(((struct module *)(THIS_MODULE))->mkobj).kobj;
	if (!kobj_pencake)
		goto done;
	kobj_pencake_created = false;
#else
	kobj_pencake = kobject_create_and_add("pencake", NULL);
	if (!kobj_pencake)
		goto done;
	kobj_pencake_created = true;
#endif

	/* Create subdirectories */
	kobj_pencake_cfg = kobject_create_and_add("cfg", kobj_pencake);
	if (!kobj_pencake_cfg)
		goto done;

	kobj_pencake_status = kobject_create_and_add("status", kobj_pencake);
	if (!kobj_pencake_status)
		goto done;

	/* Create cfg files */
	if ((err = sysfs_create_file(kobj_pencake_cfg, &ctl_attr.attr)))
		goto done;
	for (i = 0; i < MAX_SYSFS_CFG_NODES; i++) {
		PENCAKE_INIT_DATA_NODE(&cfg_nodes[i], i, 0644,
			data_sysfs_show, data_sysfs_store, &cfg_lock);

		cfg_attrs[i] = &cfg_nodes[i].kattr.attr;
	}
	cfg_attrs[MAX_SYSFS_CFG_NODES] = NULL;
	if ((err = sysfs_create_group(kobj_pencake_cfg, &cfg_group)))
		goto done;

	/* Create status files */
	PENCAKE_INIT_STAT_NODE(&status_cur_node, "cur", 0444, stat_sysfs_show, NULL);
	if ((err = sysfs_create_file(kobj_pencake_status, &status_cur_node.kattr.attr)))
		goto done;
	PENCAKE_INIT_STAT_NODE(&status_success_node, "success", 0444, stat_sysfs_show, NULL);
	if ((err = sysfs_create_file(kobj_pencake_status, &status_success_node.kattr.attr)))
		goto done;
	PENCAKE_INIT_STAT_NODE(&status_fail_node, "fail", 0444, stat_sysfs_show, NULL);
	if ((err = sysfs_create_file(kobj_pencake_status, &status_fail_node.kattr.attr)))
		goto done;
	PENCAKE_INIT_STAT_NODE(&status_fail_id_node, "fail_id", 0444, stat_sysfs_show, NULL);
	if ((err = sysfs_create_file(kobj_pencake_status, &status_fail_id_node.kattr.attr)))
		goto done;
	for (i = 0; i < MAX_SYSFS_STATUS_NODES; i++) {
		/* Status is read only */
		PENCAKE_INIT_DATA_NODE(&status_nodes[i], i, 0444,
			data_sysfs_show, NULL, &status_lock);

		status_attrs[i] = &status_nodes[i].kattr.attr;
	}
	status_attrs[MAX_SYSFS_STATUS_NODES] = NULL;
	if ((err = sysfs_create_group(kobj_pencake_status, &status_group)))
		goto done;

	err = 0;
done:
	return err;
}

void pnso_test_sysfs_finit(void)
{
	if (kobj_pencake_status) {
		sysfs_remove_file(kobj_pencake_status, &status_cur_node.kattr.attr);
		sysfs_remove_file(kobj_pencake_status, &status_success_node.kattr.attr);
		sysfs_remove_file(kobj_pencake_status, &status_fail_node.kattr.attr);
		sysfs_remove_file(kobj_pencake_status, &status_fail_id_node.kattr.attr);
		sysfs_remove_group(kobj_pencake_status, &status_group);
		kobject_put(kobj_pencake_status);
		kobj_pencake_status = NULL;
	}
	if (kobj_pencake_cfg) {
		sysfs_remove_file(kobj_pencake_cfg, &ctl_attr.attr);
		sysfs_remove_group(kobj_pencake_cfg, &cfg_group);
		kobject_put(kobj_pencake_cfg);
		kobj_pencake_cfg = NULL;
	}
	if (kobj_pencake) {
		if (kobj_pencake_created)
			kobject_put(kobj_pencake);
		kobj_pencake = NULL;
	}
}

char *pnso_test_sysfs_alloc_and_get_cfg(void)
{
	char *dst;
	struct pencake_sysfs_data_node *node;
	uint32_t len = 0;
	uint32_t max_len = MAX_SYSFS_DATA_LEN * MAX_SYSFS_CFG_NODES;
	uint32_t i;

	dst = TEST_ALLOC(max_len);
	if (!dst)
		return NULL;

	osal_atomic_lock(&cfg_lock);
	for (i = 0; i < MAX_SYSFS_CFG_NODES; i++) {
		node = &cfg_nodes[i];
		if (node->data_len) {
			memcpy(dst+len, node->data, node->data_len);
			len += node->data_len;
		}
	}
	osal_atomic_unlock(&cfg_lock);

	if (!len) {
		TEST_FREE(dst);
		return NULL;
	}
	if (len >= max_len)
		len = max_len - 1;
	dst[len] = 0;
	return dst;
}

int pnso_test_sysfs_read_ctl(void)
{
	return osal_atomic_exchange(&ctl_state, CTL_STATE_READ);
}

/* These functions are only supported on FreeBSD for now.  Stub them. */

bool pnso_test_sysfs_is_fd(const char *devname, int *fdnum)
{
	*fdnum = -1;
	return false;
}

uint32_t pnso_test_sysfs_write_fd(int fdnum, uint32_t offset,
		const uint8_t *src, uint32_t size, uint32_t *gen_id)
{
	*gen_id = 0;
	return 0;
}

uint32_t pnso_test_sysfs_read_fd(int fdnum, uint32_t offset, uint8_t *dst,
				 uint32_t size, uint32_t *gen_id)
{
	*gen_id = 0;
	return 0;
}

int pnso_test_sysfs_get_fd_data(int fdnum,
				struct pnso_flat_buffer *flat_buf,
				uint32_t *gen_id)
{
	*gen_id = 0;
	return -1;
}

uint32_t pnso_test_sysfs_get_fd_len(int fdnum, uint32_t *gen_id)
{
	*gen_id = 0;
	return 0;
}
