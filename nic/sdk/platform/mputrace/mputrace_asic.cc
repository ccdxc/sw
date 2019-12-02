//
// {C} Copyright 2019 Pensando Systems Inc. All rights reserved
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file contains mputrace conf, reset, dump and show implementation
///
//===----------------------------------------------------------------------===//

#include "platform/mputrace/mputrace.hpp"

namespace sdk {
namespace platform {

#define ADDR_BOUNDARY 0x3f
#define MPUTRACE_FOR_EACH_PIPELINE(pipeline, fn, args)                         \
    stage_count = sizeof(pipeline.mpu) / sizeof(cap_mpu_csr_t);                \
    fn(stage_count, pipeline.mpu, args);

static inline mem_addr_t
mputrace_util_base_addr_get (cap_mpu_csr_trace_t trace)
{
    return (mem_addr_t)(trace.base_addr() << 6);
}

static inline uint32_t
mputrace_util_size_get (cap_mpu_csr_trace_t trace)
{
    return (MPUTRACE_ONE << (uint32_t)trace.buf_size());
}

static inline int
mputrace_util_is_enabled (cap_mpu_csr_trace_t trace)
{
    trace.read();
    return (int)trace.int_var__enable || (int)trace.int_var__trace_enable ||
           (int)trace.int_var__phv_debug || (int)trace.int_var__phv_error ||
           (int)trace.int_var__watch_enable;
}

// Sets the register flags in the ASIC
static inline int
mputrace_util_reg_program (cap_mpu_csr_trace_t &trace,
                           mputrace_cfg_inst_t *cfg_inst, mem_addr_t base_addr,
                           bool enable_all)
{
    trace.read();

    trace.phv_debug(cfg_inst->ctrl.phv_debug);
    trace.phv_error(cfg_inst->ctrl.phv_error);
    if (cfg_inst->ctrl.watch_pc != 0) {
        trace.watch_pc(cfg_inst->ctrl.watch_pc >> 3);
        trace.watch_enable(1);
    } else {
        trace.watch_enable(0);
    }
    trace.base_addr(base_addr >> 6);
    trace.table_and_key(cfg_inst->capture.table_key);
    trace.instructions(cfg_inst->capture.instructions);
    trace.wrap(cfg_inst->settings.wrap);
    trace.rst(cfg_inst->settings.reset);
    trace.buf_size((uint32_t)log2(cfg_inst->settings.trace_size));
    trace.enable(enable_all);
    trace.trace_enable(cfg_inst->ctrl.trace_enable);
    trace.write();
    return 0;
}

//
// mputrace cfg routines
//

static inline bool
mputrace_cfg_enable_all_get (mputrace_cfg_inst_t *cfg_inst)
{
    bool enable_all =
        !(cfg_inst->ctrl.trace_enable || cfg_inst->ctrl.phv_debug ||
          cfg_inst->ctrl.phv_error || cfg_inst->ctrl.watch_pc);

    return enable_all;
}

static inline bool
mputrace_is_cfg_enabled (int pipeline, int stage_id, int mpu,
                         cap_mpu_csr_trace_t &trace)
{
    if (mputrace_util_is_enabled(trace)) {
        cout << "Warning: MPU trace for pipeline : "
             << mputrace_pipeline_str_get(pipeline).c_str()
             << ", stage : " << stage_id << ", mpu : " << mpu
             << " is already enabled. "
                "Ignoring the current config instance."
             << endl;
        return TRUE;
    }
    return FALSE;
}

static inline void
mputrace_cfg_capri_mpu_trace_enable (int pipeline, int stage_id, int mpu,
                                     cap_mpu_csr_trace_t &trace,
                                     mputrace_cfg_inst_t *cfg_inst)
{
    static mem_addr_t trace_addr = g_state.trace_base;
    bool enable_all = mputrace_cfg_enable_all_get(cfg_inst);

    if (mputrace_is_cfg_enabled(pipeline, stage_id, mpu, trace)) {
        return;
    }
    if (trace_addr + (cfg_inst->settings.trace_size * TRACE_ENTRY_SIZE) >=
        g_state.trace_end) {
        cout << "Specified trace size exceeds the trace_end boundary. "
                "Please reduce the trace size and try again"
             << endl;
        exit(EXIT_FAILURE);
    }
    if ((trace_addr & ADDR_BOUNDARY) != 0x0) {
        cout << "trace_addr " << trace_addr
             << "does not overlap with "
                ": "
             << ADDR_BOUNDARY;
        exit(EXIT_FAILURE);
    }
    mputrace_util_reg_program(trace, cfg_inst, trace_addr, enable_all);
    trace_addr += (cfg_inst->settings.trace_size * TRACE_ENTRY_SIZE);
}

static inline void
mputrace_cfg_mpu_program (int pipeline, int stage_id, int mpu,
                          mputrace_cfg_inst_t *cfg_inst)
{
    cap_top_csr_t &cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);

    switch (pipeline) {
    case TXDMA:
        mputrace_cfg_capri_mpu_trace_enable(pipeline, stage_id, mpu,
                                            cap0.pct.mpu[stage_id].trace[mpu],
                                            cfg_inst);
        break;
    case RXDMA:
        mputrace_cfg_capri_mpu_trace_enable(pipeline, stage_id, mpu,
                                            cap0.pcr.mpu[stage_id].trace[mpu],
                                            cfg_inst);
        break;
    case P4IG:
        mputrace_cfg_capri_mpu_trace_enable(pipeline, stage_id, mpu,
                                            cap0.sgi.mpu[stage_id].trace[mpu],
                                            cfg_inst);
        break;
    case P4EG:
        mputrace_cfg_capri_mpu_trace_enable(pipeline, stage_id, mpu,
                                            cap0.sge.mpu[stage_id].trace[mpu],
                                            cfg_inst);
        break;
    default:
        cout << "Unknown pipeline type " << pipeline << ", stage " << stage_id
             << ", mpu " << mpu << endl;
        break;
    }
}

static inline void
mputrace_cfg_reset ()
{
    static bool mputrace_reset_cfg = true;

    if (mputrace_reset_cfg) {
        mputrace_reset();
        mputrace_reset_cfg = false;
    }
}

void
mputrace_cfg_trace (int pipeline, int stage, int mpu,
                    mputrace_cfg_inst_t *cfg_inst)
{
    mputrace_cfg_reset();
    mputrace_cfg_mpu_program(pipeline, stage, mpu, cfg_inst);
}

//
// mputrace dump routines
//

static inline void
mputrace_dump_trace_hdr_fill (mputrace_trace_hdr_t *trace_hdr,
                              cap_mpu_csr_trace_t trace, int pipeline,
                              int stage, int mpu)
{
    trace_hdr->pipeline_num = pipeline;
    trace_hdr->stage_num = stage;
    trace_hdr->mpu_num = mpu;
    trace_hdr->trace_enable = (uint32_t)trace.trace_enable();
    trace_hdr->phv_debug = (uint32_t)trace.phv_debug();
    trace_hdr->phv_error = (uint32_t)trace.phv_error();
    trace_hdr->watch_pc = (uint32_t)(trace.watch_pc() << 3);
    trace_hdr->table_key = (uint32_t)trace.table_and_key();
    trace_hdr->instructions = (uint32_t)trace.instructions();
    trace_hdr->wrap = (uint32_t)trace.wrap();
    trace_hdr->trace_addr = (uint64_t)(trace.base_addr() << 6);
    trace_hdr->trace_size =
        (uint32_t)(MPUTRACE_ONE << (uint32_t)trace.buf_size());
}

static inline void
mputrace_dump_trace_hdr_write (cap_mpu_csr_trace_t trace, int pipeline,
                               int stage, int mpu, FILE *fp)
{
    mputrace_trace_hdr_t trace_hdr = {};

    mputrace_dump_trace_hdr_fill(&trace_hdr, trace, pipeline, stage, mpu);
    fwrite(&trace_hdr, sizeof(uint8_t), sizeof(trace_hdr), fp);
}

static inline void
mputrace_dump_trace_info_write (cap_mpu_csr_trace_t trace, FILE *fp)
{
    uint8_t buf[64] = {0};
    uint32_t trace_size = mputrace_util_size_get(trace);
    mem_addr_t trace_addr = mputrace_util_base_addr_get(trace);

    for (uint32_t i = 0; i < trace_size; i++) {
        sdk::lib::pal_mem_read(trace_addr, buf, sizeof(buf));
        fwrite(buf, sizeof(buf[0]), sizeof(buf), fp);
        trace_addr += sizeof(buf);
    }
}

static inline void
mputrace_dump_trace_file_write (cap_mpu_csr_trace_t trace, int pipeline,
                                int stage, int mpu)
{
    static FILE *fp = fopen(g_state.MPUTRACE_DUMP_FILE, "wb");

    trace.read();
    if (fp == NULL) {
        std::cerr << "Failed to open dump file for writing - "
                  << g_state.MPUTRACE_DUMP_FILE << std::endl;
        exit(EXIT_FAILURE);
    }

    mputrace_dump_trace_hdr_write(trace, pipeline, stage, mpu, fp);
    mputrace_dump_trace_info_write(trace, fp);
}

static inline void
mputrace_dump_pipeline_info_fetch (int stage_count, cap_mpu_csr_t *mpu_ptr,
                                   int pipeline)
{
    for (int stage = 0; stage < stage_count; stage++) {
        for (int mpu = 0; mpu < MPUTRACE_MAX_MPU; mpu++) {
            if (mputrace_util_is_enabled(mpu_ptr->trace[mpu])) {
                mputrace_dump_trace_file_write(mpu_ptr->trace[mpu], pipeline,
                                               stage, mpu);
            }
        }
        mpu_ptr++;
    }
}

static inline void
mputrace_dump_all_pipelines ()
{
    int stage_count = 0;
    cap_top_csr_t &cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);

    MPUTRACE_FOR_EACH_PIPELINE(cap0.pct, mputrace_dump_pipeline_info_fetch, 0);
    MPUTRACE_FOR_EACH_PIPELINE(cap0.pcr, mputrace_dump_pipeline_info_fetch, 1);
    MPUTRACE_FOR_EACH_PIPELINE(cap0.sgi, mputrace_dump_pipeline_info_fetch, 2);
    MPUTRACE_FOR_EACH_PIPELINE(cap0.sge, mputrace_dump_pipeline_info_fetch, 3);
}

void
mputrace_dump (const char *dump_file)
{
    snprintf(g_state.MPUTRACE_DUMP_FILE, MPUTRACE_STR_NAME_LEN, "%s",
             dump_file);
    mputrace_dump_all_pipelines();
    cout << "'captrace dump " << g_state.MPUTRACE_DUMP_FILE << "' success!"
         << endl;
}

//
// mputrace reset routines
//

static inline void
mputrace_reset_buffer_erase (cap_mpu_csr_trace_t trace)
{
    mem_addr_t trace_addr = 0;
    uint32_t trace_size = 0;

    trace.read();
    if (mputrace_util_is_enabled(trace)) {
        trace_addr = mputrace_util_base_addr_get(trace);
        trace_size = mputrace_util_size_get(trace);
        sdk::lib::pal_mem_set(trace_addr, 0, trace_size * TRACE_ENTRY_SIZE);
    }
}

static inline void
mputrace_reset_pipeline (int stage_count, cap_mpu_csr_t *mpu_ptr,
                         mputrace_cfg_inst_t *cfg_inst)
{
    for (int s = 0; s < stage_count; s++) {
        for (int mpu = 0; mpu < MPUTRACE_MAX_MPU; mpu++) {
            mputrace_reset_buffer_erase(mpu_ptr->trace[mpu]);
            mputrace_util_reg_program(mpu_ptr->trace[mpu], cfg_inst, 0, false);
        }
        mpu_ptr++;
    }
}

static inline void
mputrace_reset_all_pipelines (cap_top_csr_t &cap0)
{
    int stage_count = 0;
    mputrace_cfg_inst_t cfg_inst;

    memset(&cfg_inst, 0, sizeof(cfg_inst));
    cfg_inst.settings.reset = true;
    MPUTRACE_FOR_EACH_PIPELINE(cap0.sgi, mputrace_reset_pipeline, &cfg_inst);
    MPUTRACE_FOR_EACH_PIPELINE(cap0.sge, mputrace_reset_pipeline, &cfg_inst);
    MPUTRACE_FOR_EACH_PIPELINE(cap0.pct, mputrace_reset_pipeline, &cfg_inst);
    MPUTRACE_FOR_EACH_PIPELINE(cap0.pcr, mputrace_reset_pipeline, &cfg_inst);
}

void
mputrace_reset ()
{
    cap_top_csr_t &cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    mputrace_reset_all_pipelines(cap0);
    cout << "captrace reset success" << endl;
}

//
// mputrace show routines
//

static inline void
mputrace_show_pipeline_internal (cap_mpu_csr_trace_t trace,
                                 mputrace_cfg_inst_t *cfg_inst, int stage,
                                 int mpu)
{
    trace.read();
    if (mputrace_util_is_enabled(trace)) {
        printf("%10s"
               "%10" PRIu32 " %10" PRIu32 " %10" PRIu8 " %10" PRIu8 " %10" PRIu8
               " %10" PRIu8 " %10" PRIu8 " %10" PRIu8 " 0x%08" PRIx64
               " %10" PRIu8 " %10" PRIu8 " 0x%08" PRIx64 " %10" PRIu32
               " %10" PRIu32 "\n",
               // selectors
               mputrace_pipeline_str_get(g_state.pipeline_index - 1).c_str(),
               // g_state.pipeline_index,
               stage, mpu,
               // ctrl
               (uint32_t)trace.enable(), (uint32_t)trace.wrap(),
               (uint32_t)trace.rst(),
               // ctrl
               (uint32_t)trace.trace_enable(), (uint32_t)trace.phv_debug(),
               (uint32_t)trace.phv_error(), (uint64_t)(trace.watch_pc() << 3),
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
mputrace_show_pipeline (int stage_count, cap_mpu_csr_t *mpu_ptr,
                        mputrace_cfg_inst_t *cfg_inst)
{
    g_state.pipeline_index++;
    for (int stage = 0; stage < stage_count; stage++) {
        for (int mpu = 0; mpu < MPUTRACE_MAX_MPU; mpu++) {
            mputrace_show_pipeline_internal(mpu_ptr->trace[mpu], cfg_inst,
                                            stage, mpu);
        }
        mpu_ptr++;
    }
}

static inline void
mputrace_show_all_pipelines ()
{
    int stage_count = 0;
    cap_top_csr_t &cap0 = CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
    mputrace_cfg_inst_t cfg_inst;

    MPUTRACE_FOR_EACH_PIPELINE(cap0.sgi, mputrace_show_pipeline, &cfg_inst);
    MPUTRACE_FOR_EACH_PIPELINE(cap0.sge, mputrace_show_pipeline, &cfg_inst);
    MPUTRACE_FOR_EACH_PIPELINE(cap0.pcr, mputrace_show_pipeline, &cfg_inst);
    MPUTRACE_FOR_EACH_PIPELINE(cap0.pct, mputrace_show_pipeline, &cfg_inst);
}

void
mputrace_show ()
{
    printf("%10s %10s %10s %10s %10s %10s %10s %10s %10s %10s %10s %10s %10s "
           "%10s %10s\n",
           "pipeline", "stage", "mpu", "enable", "wrap", "reset", "trace",
           "phv_debug", "phv_error", "watch_pc", "table_kd", "instr",
           "trace_addr", "trace_nent", "trace_sz");
    mputrace_show_all_pipelines();
    cout << "captrace show success" << endl;
}

}    // end namespace platform
}    // end namespace sdk
