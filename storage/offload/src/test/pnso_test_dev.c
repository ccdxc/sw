#include <sys/types.h>
#include <sys/param.h>  /* defines used in kernel.h */
#include <sys/kernel.h> /* types used in module initialization */
#include <sys/conf.h>   /* cdevsw struct */
#include <sys/uio.h>    /* uio struct */
#include <sys/malloc.h>
#include <sys/fcntl.h>
#include "pnso_test.h"
#include "pnso_test_ctx.h"
#include "pnso_test_sysfs.h"
#include "pnso_test_dev.h"

static d_open_t pnso_test_open;
static d_close_t pnso_test_close;
static d_read_t pnso_test_read;
static d_write_t pnso_test_write;

#define pnso_test_open_fd pnso_test_open
static d_close_t pnso_test_close_fd;
static d_read_t pnso_test_read_fd;
#define pnso_test_write_fd pnso_test_write

#define DEVNAME "pencake"
#define DEV_FD_NAME_PREFIX "pencakefd"
#define DEV_FD_NAME_PREFIXLEN (sizeof(DEV_FD_NAME_PREFIX) - 1)
#define DEV_FD_NAME_FULL_PREFIX "/dev/pencakefd"
#define DEV_FD_NAME_FULL_PREFIXLEN (sizeof(DEV_FD_NAME_FULL_PREFIX) - 1)

#define DEV_FD_COUNT 10
#define DEV_FD_NAME(n) DEV_FD_NAME_PREFIX #n

/* Character device entry points */
static struct cdevsw pnso_test_cdevsw = {
	.d_version = D_VERSION,
	.d_open = pnso_test_open,
	.d_close = pnso_test_close,
	.d_read = pnso_test_read,
	.d_write = pnso_test_write,
	.d_name = DEVNAME,
};
#define BUFFERSIZE (128*1024)
#define FD_BUFFERSIZE (64*1024)

#define DECLARE_FD_CDEVSW(n) { \
	.d_version = D_VERSION, \
	.d_open = pnso_test_open_fd, \
	.d_close = pnso_test_close_fd, \
	.d_read = pnso_test_read_fd, \
	.d_write = pnso_test_write_fd, \
	.d_name = DEV_FD_NAME(n) }

static struct cdevsw pnso_test_fd_cdevsw[DEV_FD_COUNT] = {
	DECLARE_FD_CDEVSW(0),
	DECLARE_FD_CDEVSW(1),
	DECLARE_FD_CDEVSW(2),
	DECLARE_FD_CDEVSW(3),
	DECLARE_FD_CDEVSW(4),
	DECLARE_FD_CDEVSW(5),
	DECLARE_FD_CDEVSW(6),
	DECLARE_FD_CDEVSW(7),
	DECLARE_FD_CDEVSW(8),
	DECLARE_FD_CDEVSW(9)
};

struct test_softc_buffer {
	char *buf;
	int maxlen;
	osal_atomic_int_t atomic_len;
	osal_atomic_int_t atomic_gen_id;
};

/*
 * XXX: Fix me for multi device support using device softc.
 */
struct test_softc {
	struct test_softc_buffer input;
	struct test_softc_buffer output;
};

static struct test_softc pnso_test_softc;
static struct test_softc pnso_test_fd_softc[DEV_FD_COUNT];

static struct cdev *pnso_test_dev;
static struct cdev *pnso_test_fd_dev[DEV_FD_COUNT];

static osal_atomic_int_t ctl_state;

static struct test_softc *pnso_test_fdnum_to_softc(int fdnum)
{
	if (fdnum < 0 || fdnum >= DEV_FD_COUNT)
		return NULL;

	return &pnso_test_fd_softc[fdnum];
}

bool pnso_test_sysfs_is_fd(const char *devname, int *fdnum)
{
	char c;

	if (0 == strncmp(devname, DEV_FD_NAME_FULL_PREFIX,
			 DEV_FD_NAME_FULL_PREFIXLEN)) {
		c = devname[DEV_FD_NAME_FULL_PREFIXLEN];
		if (isdigit(c)) {
			*fdnum = (int) (c - '0');
			return true;
		}
	}

	*fdnum = -1;
	return false;
}

static struct test_softc *pnso_test_devname_to_softc(const char *name)
{
	if (0 == strcmp(name, DEVNAME))
		return &pnso_test_softc;

	if (0 == strncmp(name, DEV_FD_NAME_PREFIX, DEV_FD_NAME_PREFIXLEN)) {
		if (isdigit(name[DEV_FD_NAME_PREFIXLEN])) {
			return pnso_test_fdnum_to_softc(name[DEV_FD_NAME_PREFIXLEN] - '0');
		}
	}

	return NULL;
}

static void
pnso_test_cdev_clear_one(struct test_softc *softc)
{
	osal_atomic_set(&softc->input.atomic_len, 0);
	osal_atomic_set(&softc->output.atomic_len, 0);
	osal_atomic_fetch_add(&softc->input.atomic_gen_id, 1);
	osal_atomic_fetch_add(&softc->output.atomic_gen_id, 1);
}

static int
pnso_test_cdev_init_one(struct test_softc *softc, struct cdev **dev,
			struct cdevsw *sw,
			uint32_t in_bufsize, uint32_t out_bufsize)
{
	softc->input.maxlen = in_bufsize - 1;
	softc->output.maxlen = out_bufsize - 1;

        softc->input.buf = TEST_ALLOC(in_bufsize);
        softc->output.buf = TEST_ALLOC(out_bufsize);
	if ((softc->input.buf == NULL) || (softc->output.buf == NULL)) {
		PNSO_LOG_ERROR("Init device pnso_test alloc failed\n");
		return (EINVAL);
	}
	pnso_test_cdev_clear_one(softc);

	return make_dev_p(MAKEDEV_CHECKNAME | MAKEDEV_WAITOK,
			  dev,
			  sw,
			  0,
			  UID_ROOT,
			  GID_WHEEL,
			  0600,
			  "%s", sw->d_name);
}

int
pnso_test_cdev_init(void)
{
	int i, ret;

	ret = pnso_test_cdev_init_one(&pnso_test_softc,
				      &pnso_test_dev, &pnso_test_cdevsw,
				      BUFFERSIZE, BUFFERSIZE);
	if (ret) return ret;

	for (i = 0; i < DEV_FD_COUNT; i++) {
		ret = pnso_test_cdev_init_one(&pnso_test_fd_softc[i],
				&pnso_test_fd_dev[i], &pnso_test_fd_cdevsw[i],
				FD_BUFFERSIZE, 1);
		if (ret) return ret;
	}

	return 0;
}

static void
pnso_test_cdev_deinit_one(struct test_softc *softc,
			  struct cdev *dev)
{
	PNSO_LOG_DEBUG("Deinit device %s\n", dev && dev->si_devsw ?
		       dev->si_devsw->d_name : "unkown");
	
	if (dev)
		destroy_dev(dev);

	pnso_test_cdev_clear_one(softc);

	softc->input.maxlen = 0;
	softc->output.maxlen = 0;

	if (softc->input.buf) {
		TEST_FREE(softc->input.buf);
		softc->input.buf = NULL;
	}
	if (softc->output.buf) {
		TEST_FREE(softc->output.buf);
		softc->output.buf = NULL;
	}
}

void
pnso_test_cdev_deinit(void)
{
	int i;

	PNSO_LOG_DEBUG("Deinit device pnso_test\n");

	pnso_test_cdev_deinit_one(&pnso_test_softc, pnso_test_dev);
	pnso_test_dev = NULL;

	for (i = 0; i < DEV_FD_COUNT; i++) {
		pnso_test_cdev_deinit_one(&pnso_test_fd_softc[i], pnso_test_fd_dev[i]);
		pnso_test_fd_dev[i] = NULL;
	}
}

static int
pnso_test_open(struct cdev *dev __unused, int oflags, int devtype __unused,
    struct thread *td __unused)
{
	if (oflags & FWRITE) {
		PNSO_LOG_DEBUG("Opened write device pnso_test successfully.\n");
	} else {
		PNSO_LOG_DEBUG("Opened read device pnso_test successfully.\n");
	}

	return (0);
}

static int
pnso_test_close(struct cdev *dev, int fflag, int devtype __unused,
    struct thread *td __unused)
{
	if (fflag & FWRITE) {
		PNSO_LOG_DEBUG("Closing write device pnso_test.\n");
		/* Start the test. */
		osal_atomic_set(&ctl_state, CTL_STATE_START);
	} else {
		PNSO_LOG_DEBUG("Closing read device pnso_test.\n");
	}

	return (0);
}

static int
pnso_test_close_fd(struct cdev *dev, int fflag, int devtype __unused,
    struct thread *td __unused)
{
	if (fflag & FWRITE) {
		PNSO_LOG_DEBUG("Closing write device pnso_test_fd.\n");
	} else {
		PNSO_LOG_DEBUG("Closing read device pnso_test_fd.\n");
	}

	return (0);
}

/* Return error code */
static int
copy_dev_to_uspace(struct test_softc_buffer *dev_buffer, struct uio *uio)
{
	size_t amt;
	int error;
	int dev_len;

	if (dev_buffer->buf == NULL)
		return (ENXIO);

	dev_len = osal_atomic_read(&dev_buffer->atomic_len);

	/*
	 * How big is this read operation?  Either as big as the user wants,
	 * or as big as the remaining data.  Note that the 'input_len' does not
	 * include the trailing null character.
	 */
	amt = MIN(uio->uio_resid, uio->uio_offset >= dev_len ? 0 :
		  dev_len - uio->uio_offset);

	error = uiomove(dev_buffer->buf + uio->uio_offset, amt, uio);

	if (error != 0)
		PNSO_LOG_ERROR("uiomove failed, err %d!\n", error);

	return (error);

}

static int
pnso_test_read(struct cdev *dev, struct uio *uio, int ioflag __unused)
{
	struct cdevsw *devsw = dev->si_devsw;
	struct test_softc *softc;

	if (!devsw) return (ENXIO);

	softc = pnso_test_devname_to_softc(devsw->d_name);
	if (!softc) return (ENXIO);

	PNSO_LOG_DEBUG("Reading from device %s, offset %lu, len %lu.\n",
		       devsw->d_name, uio->uio_offset, uio->uio_resid);

	return copy_dev_to_uspace(&softc->output, uio);
}

static int
pnso_test_read_fd(struct cdev *dev, struct uio *uio, int ioflag __unused)
{
	struct cdevsw *devsw = dev->si_devsw;
	struct test_softc *softc;

	if (!devsw) return (ENXIO);

	softc = pnso_test_devname_to_softc(devsw->d_name);
	if (!softc) return (ENXIO);

	PNSO_LOG_DEBUG("Reading from device %s, offset %lu, len %lu.\n",
		       devsw->d_name, uio->uio_offset, uio->uio_resid);

	return copy_dev_to_uspace(&softc->input, uio);
}

static int
copy_uspace_to_dev(struct test_softc_buffer *dev_buffer, struct uio *uio)
{
	size_t amt;
	int error;
	int dev_len;

	if (dev_buffer->buf == NULL)
		return (ENXIO);

	dev_len = osal_atomic_read(&dev_buffer->atomic_len);

	/*
	 * We either write from the beginning or from valid data --
         * do not allow gaps.
	 */
	if (uio->uio_offset > dev_len) {
		PNSO_LOG_ERROR("Invalid uio_offset %lu\n", uio->uio_offset);
		return (EINVAL);
	}
	if (uio->uio_offset >= dev_buffer->maxlen) {
		PNSO_LOG_ERROR("Write failed: passed end of buffer, uio_offset %lu\n",
			       uio->uio_offset);
		return (EINVAL);
	}

	/* Copy the string in from user memory to kernel memory */
	amt = MIN(uio->uio_resid, (dev_buffer->maxlen - dev_len));

	error = uiomove(dev_buffer->buf + uio->uio_offset, amt, uio);

	/* Now we need to record the length */
	dev_len = uio->uio_offset;
	osal_atomic_set(&dev_buffer->atomic_len, dev_len);
	osal_atomic_fetch_add(&dev_buffer->atomic_gen_id, 1);

	if (error != 0) {
		PNSO_LOG_ERROR("Write failed: bad address!\n");
	} else {
		PNSO_LOG_DEBUG("Writing to device pnso_test done.\n");
	}

	return (error);
}

static int
pnso_test_write(struct cdev *dev, struct uio *uio, int ioflag __unused)
{
	struct cdevsw *devsw = dev->si_devsw;
	struct test_softc *softc;

	if (!devsw) return (ENXIO);

	softc = pnso_test_devname_to_softc(devsw->d_name);
	if (!softc) return (ENXIO);

	PNSO_LOG_DEBUG("Writing to device %s, offset %lu, len %lu.\n",
		       devsw->d_name, uio->uio_offset, uio->uio_resid);

	/* This is a new message, reset length */
	if (uio->uio_offset == 0) {
		pnso_test_cdev_clear_one(softc);
	}

	return copy_uspace_to_dev(&softc->input, uio);
}

char *pnso_test_sysfs_alloc_and_get_cfg(void)
{
	char *buf;
	int len;

	if (pnso_test_softc.input.buf == NULL)
		return NULL;
	
	PNSO_LOG_DEBUG("Sending pnso_test buffer for test.\n");

	buf = TEST_ALLOC(pnso_test_softc.input.maxlen + 1);
	if (buf == NULL)
		return (NULL);

	len = osal_atomic_read(&pnso_test_softc.input.atomic_len);
	memcpy(buf, pnso_test_softc.input.buf, len);

	buf[len] = 0;
	return (buf);
}

int pnso_test_sysfs_read_ctl(void)
{
	return (osal_atomic_exchange(&ctl_state, CTL_STATE_READ));
}

#define OFFSET_APPEND 0xffffffff

/* Return count of bytes copied */
static uint32_t copy_kspace_to_dev(struct test_softc_buffer *dev_buffer,
				   uint32_t dev_offset,
				   const uint8_t *src, uint32_t size)
{
	size_t amt;
	int dev_len;

	dev_len = osal_atomic_read(&dev_buffer->atomic_len);
	if (dev_offset == OFFSET_APPEND)
		dev_offset = dev_len;

	if (dev_buffer->buf == NULL ||
	    dev_offset >= dev_buffer->maxlen)
		return 0;

	if (dev_offset > dev_len) {
		/* do not allow gaps in data */
		PNSO_LOG_ERROR("Invalid dev write offset %u\n", dev_offset);
		return 0;
	}
	amt = MIN(size, dev_buffer->maxlen - dev_offset);
	memcpy(dev_buffer->buf + dev_offset, src, amt);
	dev_len = dev_offset + amt; /* overwrite causes truncation */
	osal_atomic_set(&dev_buffer->atomic_len, dev_len);
	osal_atomic_fetch_add(&dev_buffer->atomic_gen_id, 1);

	return amt;
}

/* Return count of bytes copied */
/* Assumes dst has at least size bytes */
static uint32_t copy_dev_to_kspace(struct test_softc_buffer *dev_buffer,
				   uint32_t dev_offset,
				   uint8_t *dst, uint32_t size)
{
	size_t amt;
	int dev_len;

	if (dev_buffer->buf == NULL ||
	    dev_offset >= dev_buffer->maxlen)
		return 0;

	dev_len = osal_atomic_read(&dev_buffer->atomic_len);
	if (dev_offset >= dev_len) {
		PNSO_LOG_ERROR("Invalid dev read offset %u\n", dev_offset);
		return 0;
	}
	amt = MIN(size, dev_len - dev_offset);
	memcpy(dst, dev_buffer->buf + dev_offset, amt);

	return amt;
}

/* Return 0 on success */
int pnso_test_sysfs_get_fd_data(int fdnum, struct pnso_flat_buffer *flat_buf, uint32_t *gen_id)
{
	struct test_softc *softc;

	softc = pnso_test_fdnum_to_softc(fdnum);
	if (!softc)
		return -1;

	*gen_id = (uint32_t) osal_atomic_read(&softc->input.atomic_gen_id);
	flat_buf->len = (uint32_t) osal_atomic_read(&softc->input.atomic_len);
	flat_buf->buf = (uint64_t) softc->input.buf;

	return 0;
}

uint32_t pnso_test_sysfs_get_fd_len(int fdnum, uint32_t *gen_id)
{
	struct test_softc *softc;

	softc = pnso_test_fdnum_to_softc(fdnum);
	if (!softc)
		return 0;

	*gen_id = (uint32_t) osal_atomic_read(&softc->input.atomic_gen_id);
	return (uint32_t) osal_atomic_read(&softc->input.atomic_len);
}

uint32_t pnso_test_sysfs_write_fd(int fdnum, uint32_t offset,
		const uint8_t *src, uint32_t size, uint32_t *gen_id)
{
	uint32_t ret;
	struct test_softc *softc;

	softc = pnso_test_fdnum_to_softc(fdnum);
	if (!softc)
		return 0;

	dev_lock();
	ret = copy_kspace_to_dev(&softc->input, offset, src, size);
	*gen_id = osal_atomic_read(&softc->input.atomic_gen_id);
	dev_unlock();

	return ret;
}

uint32_t pnso_test_sysfs_read_fd(int fdnum, uint32_t offset, uint8_t *dst,
				 uint32_t size, uint32_t *gen_id)
{
	uint32_t ret;
	struct test_softc *softc;

	softc = pnso_test_fdnum_to_softc(fdnum);
	if (!softc)
		return 0;

	dev_lock();
	ret = copy_dev_to_kspace(&softc->input, offset, dst, size);
	*gen_id = osal_atomic_read(&softc->input.atomic_gen_id);
	dev_unlock();

	return ret;
}

static void write_testcase_summary(uint32_t testcase_id, bool success)
{
	if (success) {
		pnso_test_inc_success_cnt();
	} else {
		pnso_test_inc_fail_cnt();
	}
}

void pnso_test_sysfs_write_status_data(const char *src, uint32_t len, void *opaque)
{
	struct testcase_context *test_ctx = (struct testcase_context *) opaque;

	copy_kspace_to_dev(&pnso_test_softc.output, OFFSET_APPEND,
			   src, len);

	if (test_ctx) {
		write_testcase_summary(test_ctx->testcase->node.idx,
			test_ctx->stats.agg_stats.validation_failures == 0);
	}
}
