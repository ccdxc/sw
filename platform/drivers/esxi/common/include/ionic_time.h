/*
 * Copyright (c) 2019 Pensando Systems. All rights reserved.
 */

/*
 * ionic_time.h --
 *
 * Definitions of time related functions and data structures
 */

#ifndef _IONIC_TIME_H_
#define _IONIC_TIME_H_


#define IONIC_TIME_AFTER(a,b)     (((long)((b) - (a))) < 0)
#define IONIC_TIME_BEFORE(a,b)    IONIC_TIME_AFTER(b,a)


#endif /* End of _IONIC_TIME_H_ */

