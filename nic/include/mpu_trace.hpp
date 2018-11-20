
// {C} Copyright 2017 Pensando Systems Inc. All rights reserved

#ifndef __MPU_TRACE_H__
#define __MPU_TRACE_H__

#include <stdint.h>

#define MAX_NUM_PIPELINE 5
#define MAX_STAGES       8
#define MAX_MPU          4

#pragma pack(push, 1)

typedef struct mpu_trace_record_s {
    uint8_t   pipeline_type;
    uint32_t  stage_id;
    uint32_t  mpu;
    uint8_t   enable;
    uint8_t   trace_enable;
    uint8_t   phv_debug;
    uint8_t   phv_error;
    uint64_t  watch_pc;
    uint64_t  trace_addr;
    uint8_t   table_key;
    uint8_t   instructions;
    uint8_t   wrap;
    uint8_t   reset;
    uint32_t  trace_size;
    uint8_t   __pad[27];        // Pad to 64 bytes
} mpu_trace_record_t;

static_assert(sizeof(mpu_trace_record_t) == 64, "mpu trace record struct should be 64B");

#pragma pack(pop)

#endif // __MPU_TRACE_H__
