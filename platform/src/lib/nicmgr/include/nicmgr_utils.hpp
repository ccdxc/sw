// {C} Copyright 2018 Pensando Systems Inc. All rights reserved

#ifndef __NICMGR_UTILS_HPP__
#define __NICMGR_UTILS_HPP__

#define NIC_ASSERT(x)  assert(x)

#define CASE(type)                                                            \
    case type:                                                                \
        return #type

/* Offset of the field in the structure. */
#define	fldoff(type, field) \
	((int)&(((type *)0)->field))

/* Size of the field in the structure. */
#define	fldsiz(type, field) \
	(sizeof(((type *)0)->field))

/* Address of the structure from a field. */
#define	strbase(type, addr, field) \
	((type *)((char *)(addr) - fldoff(type, field)))

#define DEVAPI_CHECK                                                          \
    if (!dev_api) {                                                           \
        NIC_LOG_ERR("{}: Uninitialized devapi", name);                        \
        return (IONIC_RC_EAGAIN);                                             \
    }


#endif /* __NICMGR_UTILS_HPP__ */
