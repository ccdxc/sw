#ifndef IONIC_KCOMPAT
#define IONIC_KCOMPAT

#include <linux/version.h>

#if (LINUX_VERSION_CODE <= KERNEL_VERSION(4,16,3))
#define ionic_add_gid(...) \
	ionic_add_gid(struct ib_device *ibdev, u8 port,		\
		      unsigned int index,			\
		      const union ib_gid *gid,			\
		      const struct ib_gid_attr *attr,		\
		      void **context)
#define ionic_del_gid(...) \
	ionic_del_gid(struct ib_device *ibdev, u8 port,		\
		      unsigned int index, void **context)
#endif

#endif
