//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file contains captrace conf, reset, dump and show implementation
///
//===----------------------------------------------------------------------===//

#include "platform/mputrace/mputrace.hpp"

namespace sdk {
namespace platform {

///
/// All util functions for captrace implementation
///
int g_pipeline_index = 0;

#define MPUTRACE_FOR_EACH_PIPELINE(pipeline, fn, args)                         \
    stage_count = sizeof(pipeline.mpu) / sizeof(cap_mpu_csr_t);                \
    fn(stage_count, pipeline.mpu, args);

sdk::types::mem_addr_t
mputrace_util_get_trace_addr(cap_mpu_csr_trace_t trace)
{
    return (sdk::types::mem_addr_t)(trace.int_var__base_addr << 6);
}

uint32_t
mputrace_util_get_trace_size(cap_mpu_csr_trace_t trace)
{
    return (MPUTRACE_ONE << (uint32_t)trace.int_var__buf_size);
}

int
mputrace_util_get_enable(cap_mpu_csr_trace_t trace)
{
    trace.read();
    return (int)trace.int_var__enable;
}

void
mputrace_erase_trace_buffer(sdk::types::mem_addr_t trace_addr, int trace_size)
{
    sdk::lib::pal_mem_set(trace_addr, 0, trace_size);
}

///
/// mputrace config and enable functions
///
void
mputrace_set_args(mpu_trace_record_t *args, int stage, int mpu)
{
    args->stage_id = stage;
    args->mpu = mpu;
}

///
/// Set the MPU register flags in the ASIC
///
static inline int
capri_mpu_trace_enable_internal(cap_mpu_csr_trace_t &trace, uint32_t stage_id,
                                uint32_t mpu, uint8_t enable,
                                uint8_t trace_enable, uint8_t phv_debug,
                                uint8_t phv_error, uint64_t watch_pc,
                                sdk::types::mem_addr_t base_addr,
                                uint8_t table_key, uint8_t instructions,
                                uint8_t wrap, uint8_t reset, uint32_t buf_size)
{
    trace.read();

    // TODO max check on mpu and stage_id

    trace.phv_debug(phv_debug);
    trace.phv_error(phv_error);

    if (watch_pc != 0) {
        trace.watch_pc(watch_pc >> 6); // TODO
        trace.watch_enable(1);
    } else {
        trace.watch_enable(0);
    }

    trace.base_addr(base_addr >> 6);
    trace.table_and_key(table_key);
    trace.instructions(instructions);
    trace.wrap(wrap);
    trace.rst(reset);
    trace.buf_size((uint32_t)log2(buf_size));
    trace.enable(enable);
    trace.trace_enable(trace_enable);
    trace.write();

    return 0;
}

void
capri_mpu_trace_enable(cap_mpu_csr_trace_t &trace, mpu_trace_record_t *args)
{
    static sdk::types::mem_addr_t trace_addr = g_trace_base;

    capri_mpu_trace_enable_internal(
        trace, args->stage_id, args->mpu, args->enable, args->trace_enable,
        args->phv_debug, args->phv_error, args->watch_pc, trace_addr,
        args->table_key, args->instructions, args->wrap, args->reset,
        args->trace_size);

    trace_addr += (args->trace_size * TRACE_ENTRY_SIZE);
}

static inline void
mputrace_program_txdma_mpu(cap_top_csr_t &cap0, mpu_trace_record_t *args)
{
    capri_mpu_trace_enable(cap0.pct.mpu[args->stage_id].trace[args->mpu], args);
}

static inline void
mputrace_program_rxdma_mpu(cap_top_csr_t &cap0, mpu_trace_record_t *args)
{
    capri_mpu_trace_enable(cap0.pcr.mpu[args->stage_id].trace[args->mpu], args);
}

static inline void
mputrace_program_sgi_mpu(cap_top_csr_t &cap0, mpu_trace_record_t *args)
{
    capri_mpu_trace_enable(cap0.sgi.mpu[args->stage_id].trace[args->mpu], args);
}

static inline void
mputrace_program_sge_mpu(cap_top_csr_t &cap0, mpu_trace_record_t *args)
{
    capri_mpu_trace_enable(cap0.sge.mpu[args->stage_id].trace[args->mpu], args);
}

static inline void
mputrace_program_mpu(mpu_trace_record_t *args)
{
    cap_top_csr_t &cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    switch (args->pipeline_type) {
        case TXDMA:
            mputrace_program_txdma_mpu(cap0, args);
            break;
        case RXDMA:
            mputrace_program_rxdma_mpu(cap0, args);
            break;
        case P4IG:
            mputrace_program_sgi_mpu(cap0, args);
            break;
        case P4EG:
            mputrace_program_sge_mpu(cap0, args);
            break;
        default:
            printf("Unknown pipeline type:%d. stage:%d mpu:%d\n",
                   args->pipeline_type, args->stage_id, args->mpu);
            break;
    }
}

///
/// Enable tracing for each mpu.
///
void
mputrace_trace_enable(mpu_trace_record_t *args)
{
    mputrace_program_mpu(args);
}

static inline void
mputrace_fill_pd_args(int pipeline, int stage, int mpu,
                      mpu_trace_record_t *args, mpu_trace_record_t *record)
{
    args->pipeline_type = pipeline;
    args->stage_id = stage;
    args->mpu = mpu;
    args->enable = record->enable;
    args->trace_enable = record->trace_enable;
    args->phv_debug = record->phv_debug;
    args->phv_error = record->phv_error;
    args->watch_pc = record->table_key;
    args->trace_addr = record->trace_addr;
    args->table_key = record->table_key;
    args->instructions = record->instructions;
    args->wrap = record->wrap;
    args->reset = record->reset;
    args->trace_size = record->trace_size;
}

static inline void
mputrace_reset_before_config()
{
    static bool mputrace_reset_config = true;
    if (mputrace_reset_config) {
        mputrace_reset();
        mputrace_reset_config = false;
    }
}

void
mputrace_config_trace(int pipeline, int stage, int mpu,
                      mpu_trace_record_t *record)
{
    mputrace_reset_before_config();
    mpu_trace_record_t args = {0};
    mputrace_fill_pd_args(pipeline, stage, mpu, &args, record);
    mputrace_trace_enable(&args);
}

static inline void
mputrace_dump_fill_record(mpu_trace_record_t *record, cap_mpu_csr_trace_t trace,
                          int pipeline, int stage, int mpu)
{
    record->pipeline_type = pipeline;
    record->stage_id = stage;
    record->mpu = mpu;
    record->trace_enable = (uint32_t)trace.trace_enable();
    record->phv_debug = (uint32_t)trace.phv_debug();
    record->phv_error = (uint32_t)trace.phv_error();
    record->watch_pc = (uint32_t)(trace.watch_pc() << 6);
    record->table_key = (uint32_t)trace.table_and_key();
    record->instructions = (uint32_t)trace.instructions();
    record->wrap = (uint32_t)trace.wrap();
    record->trace_addr = (uint64_t)(trace.base_addr() << 6);
    record->trace_size = (uint32_t)(MPUTRACE_ONE << (uint32_t)trace.buf_size());
}

///
/// mputrace dump functions
///
void
mputrace_dump_trace_to_file(cap_mpu_csr_trace_t trace, int pipeline, int stage,
                            int mpu)
{
    mpu_trace_record_t record = {};
    trace.read();

    uint32_t trace_size = mputrace_util_get_trace_size(trace);
    sdk::types::mem_addr_t trace_addr = mputrace_util_get_trace_addr(trace);
    uint8_t buf[64] = {0};
    static FILE *fp = fopen(g_MPUTRACE_DUMP_FILE, "wb");

    if (fp == NULL) {
        std::cerr << "Failed to open dump file for writing - "
                  << g_MPUTRACE_DUMP_FILE << std::endl;
        return;
    }
    mputrace_dump_fill_record(&record, trace, pipeline, stage, mpu);

    // Write header
    fwrite(&record, sizeof(uint8_t), sizeof(record), fp);
    for (uint32_t i = 0; i < trace_size; i++) {
        sdk::lib::pal_mem_read(trace_addr, buf, sizeof(buf));
        // Write trace buffer
        fwrite(buf, sizeof(buf[0]), sizeof(buf), fp);
        trace_addr += sizeof(buf);
    }
}

static inline void
mputrace_dump_trace(int stage_count, cap_mpu_csr_t *mpu_ptr, int pipeline)
{
    for (int stage = 0; stage < stage_count; stage++) {
        for (int mpu = 0; mpu < MPUTRACE_MAX_MPU; mpu++) {
            if (mputrace_util_get_enable(mpu_ptr->trace[mpu])) {
                mputrace_dump_trace_to_file(mpu_ptr->trace[mpu], pipeline,
                                            stage, mpu);
            }
        }
        mpu_ptr++;
    }
}

void
mputrace_dump()
{
    cap_top_csr_t &cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    int stage_count = 0;

    MPUTRACE_FOR_EACH_PIPELINE(cap0.pct, mputrace_dump_trace, 0);
    MPUTRACE_FOR_EACH_PIPELINE(cap0.pcr, mputrace_dump_trace, 1);
    MPUTRACE_FOR_EACH_PIPELINE(cap0.sgi, mputrace_dump_trace, 2);
    MPUTRACE_FOR_EACH_PIPELINE(cap0.sge, mputrace_dump_trace, 3);
}

///
/// mputrace reset functions
///
void
mputrace_reset_trace_buffer_internal(cap_mpu_csr_trace_t trace, void *ptr)
{
    sdk::types::mem_addr_t trace_addr = 0;
    uint32_t trace_size = 0;
    trace.read();

    if (mputrace_util_get_enable(trace)) {
        trace_addr = mputrace_util_get_trace_addr(trace);
        trace_size = mputrace_util_get_trace_size(trace);
        mputrace_erase_trace_buffer(trace_addr, trace_size);
    }
}

static inline void
mputrace_reset_trace(int stage_count, cap_mpu_csr_t *mpu_ptr,
                     mpu_trace_record_t *args)
{
    for (int s = 0; s < stage_count; s++) {
        for (int mpu = 0; mpu < MPUTRACE_MAX_MPU; mpu++) {
            mputrace_reset_trace_buffer_internal(mpu_ptr->trace[mpu], args);
        }
        mpu_ptr++;
    }
}

void
mputrace_reset_trace_buffer(cap_top_csr_t &cap0)
{
    int stage_count = 0;

    MPUTRACE_FOR_EACH_PIPELINE(cap0.sgi, mputrace_reset_trace, NULL);
    MPUTRACE_FOR_EACH_PIPELINE(cap0.sge, mputrace_reset_trace, NULL);
    MPUTRACE_FOR_EACH_PIPELINE(cap0.pct, mputrace_reset_trace, NULL);
    MPUTRACE_FOR_EACH_PIPELINE(cap0.pcr, mputrace_reset_trace, NULL);
}

static inline void
capri_mpu_trace_reset(cap_mpu_csr_trace_t &trace, mpu_trace_record_t *args)
{
    capri_mpu_trace_enable_internal(
        trace, args->stage_id, args->mpu, args->enable, args->trace_enable,
        args->phv_debug, args->phv_error, args->watch_pc, args->trace_addr,
        args->table_key, args->instructions, args->wrap, args->reset,
        args->trace_size);
}

static inline void
mputrace_reset_trace_regs(int stage_count, cap_mpu_csr_t *mpu_ptr,
                          mpu_trace_record_t *args)
{
    for (int s = 0; s < stage_count; s++) {
        for (int mpu = 0; mpu < MPUTRACE_MAX_MPU; mpu++) {
            capri_mpu_trace_reset(mpu_ptr->trace[mpu], args);
        }
        mpu_ptr++;
    }
}

void
mputrace_reset_regs(cap_top_csr_t &cap0)
{
    mpu_trace_record_t args = {0};
    int stage_count = 0;

    MPUTRACE_FOR_EACH_PIPELINE(cap0.sgi, mputrace_reset_trace_regs, &args);
    MPUTRACE_FOR_EACH_PIPELINE(cap0.sge, mputrace_reset_trace_regs, &args);
    MPUTRACE_FOR_EACH_PIPELINE(cap0.pct, mputrace_reset_trace_regs, &args);
    MPUTRACE_FOR_EACH_PIPELINE(cap0.pcr, mputrace_reset_trace_regs, &args);
}

void
mputrace_reset_capri()
{
    cap_top_csr_t &cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);

    mputrace_reset_trace_buffer(cap0);

    mputrace_reset_regs(cap0);
}

void
mputrace_reset()
{
    mputrace_reset_capri();
}

///
/// mputrace show functions
///
void
mputrace_show_pipeline_internal(cap_mpu_csr_trace_t trace,
                                mpu_trace_record_t *args)
{
    trace.read();
    if (mputrace_util_get_enable(trace)) {
        printf("%10" PRIu8 " %10" PRIu32 " %10" PRIu32 " %10" PRIu8 " %10" PRIu8
               " %10" PRIu8 " %10" PRIu8 " %10" PRIu8 " %10" PRIu8
               " 0x%08" PRIx64 " %10" PRIu8 " %10" PRIu8 " 0x%08" PRIx64
               " %10" PRIu32 " %10" PRIu32 "\n",
               // selectors
               g_pipeline_index, args->stage_id, args->mpu,
               // control
               (uint32_t)trace.enable(), (uint32_t)trace.wrap(),
               (uint32_t)trace.rst(),
               // triggers
               (uint32_t)trace.trace_enable(), (uint32_t)trace.phv_debug(),
               (uint32_t)trace.phv_error(), (uint64_t)(trace.watch_pc() << 6),
               // content
               (uint32_t)trace.table_and_key(), (uint32_t)trace.instructions(),
               // location
               (long unsigned int)(trace.base_addr() << 6),
               (uint32_t)(MPUTRACE_ONE << (uint32_t)trace.buf_size()),
               (uint32_t)(TRACE_ENTRY_SIZE *
                          (MPUTRACE_ONE << (uint32_t)trace.buf_size())));
    }
}

static inline void
mputrace_show_trace(int stage_count, cap_mpu_csr_t *mpu_ptr,
                    mpu_trace_record_t *args)
{
    g_pipeline_index++;
    for (int stage = 0; stage < stage_count; stage++) {
        for (int mpu = 0; mpu < MPUTRACE_MAX_MPU; mpu++) {
            mputrace_set_args(args, stage, mpu);
            mputrace_show_pipeline_internal(mpu_ptr->trace[mpu], args);
        }
        mpu_ptr++;
    }
}

void
mputrace_capri_show()
{
    cap_top_csr_t &cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    mpu_trace_record_t args = {0};
    int stage_count = 0;

    MPUTRACE_FOR_EACH_PIPELINE(cap0.sgi, mputrace_show_trace, &args);
    MPUTRACE_FOR_EACH_PIPELINE(cap0.sge, mputrace_show_trace, &args);
    MPUTRACE_FOR_EACH_PIPELINE(cap0.pcr, mputrace_show_trace, &args);
    MPUTRACE_FOR_EACH_PIPELINE(cap0.pct, mputrace_show_trace, &args);
}

void
mputrace_show()
{
    mputrace_capri_show();
}

} // end namespace platform
} // end namespace sdk
