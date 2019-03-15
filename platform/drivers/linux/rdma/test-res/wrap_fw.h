#ifndef WRAP_FW_H
#define WRAP_FW_H

/* This is to hide the c stuff from c++ */

extern "C" {
#include <linux/bitmap.h>
#include "../drv/ionic/ionic_fw.h"
}

#endif
