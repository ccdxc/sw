#ifndef WRAP_RES_H
#define WRAP_RES_H

/* This is to hide the c stuff from c++ */

extern "C" {
#include <linux/bitmap.h>
#include <linux/slab.h>
#define kfree(x) kfree((void *)(x))
#include "../drv/ionic/res.h"
}

#endif
