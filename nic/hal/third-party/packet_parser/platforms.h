/*
 * Copyright 2013 Google Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */
/*
 * Author: ncardwell@google.com (Neal Cardwell)
 *
 * Declarations for platform-specific information.
 */

#ifndef __PLATFORMS_H__
#define __PLATFORMS_H__


/* ------------------------- Linux --------------------- */


#include <features.h>
#include <linux/types.h>

#define HAVE_OPEN_MEMSTREAM     1
#define HAVE_FMEMOPEN           1
#define TUN_PATH                "/dev/net/tun"
#define HAVE_TCP_INFO           1
#define HAVE_TCP_CC_INFO	1
#define HAVE_SO_MEMINFO		1

#include "uapi_linux.h"

#endif /* __PLATFORMS_H__ */
