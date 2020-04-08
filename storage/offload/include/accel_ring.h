/*
 * Copyright 2018-2019 Pensando Systems, Inc.  All rights reserved.
 *
 * This program is free software; you may redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#ifndef _ACCEL_RING_H_
#define _ACCEL_RING_H_

/*
 * Accelerator ring IDs
 */
enum {
    ACCEL_RING_ID_FIRST = 0,
    ACCEL_RING_CP = ACCEL_RING_ID_FIRST,
    ACCEL_RING_CP_HOT,
    ACCEL_RING_DC,
    ACCEL_RING_DC_HOT,
    ACCEL_RING_XTS0,
    ACCEL_RING_XTS1,
    ACCEL_RING_GCM0,
    ACCEL_RING_GCM1,
    ACCEL_RING_ID_MAX
};

typedef uint32_t    accel_ring_id_t;

/**
 * accel_ring_t - Hardware accelerator ring information
 *
 *     @ring_id:          ring ID
 *     @ring_base_pa:     physical address of ring base memory
 *     @ring_pndx_pa:     physical address of producer index memory
 *     @ring_shadow_pndx_pa: physical address of shadow producer index memory
 *     @ring_opaque_tag_pa: physical address of opaque tag memory
 *     @ring_size:        max number of ring entries (descriptors)
 *     @ring_desc_size:   descriptor size in bytes
 *     @ring_pndx_size:   producer index size in bytes
 *     @ring_opaque_tag_size: opaque tag size in bytes
 */
typedef struct accel_ring {
    accel_ring_id_t ring_id;
    uint64_t        ring_base_pa;
    uint64_t        ring_pndx_pa;
    uint64_t        ring_shadow_pndx_pa;
    uint64_t        ring_opaque_tag_pa;
    uint32_t        ring_size;
    uint16_t        ring_desc_size;
    uint8_t         ring_pndx_size;
    uint8_t         ring_opaque_tag_size;
} accel_ring_t;

#endif /* _ACCEL_RING_H_ */
