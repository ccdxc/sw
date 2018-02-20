#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/ethtool.h>
#include <linux/mutex.h>
#include <linux/list.h>
#include <linux/rculist.h>
#include <linux/spinlock.h>
#include <linux/pci.h>
#include <net/dcbnl.h>
#include <net/ipv6.h>
#include <net/addrconf.h>
#include <linux/if_ether.h>
#include <linux/printk.h>

#include <rdma/ib_verbs.h>
#include <rdma/ib_user_verbs.h>
#include <rdma/ib_umem.h>
#include <rdma/ib_addr.h>

#include "ionic_ib.h"
//#include <rdma/ionic-abi.h>
#include "../../include/ionic-abi.h"
#include "ib_verbs.h"

#define DRIVER_NAME "ionic_ib"
#define DRIVER_VERSION "1.0-0"


static struct list_head ionic_ib_dev_list = LIST_HEAD_INIT(ionic_ib_dev_list);
/* Mutex to protect the list of ionic devices added */
static DEFINE_MUTEX(ionic_dev_lock);

static struct workqueue_struct *ionic_wq;

static ssize_t show_rev(struct device *device, struct device_attribute *attr,
                        char *buf)
{
	struct ionic_ib_dev *rdev = dev_to_ionic_ib_dev(device);

	return scnprintf(buf, PAGE_SIZE, "0x%x\n", rdev->pdev->vendor);
}

static ssize_t show_fw_ver(struct device *device, struct device_attribute *attr,
                           char *buf)
{
	struct ionic_ib_dev *rdev = dev_to_ionic_ib_dev(device);

	return scnprintf(buf, PAGE_SIZE, "%s\n", rdev->ionic->ident->dev.fw_version);
}

static ssize_t show_hca(struct device *device, struct device_attribute *attr,
                        char *buf)
{
	struct ionic_ib_dev *rdev = dev_to_ionic_ib_dev(device);

	return scnprintf(buf, PAGE_SIZE, "%s\n", rdev->ibdev.node_desc);
}

static DEVICE_ATTR(hw_rev, 0444, show_rev, NULL);
static DEVICE_ATTR(fw_rev, 0444, show_fw_ver, NULL);
static DEVICE_ATTR(hca_type, 0444, show_hca, NULL);

static struct device_attribute *ionic_attributes[] = {
	&dev_attr_hw_rev,
	&dev_attr_fw_rev,
	&dev_attr_hca_type
};

static struct ionic_ib_dev *ionic_from_netdev(struct net_device *netdev)
{
	struct ionic_ib_dev *rdev;

	rcu_read_lock();
	list_for_each_entry_rcu(rdev, &ionic_ib_dev_list, list) {
		if (rdev->netdev == netdev) {
			rcu_read_unlock();
			return rdev;
		}
	}
	rcu_read_unlock();
	return NULL;
}

static bool is_ionic_dev(struct net_device *netdev)
{
	struct ethtool_drvinfo drvinfo;

	if (netdev->ethtool_ops && netdev->ethtool_ops->get_drvinfo) {
		memset(&drvinfo, 0, sizeof(drvinfo));
		netdev->ethtool_ops->get_drvinfo(netdev, &drvinfo);

		if (strcmp(drvinfo.driver, "ionic"))
			return false;
		return true;
	}
	return false;
}

static struct ionic_ib_dev *ionic_ib_dev_add(struct net_device *netdev,
                                           struct lif *lif)
{
	struct ionic_ib_dev *rdev;

    pr_info("\n%s:", __FUNCTION__);
    
	/* Allocate ionic_ib_dev instance here */
	rdev = (struct ionic_ib_dev *)ib_alloc_device(sizeof(*rdev));
	if (!rdev) {
		pr_err("%s: ionic_ib_dev allocation failure!",
                __FUNCTION__);
		return NULL;
	}
	/* Default values */
	rdev->netdev = netdev;
	dev_hold(rdev->netdev);
	rdev->lif = lif;
    rdev->pdev = lif->ionic->pdev;
    rdev->ionic = lif->ionic;
	rdev->id = rdev->pdev->devfn;
	INIT_LIST_HEAD(&rdev->qp_list);
	mutex_init(&rdev->qp_lock);
	atomic_set(&rdev->qp_count, 0);
	atomic_set(&rdev->cq_count, 0);
	atomic_set(&rdev->srq_count, 0);
	atomic_set(&rdev->mr_count, 0);
	atomic_set(&rdev->mw_count, 0);

	rdev->sgid_tbl = kzalloc(sizeof(union ib_gid) *
                            IONIC_MAX_SGID, GFP_KERNEL);
	if (!rdev->sgid_tbl)
		return ERR_PTR(-ENOMEM);
    
	mutex_lock(&ionic_dev_lock);
	list_add_tail_rcu(&rdev->list, &ionic_ib_dev_list);
	mutex_unlock(&ionic_dev_lock);
	return rdev;
}

static int ionic_ib_dev_reg(struct ionic_ib_dev **rdev, struct net_device *netdev)
{
	struct lif *lif = netdev_priv(netdev);
	int rc = 0;

    pr_info("\n%s:", __FUNCTION__);
	if (!is_ionic_dev(netdev))
		return -ENODEV;

    // TODO: need bump up the netdev refcount
	if (!try_module_get(lif->ionic->pdev->driver->driver.owner)) {
        rc = -ENODEV;
        goto exit;
    }
    
    dev_hold(netdev);
    
	*rdev = ionic_ib_dev_add(netdev, lif);
	if (!*rdev) {
		rc = -ENOMEM;
        dev_put(netdev);
		goto exit;
	}
exit:
    
	return rc;
}

static void ionic_init_one(struct ionic_ib_dev *rdev)
{
	pci_dev_get(rdev->pdev);
}

/* GUID */
void ionic_get_guid(u8 *dev_addr, u8 *guid)
{
	u8 mac[ETH_ALEN];

    pr_info("\n%s:", __FUNCTION__);
	/* MAC-48 to EUI-64 mapping */
	memcpy(mac, dev_addr, ETH_ALEN);
	guid[0] = mac[0] ^ 2;
	guid[1] = mac[1];
	guid[2] = mac[2];
	guid[3] = 0xff;
	guid[4] = 0xfe;
	guid[5] = mac[3];
	guid[6] = mac[4];
	guid[7] = mac[5];
}

static int ionic_register_ib(struct ionic_ib_dev *rdev)
{
	struct ib_device *ibdev = &rdev->ibdev;

    pr_info("\n%s:", __FUNCTION__);
	/* ib device init */
	ibdev->owner = THIS_MODULE;
	ibdev->node_type = RDMA_NODE_IB_CA;
	strlcpy(ibdev->name, "ionic%d", IB_DEVICE_NAME_MAX);
	strlcpy(ibdev->node_desc, IONIC_DESC " HCA",
            strlen(IONIC_DESC) + 5);
	ibdev->phys_port_cnt = 1;

	ionic_get_guid(rdev->netdev->dev_addr, (u8 *)&ibdev->node_guid);

	ibdev->num_comp_vectors	= 1;
	ibdev->dev.parent = &rdev->pdev->dev;

	/* User space */
	ibdev->uverbs_abi_ver = IONIC_ABI_VERSION;
	ibdev->uverbs_cmd_mask =
        (1ull << IB_USER_VERBS_CMD_GET_CONTEXT)		|
        (1ull << IB_USER_VERBS_CMD_QUERY_DEVICE)	|
        (1ull << IB_USER_VERBS_CMD_QUERY_PORT)		|
        (1ull << IB_USER_VERBS_CMD_ALLOC_PD)		|
        (1ull << IB_USER_VERBS_CMD_DEALLOC_PD)		|
        (1ull << IB_USER_VERBS_CMD_REG_MR)		|
        (1ull << IB_USER_VERBS_CMD_REREG_MR)		|
        (1ull << IB_USER_VERBS_CMD_DEREG_MR)		|
        (1ull << IB_USER_VERBS_CMD_CREATE_COMP_CHANNEL) |
        (1ull << IB_USER_VERBS_CMD_CREATE_CQ)		|
        (1ull << IB_USER_VERBS_CMD_RESIZE_CQ)		|
        (1ull << IB_USER_VERBS_CMD_DESTROY_CQ)		|
        (1ull << IB_USER_VERBS_CMD_CREATE_QP)		|
        (1ull << IB_USER_VERBS_CMD_MODIFY_QP)		|
        (1ull << IB_USER_VERBS_CMD_QUERY_QP)		|
        (1ull << IB_USER_VERBS_CMD_DESTROY_QP)		|
        (1ull << IB_USER_VERBS_CMD_CREATE_SRQ)		|
        (1ull << IB_USER_VERBS_CMD_MODIFY_SRQ)		|
        (1ull << IB_USER_VERBS_CMD_QUERY_SRQ)		|
        (1ull << IB_USER_VERBS_CMD_DESTROY_SRQ)		|
        (1ull << IB_USER_VERBS_CMD_CREATE_AH)		|
        (1ull << IB_USER_VERBS_CMD_MODIFY_AH)		|
        (1ull << IB_USER_VERBS_CMD_QUERY_AH)		|
        (1ull << IB_USER_VERBS_CMD_DESTROY_AH);
	/* POLL_CQ and REQ_NOTIFY_CQ is directly handled in libionic */

	/* Kernel verbs */
	ibdev->query_device		= ionic_query_device;
	ibdev->query_port		= ionic_query_port;
	ibdev->query_gid		= ionic_query_gid;

	ibdev->alloc_pd			= ionic_alloc_pd;
	ibdev->dealloc_pd		= ionic_dealloc_pd;

    //Dummy routines
	ibdev->create_ah		= ionic_create_ah;
	ibdev->destroy_ah		= ionic_destroy_ah;
    ibdev->get_dma_mr		= ionic_get_dma_mr;
	ibdev->get_port_immutable	= ionic_get_port_immutable;
	ibdev->query_pkey		= ionic_query_pkey;
	ibdev->dereg_mr			= ionic_dereg_mr;

    // Needed for user space programs
	ibdev->reg_user_mr		= ionic_reg_user_mr;
	ibdev->alloc_ucontext	= ionic_alloc_ucontext;
	ibdev->dealloc_ucontext	= ionic_dealloc_ucontext;
	ibdev->mmap			    = ionic_mmap;

	ibdev->create_cq		= ionic_create_cq;
	ibdev->destroy_cq		= ionic_destroy_cq;
	ibdev->poll_cq			= ionic_poll_cq;
	ibdev->req_notify_cq	= ionic_req_notify_cq;


    ibdev->create_qp		= ionic_create_qp;
	ibdev->destroy_qp		= ionic_destroy_qp;
	ibdev->modify_qp		= ionic_modify_qp;
	ibdev->query_qp			= ionic_query_qp;

	ibdev->post_send		= ionic_post_send;
	ibdev->post_recv		= ionic_post_recv;

	ibdev->add_gid          = ionic_add_gid;
	ibdev->del_gid          = ionic_del_gid;
	ibdev->query_gid        = ionic_query_gid;

	ibdev->get_netdev       = ionic_get_netdev;
	ibdev->get_link_layer   = ionic_port_link_layer;
    
#if 0

	ibdev->alloc_mr			= ionic_alloc_mr;
#endif

	rdev->cq_tbl = kcalloc(IONIC_MAX_QP_COUNT, sizeof(void *),
                          GFP_KERNEL);
	if (!rdev->cq_tbl)
		return -ENOMEM;;
	spin_lock_init(&rdev->cq_tbl_lock);

	rdev->qp_tbl = kcalloc(IONIC_MAX_QP_COUNT, sizeof(void *),
                          GFP_KERNEL);
	if (!rdev->qp_tbl)
		return -ENOMEM;
	spin_lock_init(&rdev->qp_tbl_lock);
    
	return ib_register_device(ibdev, NULL);
}

static void ionic_unregister_ib(struct ionic_ib_dev *rdev)
{
	ib_unregister_device(&rdev->ibdev);
}

static int ionic_ib_reg(struct ionic_ib_dev *rdev)
{
	int rc = 0, i, j;
    
    pr_info("\n%s:", __FUNCTION__);
    /* Register ib dev */
    rc = ionic_register_ib(rdev);
    if (rc) {
        pr_err("Failed to register with IB: %#x\n", rc);
        return rc;
    }
        
    pr_err("%s:Device registered successfully", __FUNCTION__);

    for (i = 0; i < ARRAY_SIZE(ionic_attributes); i++) {
        rc = device_create_file(&rdev->ibdev.dev,
                                ionic_attributes[i]);
        if (rc) {
            dev_err(rdev_to_dev(rdev),
                    "Failed to create IB sysfs: %#x", rc);
            /* Must clean up all created device files */
            for (j = 0; j < i; j++)
                device_remove_file(&rdev->ibdev.dev,
                                   ionic_attributes[j]);
            ionic_unregister_ib(rdev);
            return rc;
        }
    }
	set_bit(IONIC_FLAG_IBDEV_REGISTERED, &rdev->flags);        
    ib_get_eth_speed(&rdev->ibdev, 1, &rdev->active_speed,
                     &rdev->active_width);

    return rc;
}

/* Handle all deferred netevents tasks */
static void ionic_task(struct work_struct *work)
{
	struct ionic_work *re_work;
	struct ionic_ib_dev *rdev;
	int rc = 0;

    pr_info("\n%s:", __FUNCTION__);
	re_work = container_of(work, struct ionic_work, work);
	rdev = re_work->rdev;

	if (re_work->event != NETDEV_REGISTER &&
	    !test_bit(IONIC_FLAG_IBDEV_REGISTERED, &rdev->flags))
		return;

	switch (re_work->event) {
	case NETDEV_REGISTER:
		rc = ionic_ib_reg(rdev);
		if (rc)
            pr_err("Failed to register with IB: %#x", rc);
		break;
#ifdef TODO        
	case NETDEV_UP:
		ionic_dispatch_event(&rdev->ibdev, NULL, 1,
                               IB_EVENT_PORT_ACTIVE);
		break;
	case NETDEV_DOWN:
		ionic_dev_stop(rdev);
		break;
	case NETDEV_CHANGE:
		if (!netif_carrier_ok(rdev->netdev))
			ionic_dev_stop(rdev);
		else if (netif_carrier_ok(rdev->netdev))
			ionic_dispatch_event(&rdev->ibdev, NULL, 1,
                                   IB_EVENT_PORT_ACTIVE);
		ib_get_eth_speed(&rdev->ibdev, 1, &rdev->active_speed,
                         &rdev->active_width);
		break;
#endif
	default:
		break;
	}
	smp_mb__before_atomic();
	clear_bit(IONIC_FLAG_TASK_IN_PROG, &rdev->flags);
	kfree(re_work);
}

static int ionic_netdev_event(struct notifier_block *notifier,
                                unsigned long event, void *ptr)
{
	struct net_device *real_dev, *netdev = netdev_notifier_info_to_dev(ptr);
	struct ionic_work *re_work;
	struct ionic_ib_dev *rdev;
	bool sch_work = false;
    int rc = 0;
    
    pr_err("\n%s: Enter event %d", __FUNCTION__, (int) event);
    
	real_dev = rdma_vlan_dev_real_dev(netdev);
	if (!real_dev)
		real_dev = netdev;

	rdev = ionic_from_netdev(real_dev);
	if (!rdev && event != NETDEV_REGISTER)
		goto exit;
	if (real_dev != netdev)
		goto exit;

	switch (event) {
	case NETDEV_REGISTER:
        pr_err("\n%s: event: netdev register dev:%s", __FUNCTION__, real_dev->name);

		if (rdev)
			break;
		rc = ionic_ib_dev_reg(&rdev, real_dev);
		if (rc == -ENODEV)
			break;
		if (rc) {
			pr_err("Failed to register with the device %s: %#x\n",
			       real_dev->name, rc);
			break;
		}
		ionic_init_one(rdev);

		sch_work = true;        
		break;

	case NETDEV_UNREGISTER:
        pr_err("\n%s: event: netdev unregister dev:%s", __FUNCTION__, real_dev->name);

#ifdef TODO
		/* netdev notifier will call NETDEV_UNREGISTER again later since
		 * we are still holding the reference to the netdev
		 */
		if (test_bit(IONIC_FLAG_TASK_IN_PROG, &rdev->flags))
			goto exit;
		ionic_ib_unreg(rdev, false);
		ionic_remove_one(rdev);
		ionic_ib_dev_unreg(rdev);
		break;
#endif
        
	default:
        pr_err("\n%s: default event: %d", __FUNCTION__, (int)event);        
		sch_work = true;
		break;
	}

	if (sch_work) {
		/* Allocate for the deferred task */
		re_work = kzalloc(sizeof(*re_work), GFP_ATOMIC);
		if (re_work) {
			re_work->rdev = rdev;
			re_work->event = event;
			re_work->vlan_dev = (real_dev == netdev ?
                                 NULL : netdev);
			INIT_WORK(&re_work->work, ionic_task);
			set_bit(IONIC_FLAG_TASK_IN_PROG, &rdev->flags);
			queue_work(ionic_wq, &re_work->work);
		}
	}
    
exit:
	return NOTIFY_DONE;
}

static struct notifier_block ionic_netdev_notifier = {
	.notifier_call = ionic_netdev_event
};

static int __init ionic_mod_init(void)
{
    //struct ionic_ib_dev *dev;
    int rc;
    
	pr_err("\n%s: Loading driver: %s version: %s", __FUNCTION__,
           DRIVER_NAME, DRIVER_VERSION);

	ionic_wq = create_singlethread_workqueue("ionic_rdma");
	if (!ionic_wq)
		return -ENOMEM;

	rc = register_netdevice_notifier(&ionic_netdev_notifier);
	if (rc) {
		pr_err("%s: Cannot register to netdevice_notifier",
		       __FUNCTION__);
		return rc;
	}

	return 0;
}

static void __exit ionic_mod_exit(void)
{
	pr_err("%s: Unloading driver: %s version: %s", __FUNCTION__,
           DRIVER_NAME, DRIVER_VERSION);
    
	unregister_netdevice_notifier(&ionic_netdev_notifier);

    if (ionic_wq)
		destroy_workqueue(ionic_wq);
}

module_init(ionic_mod_init);
module_exit(ionic_mod_exit);

MODULE_AUTHOR("Harinadh Nagulapalli <harinadh@pensando.io>");
MODULE_DESCRIPTION("Pensando Capri HCA IB driver");
MODULE_LICENSE("Dual BSD/GPL");


