#include <linux/list.h>

#include <rdma/ib_verbs.h>
#include <rdma/ib_user_verbs.h>
#include <rdma/ib_umem.h>
#include <rdma/ib_addr.h>

#include "ionic_ib.h"

#if 0
union dev_cmd {
	u32 words[16];
    u16 opcode;
	struct create_mr_cmd mr;
	struct create_cq_cmd create_cq;
} __packed;

union dev_cmd_comp {
	u32 words[4];
    u8 status;
	struct create_mr_comp mr;
	struct create_cq_comp create_cq;
} __packed;

static u8 ionic_dev_cmd_status(struct ionic_dev *idev)
{
	return ioread8(&idev->dev_cmd->comp.status);
}

static bool ionic_dev_cmd_done(struct ionic_dev *idev)
{
	return ioread32(&idev->dev_cmd->done) & DEV_CMD_DONE;
}

static void ionic_dev_cmd_comp(struct ionic_dev *idev, void *mem)
{
	union dev_cmd_comp *comp = mem;
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(comp->words); i++)
		comp->words[i] = ioread32(&idev->dev_cmd->comp.words[i]);
}

static void ionic_dev_cmd_go(struct ionic_dev *idev, union rdma_dev_cmd *cmd)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(cmd->words); i++)
		iowrite32(cmd->words[i], &idev->dev_cmd->cmd.words[i]);
	iowrite32(0, &idev->dev_cmd->done);
	iowrite32(1, &idev->dev_cmd_db->v);
}

static int ionic_dev_cmd_wait(struct ionic_dev *idev, unsigned long max_wait)
{
	unsigned long time;
	int done;

	BUG_ON(in_interrupt());

	/* Wait for dev cmd to complete...but no more than max_wait
	 */

	time = jiffies + max_wait;
	do {

		done = ionic_dev_cmd_done(idev);
		if (done)
			return 0;

		schedule_timeout_uninterruptible(HZ / 10);

	} while (time_after(time, jiffies));

	return -ETIMEDOUT;
}

static int ionic_dev_cmd_check_error(struct ionic_dev *idev)
{
	u8 status;

	status = ionic_dev_cmd_status(idev);
	switch (status) {
	case 0:
		return 0;
	}

	return -EIO;
}

static int ionic_dev_cmd_wait_check(struct ionic_dev *idev,
                                    unsigned long max_wait)
{
	int err;

	err = ionic_dev_cmd_wait(idev, max_wait);
	if (err)
		return err;
	return ionic_dev_cmd_check_error(idev);
}
#endif

int
ionic_rdma_cmd_post(struct ionic_ib_dev *rdev,
                    void *req,
                    void *resp)
{
    int err;
    struct ionic_dev *idev = &rdev->ionic->idev;
    
    printk(KERN_ERR "%s: cmd opcode %d\n", __FUNCTION__, ((struct nop_cmd *)req)->opcode);

    /*
     * TODO:
     * This is to make sure previous command is finished. This still does not
     * avoid race between eth driver and rdma driver for dev comamnds.
     */
    err = ionic_dev_cmd_wait_check(idev, HZ * 10);
    if (err) {
        printk(KERN_ERR "%s: pre cmd done wait failed\n", __FUNCTION__);
        return err;
    }

    ionic_dev_cmd_go(idev, req);

	err = ionic_dev_cmd_wait_check(idev, HZ * 10);
	if (err) {
        printk(KERN_ERR "%s: cmd failed\n", __FUNCTION__);        
		return err;
    }
	ionic_dev_cmd_comp(idev, resp);

    return 0;
}
