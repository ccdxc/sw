//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file contains mputrace conf, reset, dump and show implementation
///
//===----------------------------------------------------------------------===//

#include "platform/elbtrace/elbtrace.hpp"

namespace sdk {
namespace platform {

#define ADDR_BOUNDARY 0x3f
#define ELBTRACE_FOR_EACH_PIPELINE(pipeline, fn, args)                         \
    stage_count = sizeof(pipeline.stg) / sizeof(elb_stg_csr_t);                \
    fn(stage_count, pipeline.stg, args);

#define ELBTRACE_FOR_EACH_PIPELINE_SHOW(pipeline, fn, arg0, arg1)		\
    stage_count = sizeof(pipeline.stg) / sizeof(elb_stg_csr_t);                \
    fn(stage_count, pipeline.stg, arg0, arg1);

#define DMATRACE_CFG_WRITE(dma_trace, cfg_inst, base_addr, enable)  \
    dma_trace.base_addr(base_addr >> 6);                                      \
    dma_trace.buf_size((uint32_t)log2(cfg_inst->settings.buf_size));          \
    dma_trace.enable(enable);                                                 \
    dma_trace.wrap(cfg_inst->settings.wrap);                                  \
    dma_trace.rst(cfg_inst->settings.reset);                                  \
    dma_trace.phv_enable(cfg_inst->ctrl.phv_enable);                          \
    dma_trace.capture_all(cfg_inst->ctrl.capture_all);                        \
    dma_trace.axi_err_enable(cfg_inst->ctrl.axi_err_enable);                  \


 
#define DMATRACE_TRACE_HDR_WRITE(sta_trace, dma_trace, pipeline)	\
    trace_hdr->pipeline_num = pipeline;                                       \
    trace_hdr->enable = (uint32_t)dma_trace.enable();                         \
    trace_hdr->phv_enable = (uint32_t)dma_trace.phv_enable();                 \
    trace_hdr->capture_all = (uint32_t)dma_trace.capture_all();               \
    trace_hdr->axi_err_enable = (uint32_t)dma_trace.axi_err_enable();         \
    trace_hdr->wrap = (uint32_t)dma_trace.wrap();                             \
    trace_hdr->reset = (uint32_t)dma_trace.rst();                             \
    trace_hdr->base_addr = (uint64_t)(dma_trace.base_addr() << 6);            \
    trace_hdr->trace_index = (uint32_t)sta_trace.debug_index();		\
    trace_hdr->buf_size =                                                     \
        (uint32_t)(ELBTRACE_ONE << (uint32_t)dma_trace.buf_size());           \




static inline mem_addr_t
mputrace_util_base_addr_get (elb_mpu_csr_trace_t trace)
{
    return (mem_addr_t)(trace.base_addr() << 6);
}

static inline uint32_t
mputrace_util_size_get (elb_mpu_csr_trace_t trace)
{
    return (ELBTRACE_ONE << (uint32_t)trace.buf_size());
}

static inline int
mputrace_util_is_enabled (elb_mpu_csr_trace_t trace,
                          elb_mpu_csr_watch_pc_t watch_pc,
                          elb_mpu_csr_watch_data_t watch_data)
{
    trace.read();
    watch_pc.read();
    watch_data.read();

    return (int)trace.int_var__enable || (int)trace.int_var__trace_enable ||
        (int)trace.int_var__phv_debug || (int)trace.int_var__phv_error
        || (int)watch_pc.int_var__trace
        || (int)watch_data.int_var__rtrace
        || (int)watch_data.int_var__wtrace;
}

// Sets the register flags in the ASIC
static inline int
mputrace_util_reg_program (elb_mpu_csr_trace_t &trace,
                           elb_mpu_csr_watch_pc_t &watch_pc,
                           elb_mpu_csr_watch_data_t &watch_data,
                           mputrace_cfg_inst_t *cfg_inst, mem_addr_t base_addr,
                           bool enable_all)
{

  ///TRACE
    trace.read();

    trace.phv_debug(cfg_inst->ctrl.phv_debug);
    trace.phv_error(cfg_inst->ctrl.phv_error);
    trace.base_addr(base_addr >> 6);
    trace.table_and_key(cfg_inst->capture.table_key);
    trace.instructions(cfg_inst->capture.instructions);
    trace.wrap(cfg_inst->settings.wrap);
    trace.rst(cfg_inst->settings.reset);
    trace.buf_size((uint32_t)log2(cfg_inst->settings.trace_size));

    trace.enable(enable_all);
    trace.trace_enable(cfg_inst->ctrl.trace_enable);
    trace.write();

  ///WATCH_PC
    watch_pc.read();
    watch_pc.trace(cfg_inst->ctrl.watch_pc.trace);
    watch_pc.count(cfg_inst->ctrl.watch_pc.count);
    watch_pc.intr(cfg_inst->ctrl.watch_pc.intr);
    watch_pc.stop(cfg_inst->ctrl.watch_pc.stop);
    watch_pc.exception(cfg_inst->ctrl.watch_pc.exception);
    watch_pc.addr_lo(cfg_inst->ctrl.watch_pc.addr_lo >> 3);
    watch_pc.addr_hi(cfg_inst->ctrl.watch_pc.addr_hi >> 3);
    watch_pc.write();

  ///WATCH_DATA
    watch_data.read();
    watch_data.rtrace(cfg_inst->ctrl.watch_data.rtrace);
    watch_data.wtrace(cfg_inst->ctrl.watch_data.wtrace);
    watch_data.rintr(cfg_inst->ctrl.watch_data.rintr);
    watch_data.wintr(cfg_inst->ctrl.watch_data.wintr);
    watch_data.rstop(cfg_inst->ctrl.watch_data.rstop);
    watch_data.wstop(cfg_inst->ctrl.watch_data.wstop);
    watch_data.rexception(cfg_inst->ctrl.watch_data.rexception);
    watch_data.wexception(cfg_inst->ctrl.watch_data.wexception);
    watch_data.addr_lo(cfg_inst->ctrl.watch_data.addr_lo);
    watch_data.addr_hi(cfg_inst->ctrl.watch_data.addr_hi);
    watch_data.write();

    return 0;
}


// Sets the register flags in the ASIC
static inline int
sdptrace_util_reg_program (elb_sdp_csr_cfg_sdp_axi_t &sdp_axi,
			   elb_sdp_csr_cfg_sdp_axi_sw_reset_t &sdp_axi_sw_reset,
			   elb_sdp_csr_cfg_sdp_trace_trigger_t &sdp_trace_trigger,
                           sdptrace_cfg_inst_t *cfg_inst, mem_addr_t phv_base_addr,
                           mem_addr_t ctl_base_addr, bool enable)
{

  //cout << "SDP util_reg_program 1" << endl;
  ///cfg_sdp_axi
    sdp_axi.read();

    sdp_axi.ctl_base_addr(ctl_base_addr >> 6); //todo
    sdp_axi.phv_base_addr(phv_base_addr >> 6); //todo
    //if 8192 entries are needed for PHV, 
    //program 8192 >> 4 (8192/16) in ring_size.
    //so control ring = 512 and phv ring = 8192
    sdp_axi.ring_size((uint32_t)((cfg_inst->settings.ring_size) >> 4)); //todo: check with Neil/pratima
    sdp_axi.enable(enable);
    sdp_axi.stop_when_full(cfg_inst->settings.stop_when_full);
    sdp_axi.no_trace_when_full(cfg_inst->settings.no_trace_when_full);
    sdp_axi.write();

    //printf ("Enable 0x%x\n",enable);
    //printf ("ctl_base_addr 0x%lx\n",ctl_base_addr);


    //    cout << "SDP util_reg_program 2" << endl;

  ///cfg_sdp_axi_sw_reset
    sdp_axi_sw_reset.read();

    sdp_axi_sw_reset.enable(cfg_inst->settings.sw_reset_enable);
    sdp_axi_sw_reset.write();

    //    cout << "SDP util_reg_program 3" << endl;
      
  ///cfg_sdp_trace_trigger
    sdp_trace_trigger.read();

    sdp_trace_trigger.enable(cfg_inst->ctrl.trace_trigger_enable);

    //printf ("trace trigger enable 0x%x\n",cfg_inst->ctrl.trace_trigger_enable);

    cpp_int trigger_data_val = sdp_trace_trigger.data();
    cpp_int_helper hlp0;
    
    if (cfg_inst->pipeline_str == "rxdma") {
      trigger_data_val = hlp0.set_slc(trigger_data_val,cfg_inst->capture.trigger_data_rx, 0, 511);
    } else if (cfg_inst->pipeline_str == "txdma") {
      trigger_data_val = hlp0.set_slc(trigger_data_val,cfg_inst->capture.trigger_data_tx, 0, 511);
    } else {
      trigger_data_val = hlp0.set_slc(trigger_data_val,cfg_inst->capture.trigger_data_p4, 0, 511);
    }
    //        cout << " justina " << hex << trigger_data_val << dec << endl;
    sdp_trace_trigger.data(trigger_data_val);

    cpp_int trigger_mask_val = sdp_trace_trigger.mask();
    cpp_int_helper hlp1;


    if (cfg_inst->pipeline_str == "rxdma") {
      trigger_mask_val = hlp1.set_slc(trigger_mask_val,cfg_inst->capture.trigger_mask_rx, 0, 511);
    } else if (cfg_inst->pipeline_str == "txdma") {
      trigger_mask_val = hlp1.set_slc(trigger_mask_val,cfg_inst->capture.trigger_mask_rx, 0, 511);
    } else {
      trigger_mask_val = hlp1.set_slc(trigger_mask_val,cfg_inst->capture.trigger_mask_p4, 0, 511);
    }

    sdp_trace_trigger.mask(trigger_mask_val);

    sdp_trace_trigger.write();

    //    cout << "SDP util_reg_program 4" << endl;

    return 0;
}


// Sets the register flags in the ASIC
static inline int
dmatrace_util_reg_program (elb_top_csr_t &elb0,
                           dmatrace_cfg_inst_t *cfg_inst, 
                           mem_addr_t base_addr, 
			   bool enable, 
			   std::string mod_name)
{
  elb_prd_csr_cfg_trace_t prd_trace = elb0.pr.pr.prd.cfg_trace;
  elb_ptd_csr_cfg_trace_t ptd_trace = elb0.pt.pt.ptd.cfg_trace;

  if (mod_name == "prd") {
    cout << "writing PRD config" << endl;

    printf("enable %d\n", enable);
    printf("base_addr %lx\n",  base_addr);
    printf("buf_size %d\n",  cfg_inst->settings.buf_size);
    printf("wrap %d\n",             cfg_inst->settings.wrap	 );
    printf("reset %d\n",            cfg_inst->settings.reset	 );
    printf("phv_enable %d\n",       cfg_inst->ctrl.phv_enable	 );
    printf("capture_all %d\n",      cfg_inst->ctrl.capture_all	 );
    printf("axi_err_enable %d\n",   cfg_inst->ctrl.axi_err_enable  );

    prd_trace.read();
    DMATRACE_CFG_WRITE(prd_trace, cfg_inst, base_addr, enable);
    prd_trace.pkt_phv_sync_err_enable(cfg_inst->ctrl.pkt_phv_sync_err_enable);
    prd_trace.write();

    cout << "done writing PRD config " << enable << endl;

  } 
  else {
    cout << "writing PTD config" << endl;

    printf("enable %d\n",  enable);
    printf("base_addr %lx\n",  base_addr);
    printf("buf_size %d\n",  cfg_inst->settings.buf_size);
    printf("wrap %d\n",             cfg_inst->settings.wrap	 );
    printf("reset %d\n",            cfg_inst->settings.reset	 );
    printf("phv_enable %d\n",       cfg_inst->ctrl.phv_enable	 );
    printf("capture_all %d\n",      cfg_inst->ctrl.capture_all	 );
    printf("axi_err_enable %d\n",   cfg_inst->ctrl.axi_err_enable  );

    ptd_trace.read();
    DMATRACE_CFG_WRITE(ptd_trace, cfg_inst, base_addr, enable);
    ptd_trace.write();
    cout << "done writing PTD config " << enable << endl;
  }

    return 0;
}


//
// mputrace cfg routines
//

//todo: justina: My assumption is that watch_pc/watch_data should set trace.enable as well.
//         Check with Neel/Mike
static inline bool
mputrace_cfg_enable_all_get (mputrace_cfg_inst_t *cfg_inst)
{
    bool enable_all = !(cfg_inst->ctrl.trace_enable      ||
                        cfg_inst->ctrl.phv_debug         ||
                        cfg_inst->ctrl.phv_error         ||
                        cfg_inst->ctrl.watch_pc.trace    ||
                        cfg_inst->ctrl.watch_data.rtrace ||
                        cfg_inst->ctrl.watch_data.wtrace);

    return enable_all;
}

static inline bool
mputrace_is_cfg_enabled (int pipeline, int stage_id, int mpu,
                         elb_mpu_csr_trace_t &trace,
                         elb_mpu_csr_watch_pc_t &watch_pc,
                         elb_mpu_csr_watch_data_t &watch_data)
{
    if (mputrace_util_is_enabled(trace, watch_pc, watch_data)) {
        cout << "Warning: MPU trace for pipeline : "
            << mputrace_pipeline_str_get(pipeline).c_str()
            << ", stage : " << stage_id << ", mpu : " << mpu
            << " is already enabled. " "Ignoring the current config instance."
            << endl;
        return TRUE;
    }

    return FALSE;
}

static inline void
mputrace_cfg_elba_mpu_trace_enable (int pipeline, int stage_id, int mpu,
                                    elb_mpu_csr_trace_t &trace,
                                    elb_mpu_csr_watch_pc_t &watch_pc,
                                    elb_mpu_csr_watch_data_t &watch_data,
                                    mputrace_cfg_inst_t *cfg_inst)
{
    static mem_addr_t trace_addr = g_state.trace_base;
    bool enable_all = mputrace_cfg_enable_all_get(cfg_inst);

    if (mputrace_is_cfg_enabled(pipeline, stage_id, mpu, trace,
                                watch_pc, watch_data)) {
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
    mputrace_util_reg_program(trace, watch_pc, watch_data,
                              cfg_inst, trace_addr, enable_all);
    trace_addr += (cfg_inst->settings.trace_size * TRACE_ENTRY_SIZE);
}

static inline void
sdptrace_cfg_elba_sdp_trace_enable (int pipeline, int stage_id,
				    elb_sdp_csr_cfg_sdp_axi_t &sdp_axi,
				    elb_sdp_csr_cfg_sdp_axi_sw_reset_t &sdp_axi_sw_reset,
				    elb_sdp_csr_cfg_sdp_trace_trigger_t &sdp_trace_trigger,
                                    sdptrace_cfg_inst_t *cfg_inst)
{
    static mem_addr_t phv_base_addr = g_state.sdptrace_phv_base;
    static mem_addr_t ctl_base_addr = g_state.sdptrace_ctl_base;

    bool enable = cfg_inst->ctrl.enable; 

    printf ("Enable jk 0x%x\n", cfg_inst->ctrl.enable);

    sdp_axi.read();

    if ( (int)sdp_axi.int_var__enable ) {
        cout << "Warning: SDP trace for pipeline : "
            << mputrace_pipeline_str_get(pipeline).c_str()
            << ", stage : " << stage_id 
            << " is already enabled. " "Ignoring the current config instance."
            << endl;
        return;
    }
    
    if (phv_base_addr + (cfg_inst->settings.ring_size * SDPTRACE_PHV_ENTRY_SIZE) >=
        g_state.sdptrace_phv_end) {
        cout << "Specified SDP PHV trace size exceeds the trace_end boundary. "
                "Please reduce the trace size and try again"
             << endl;
        exit(EXIT_FAILURE);
    }

    if (ctl_base_addr + (cfg_inst->settings.ring_size * SDPTRACE_CTL_ENTRY_SIZE) >=
        g_state.sdptrace_ctl_end) {
        cout << "Specified SDP CTL trace size exceeds the trace_end boundary. "
                "Please reduce the trace size and try again"
             << endl;
        exit(EXIT_FAILURE);
    }

    if ((phv_base_addr & ADDR_BOUNDARY) != 0x0) {
        cout << "SDP phv_base_addr " << phv_base_addr
             << "does not overlap with "
                ": "
             << ADDR_BOUNDARY;
        exit(EXIT_FAILURE);
    }
    if ((ctl_base_addr & ADDR_BOUNDARY) != 0x0) {
        cout << "SDP ctl_base_addr " << ctl_base_addr
             << "does not overlap with "
                ": "
             << ADDR_BOUNDARY;
        exit(EXIT_FAILURE);
    }

    sdptrace_util_reg_program(sdp_axi, sdp_axi_sw_reset, sdp_trace_trigger,
                              cfg_inst, phv_base_addr, ctl_base_addr, enable);
    phv_base_addr += (cfg_inst->settings.ring_size * SDPTRACE_PHV_ENTRY_SIZE);
    ctl_base_addr += (cfg_inst->settings.ring_size * SDPTRACE_CTL_ENTRY_SIZE);
}

static inline void
dmatrace_cfg_elba_dma_trace_enable (int pipeline, 
				    elb_top_csr_t &elb0,
                                    dmatrace_cfg_inst_t *cfg_inst,
				    std::string mod_name)
{
    static mem_addr_t base_addr = g_state.dmatrace_base;
    bool enable_all = cfg_inst->ctrl.enable; 

    cout << "inside dma_trace_enable" << endl;

    if (mod_name == "prd") {
      elb_prd_csr_cfg_trace_t prd_trace = elb0.pr.pr.prd.cfg_trace;
      prd_trace.read();

      if ( (int)prd_trace.int_var__enable ) {
        cout << "Warning: DMA trace for pipeline : "
	     << dmatrace_pipeline_str_get(pipeline).c_str()
	     << " is already enabled. " "Ignoring the current config instance."
	     << endl;
        return;
      }
    }
    else {
      elb_ptd_csr_cfg_trace_t ptd_trace = elb0.pt.pt.ptd.cfg_trace;
      ptd_trace.read();

      if ( (int)ptd_trace.int_var__enable ) {
        cout << "Warning: DMA trace for pipeline : "
	     << dmatrace_pipeline_str_get(pipeline).c_str()
	     << " is already enabled. " "Ignoring the current config instance."
	     << endl;
        return;
      }
    }

    
    if (base_addr + (cfg_inst->settings.buf_size * DMATRACE_ENTRY_SIZE) >=
        g_state.dmatrace_end) {
        cout << "Specified DMA trace size exceeds the trace_end boundary. "
                "Please reduce the trace size and try again"
             << endl;
        exit(EXIT_FAILURE);
    }

    if ((base_addr & ADDR_BOUNDARY) != 0x0) {
        cout << "DMA base_addr " << base_addr
             << "does not overlap with "
                ": "
             << ADDR_BOUNDARY;
        exit(EXIT_FAILURE);
    }

    dmatrace_util_reg_program(elb0, cfg_inst, base_addr, enable_all, mod_name);
    base_addr += (cfg_inst->settings.buf_size * DMATRACE_ENTRY_SIZE);
}


static inline void
mputrace_cfg_program (int pipeline, int stage_id, int mpu,
                          mputrace_cfg_inst_t *cfg_inst)
{
    elb_top_csr_t &elb0 = ELB_BLK_REG_MODEL_ACCESS(elb_top_csr_t, 0, 0);

    switch (pipeline) {
    case TXDMA:
        mputrace_cfg_elba_mpu_trace_enable(pipeline, stage_id, mpu,
                                           elb0.pct.stg[stage_id].mpu[mpu].trace,
                                           elb0.pct.stg[stage_id].mpu[mpu].watch_pc,
                                           elb0.pct.stg[stage_id].mpu[mpu].watch_data,
                                           cfg_inst);
        break;
    case RXDMA:
        mputrace_cfg_elba_mpu_trace_enable(pipeline, stage_id, mpu,
                                           elb0.pcr.stg[stage_id].mpu[mpu].trace,
                                           elb0.pcr.stg[stage_id].mpu[mpu].watch_pc,
                                           elb0.pcr.stg[stage_id].mpu[mpu].watch_data,
                                           cfg_inst);
        break;
    case P4IG:
        mputrace_cfg_elba_mpu_trace_enable(pipeline, stage_id, mpu,
                                           elb0.sgi.stg[stage_id].mpu[mpu].trace,
                                           elb0.sgi.stg[stage_id].mpu[mpu].watch_pc,
                                           elb0.sgi.stg[stage_id].mpu[mpu].watch_data,
                                           cfg_inst);
        break;
    case P4EG:
        mputrace_cfg_elba_mpu_trace_enable(pipeline, stage_id, mpu,
                                           elb0.sge.stg[stage_id].mpu[mpu].trace,
                                           elb0.sge.stg[stage_id].mpu[mpu].watch_pc,
                                           elb0.sge.stg[stage_id].mpu[mpu].watch_data,
                                           cfg_inst);
        break;
    default:
        cout << "Unknown pipeline type " << pipeline << ", stage " << stage_id
             << ", mpu " << mpu << endl;
        break;
    }
}

static inline void
sdptrace_cfg_program (int pipeline, int stage_id,
                          sdptrace_cfg_inst_t *cfg_inst)
{
    elb_top_csr_t &elb0 = ELB_BLK_REG_MODEL_ACCESS(elb_top_csr_t, 0, 0);

    switch (pipeline) {
    case TXDMA:
        sdptrace_cfg_elba_sdp_trace_enable(pipeline, stage_id, 
                                           elb0.pct.stg[stage_id].sdp.cfg_sdp_axi,	    
                                           elb0.pct.stg[stage_id].sdp.cfg_sdp_axi_sw_reset, 
                                           elb0.pct.stg[stage_id].sdp.cfg_sdp_trace_trigger,
                                           cfg_inst);
        break;
    case RXDMA:
        sdptrace_cfg_elba_sdp_trace_enable(pipeline, stage_id, 
                                           elb0.pcr.stg[stage_id].sdp.cfg_sdp_axi,	    
                                           elb0.pcr.stg[stage_id].sdp.cfg_sdp_axi_sw_reset, 
                                           elb0.pcr.stg[stage_id].sdp.cfg_sdp_trace_trigger,
                                           cfg_inst);
        break;
    case P4IG:
        sdptrace_cfg_elba_sdp_trace_enable(pipeline, stage_id, 
                                           elb0.sgi.stg[stage_id].sdp.cfg_sdp_axi,	    
                                           elb0.sgi.stg[stage_id].sdp.cfg_sdp_axi_sw_reset, 
                                           elb0.sgi.stg[stage_id].sdp.cfg_sdp_trace_trigger,
                                           cfg_inst);
        break;
    case P4EG:
        sdptrace_cfg_elba_sdp_trace_enable(pipeline, stage_id, 
                                           elb0.sge.stg[stage_id].sdp.cfg_sdp_axi,	    
                                           elb0.sge.stg[stage_id].sdp.cfg_sdp_axi_sw_reset, 
                                           elb0.sge.stg[stage_id].sdp.cfg_sdp_trace_trigger,
                                           cfg_inst);
        break;
    default:
        cout << "Unknown pipeline type " << pipeline << ", stage " << stage_id << endl;
        break;
    }
}


static inline void
dmatrace_cfg_program (int pipeline,
		      dmatrace_cfg_inst_t *cfg_inst)
{
    elb_top_csr_t &elb0 = ELB_BLK_REG_MODEL_ACCESS(elb_top_csr_t, 0, 0);

    switch (pipeline) {
    case TXPDMA:
        dmatrace_cfg_elba_dma_trace_enable(pipeline, elb0, 
					   cfg_inst, "ptd");
	cout << "cfg program TX" << endl;
        break;
    case RXPDMA:
        dmatrace_cfg_elba_dma_trace_enable(pipeline, elb0, 
					   cfg_inst, "prd");
	cout << "cfg program RX" << endl;
        break;
    default:
        cout << "Unknown pipeline type " << pipeline << endl;
        break;
    }
}


static inline void
mputrace_cfg_reset (void)
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
    mputrace_cfg_program(pipeline, stage, mpu, cfg_inst);
}

static inline void
sdptrace_cfg_reset (void)
{
    static bool sdptrace_reset_cfg = true;

    if (sdptrace_reset_cfg) {
        sdptrace_reset();
        sdptrace_reset_cfg = false;
    }
}

void
sdptrace_cfg_trace (int pipeline, int stage,
                    sdptrace_cfg_inst_t *cfg_inst)
{
    sdptrace_cfg_reset();
    sdptrace_cfg_program(pipeline, stage, cfg_inst);
}

static inline void
dmatrace_cfg_reset (void)
{
    static bool dmatrace_reset_cfg = true;

    cout << "dmatrace_cfg_trace 1" << endl;

    if (dmatrace_reset_cfg) {
        dmatrace_reset();
        dmatrace_reset_cfg = false;
    }

    cout << "dmatrace_cfg_trace 2" << endl;
}

void
dmatrace_cfg_trace (int pipeline, 
                    dmatrace_cfg_inst_t *cfg_inst)
{
    dmatrace_cfg_reset();
    cout << "cfg trace" << endl;
    dmatrace_cfg_program(pipeline, cfg_inst);
}

//
// mputrace dump routines
//

static inline void
mputrace_dump_trace_hdr_fill (mputrace_trace_hdr_t *trace_hdr,
                              elb_mpu_csr_trace_t trace,
                              elb_mpu_csr_watch_pc_t watch_pc,
                              elb_mpu_csr_watch_data_t watch_data,
                              int pipeline,
                              int stage, int mpu)
{
    trace_hdr->pipeline_num = pipeline;
    trace_hdr->stage_num = stage;
    trace_hdr->mpu_num = mpu;
    trace_hdr->enable = (uint32_t)trace.enable();
    trace_hdr->trace_enable = (uint32_t)trace.trace_enable();
    trace_hdr->phv_debug = (uint32_t)trace.phv_debug();
    trace_hdr->phv_error = (uint32_t)trace.phv_error();
    trace_hdr->table_key = (uint32_t)trace.table_and_key();
    trace_hdr->instructions = (uint32_t)trace.instructions();
    trace_hdr->wrap = (uint32_t)trace.wrap();
    trace_hdr->trace_addr = (uint64_t)(trace.base_addr() << 6);
    trace_hdr->trace_size =
        (uint32_t)(ELBTRACE_ONE << (uint32_t)trace.buf_size());

    trace_hdr->debug_index	= (uint32_t)trace.debug_index();
    trace_hdr->debug_generation =  (uint8_t)trace.debug_generation();

    //watch_pc
    trace_hdr->wpc_trace = (uint32_t)watch_pc.trace();
    trace_hdr->wpc_count = (uint32_t)watch_pc.count();
    trace_hdr->wpc_intr  = (uint32_t)watch_pc.intr();
    trace_hdr->wpc_stop  = (uint32_t)watch_pc.stop();
    trace_hdr->wpc_exception = (uint32_t)watch_pc.exception();
    trace_hdr->wpc_addr_lo = (uint64_t)(watch_pc.addr_lo() << 3);
    trace_hdr->wpc_addr_hi = (uint64_t)(watch_pc.addr_hi() << 3);

    //watch_data
    trace_hdr->wdata_rtrace = (uint32_t)watch_data.rtrace();
    trace_hdr->wdata_wtrace = (uint32_t)watch_data.wtrace();
    trace_hdr->wdata_rintr  = (uint32_t)watch_data.rintr();
    trace_hdr->wdata_wintr  = (uint32_t)watch_data.wintr();
    trace_hdr->wdata_rstop  = (uint32_t)watch_data.rstop();
    trace_hdr->wdata_wstop  = (uint32_t)watch_data.wstop();
    trace_hdr->wdata_rexception = (uint32_t)watch_data.rexception();
    trace_hdr->wdata_wexception = (uint32_t)watch_data.wexception();
    trace_hdr->wdata_addr_lo = (uint64_t)watch_data.addr_lo();
    trace_hdr->wdata_addr_hi = (uint64_t)watch_data.addr_hi();
}

static inline void
sdptrace_dump_trace_hdr_fill (sdptrace_trace_hdr_t *trace_hdr,
			      elb_sdp_csr_sta_sdp_axi_write_control_t &ctl_ptr,
			      elb_sdp_csr_sta_sdp_axi_write_phv_t &phv_ptr,
			      elb_sdp_csr_cfg_sdp_axi_t &sdp_axi,
			      elb_sdp_csr_cfg_sdp_axi_sw_reset_t &sdp_axi_sw_reset,
			      elb_sdp_csr_cfg_sdp_trace_trigger_t &sdp_trace_trigger,
                              int pipeline,
                              int stage)
{
    trace_hdr->pipeline_num = pipeline;
    trace_hdr->stage_num = stage;

    trace_hdr->enable = (uint32_t)sdp_axi.enable();
    trace_hdr->stop_when_full = (uint32_t)sdp_axi.stop_when_full();
    trace_hdr->no_trace_when_full = (uint32_t)sdp_axi.no_trace_when_full();
    trace_hdr->ctl_base_addr = (uint64_t)(sdp_axi.ctl_base_addr() << 6);
    trace_hdr->phv_base_addr = (uint64_t)(sdp_axi.phv_base_addr() << 6);
    trace_hdr->ring_size = (uint32_t)sdp_axi.ring_size();

    trace_hdr->sw_reset_enable = (uint32_t)sdp_axi_sw_reset.enable();

    trace_hdr->trigger_data = (uint512_t)sdp_trace_trigger.data();
    trace_hdr->trigger_mask = (uint512_t)sdp_trace_trigger.mask();

    //get the write pointers for control and phv ring
    trace_hdr->ctl_ring_wr_ptr = (uint32_t)ctl_ptr.pointer();
    trace_hdr->phv_ring_wr_ptr = (uint32_t)phv_ptr.pointer();
    
}

static inline void
dmatrace_dump_trace_hdr_fill (dmatrace_trace_hdr_t *trace_hdr,
			      elb_top_csr_t &elb0, 
                              int pipeline,
			      std::string mod_name)
{

  elb_prd_csr_cfg_trace_t prd_trace = elb0.pr.pr.prd.cfg_trace;
  elb_ptd_csr_cfg_trace_t ptd_trace = elb0.pt.pt.ptd.cfg_trace;
  elb_prd_csr_sta_trace_t prd_sta_trace = elb0.pr.pr.prd.sta_trace;
  elb_ptd_csr_sta_trace_t ptd_sta_trace = elb0.pt.pt.ptd.sta_trace;

    if (mod_name == "prd") {
      prd_trace.read();
      prd_sta_trace.read();
      DMATRACE_TRACE_HDR_WRITE(prd_sta_trace, prd_trace, pipeline);
	trace_hdr->pkt_phv_sync_err_enable =			      
	(uint32_t)prd_trace.pkt_phv_sync_err_enable();		    
    }
    else {
      ptd_trace.read();
      ptd_sta_trace.read();
      DMATRACE_TRACE_HDR_WRITE(ptd_sta_trace, ptd_trace, pipeline);
    }
}

static inline void
mputrace_dump_trace_hdr_write (elb_mpu_csr_trace_t trace,
                               elb_mpu_csr_watch_pc_t watch_pc,
                               elb_mpu_csr_watch_data_t watch_data,
                               int pipeline, int stage, int mpu, FILE *fp)
{
    mputrace_trace_hdr_t trace_hdr = {};

    mputrace_dump_trace_hdr_fill(&trace_hdr, trace, watch_pc,
                                 watch_data, pipeline, stage, mpu);
    fwrite(&trace_hdr, sizeof(uint8_t), sizeof(trace_hdr), fp);
}

static inline void
sdptrace_dump_trace_hdr_write (elb_sdp_csr_sta_sdp_axi_write_control_t ctl_ptr,
				 elb_sdp_csr_sta_sdp_axi_write_phv_t phv_ptr,
				 elb_sdp_csr_cfg_sdp_axi_t sdp_axi, 
				 elb_sdp_csr_cfg_sdp_axi_sw_reset_t &sdp_axi_sw_reset,
				 elb_sdp_csr_cfg_sdp_trace_trigger_t &sdp_trace_trigger,
				 int pipeline, int stage, FILE *fp)
{
    sdptrace_trace_hdr_t trace_hdr = {};

    sdptrace_dump_trace_hdr_fill(&trace_hdr, ctl_ptr, phv_ptr, sdp_axi, 
				 sdp_axi_sw_reset, sdp_trace_trigger,
				 pipeline, stage);
    fwrite(&trace_hdr, sizeof(uint8_t), sizeof(trace_hdr), fp);
}

static inline void
dmatrace_dump_trace_hdr_write (elb_top_csr_t &elb0, 
			       int pipeline, FILE *fp,
			       std::string mod_name)

{
    dmatrace_trace_hdr_t trace_hdr = {};

    dmatrace_dump_trace_hdr_fill(&trace_hdr, elb0, 
				 pipeline, mod_name);
    fwrite(&trace_hdr, sizeof(uint8_t), sizeof(trace_hdr), fp);
}

static inline void
mputrace_dump_trace_info_write (elb_mpu_csr_trace_t trace, FILE *fp)
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
sdptrace_dump_trace_info_write (elb_sdp_csr_cfg_sdp_axi_t sdp_axi, 
				FILE *fp)
{
  uint8_t buf[64] = {0}; 
  
  uint32_t   ctl_trace_size = (uint32_t)sdp_axi.ring_size();
  mem_addr_t ctl_trace_addr = (mem_addr_t)(sdp_axi.ctl_base_addr() << 6);

  uint32_t   phv_trace_size = ((uint32_t)sdp_axi.ring_size() << 4);
  mem_addr_t phv_trace_addr = (mem_addr_t)(sdp_axi.phv_base_addr() << 6);
  
  //printf ("ctl_trace_size %d\n",ctl_trace_size);
  //printf ("ctl_trace_addr 0x%lx\n",ctl_trace_addr);

  //printf ("phv_trace_size %d\n",phv_trace_size);
  //printf ("phv_trace_addr 0x%lx\n",phv_trace_addr);

  //dump control ring first
  for (uint32_t i = 0; i < ctl_trace_size; i++) {
    sdk::lib::pal_mem_read(ctl_trace_addr, buf, sizeof(buf));
    fwrite(buf, sizeof(buf[0]), sizeof(buf), fp);
    ctl_trace_addr += sizeof(buf);
    //printf ("ctl_trace_addr %d is 0x%lx\n",i, ctl_trace_addr);

  }
  //followed by PHV ring
  for (uint32_t i = 0; i < phv_trace_size; i++) {
    sdk::lib::pal_mem_read(phv_trace_addr, buf, sizeof(buf));
    fwrite(buf, sizeof(buf[0]), sizeof(buf), fp);
    phv_trace_addr += sizeof(buf);
    //printf ("phv_trace_addr %d is 0x%lx\n",i, phv_trace_addr);
  }
}

static inline void
dmatrace_dump_trace_info_write (elb_top_csr_t &elb0,
				FILE *fp,
				std::string mod_name)
{
  uint8_t buf[64] = {0}; 

  elb_prd_csr_cfg_trace_t prd_trace = elb0.pr.pr.prd.cfg_trace;
  elb_ptd_csr_cfg_trace_t ptd_trace = elb0.pt.pt.ptd.cfg_trace;

  prd_trace.read();
  ptd_trace.read();

  uint32_t prd_trace_size = (ELBTRACE_ONE << (uint32_t)prd_trace.buf_size());
  mem_addr_t prd_trace_addr = (mem_addr_t)(prd_trace.base_addr() << 6);

  uint32_t ptd_trace_size = (ELBTRACE_ONE << (uint32_t)ptd_trace.buf_size());
  mem_addr_t ptd_trace_addr = (mem_addr_t)(ptd_trace.base_addr() << 6);

  if (mod_name == "prd") {
    for (uint32_t i = 0; i < prd_trace_size; i++) {
      sdk::lib::pal_mem_read(prd_trace_addr, buf, sizeof(buf));
      fwrite(buf, sizeof(buf[0]), sizeof(buf), fp);
      prd_trace_addr += sizeof(buf);
    }
  }
  else {
    for (uint32_t i = 0; i < ptd_trace_size; i++) {
      sdk::lib::pal_mem_read(ptd_trace_addr, buf, sizeof(buf));
      fwrite(buf, sizeof(buf[0]), sizeof(buf), fp);
      ptd_trace_addr += sizeof(buf);
    }
  }

}

static inline void
mputrace_dump_trace_file_write (elb_mpu_csr_trace_t trace,
                                elb_mpu_csr_watch_pc_t watch_pc,
                                elb_mpu_csr_watch_data_t watch_data,
                                int pipeline, int stage, int mpu)
{
    static FILE *fp = fopen(g_state.ELBTRACE_DUMP_FILE, "wb");

    trace.read();
    watch_pc.read();
    watch_data.read();
    if (fp == NULL) {
        std::cerr << "Failed to open dump file for writing - "
                  << g_state.ELBTRACE_DUMP_FILE << std::endl;
        exit(EXIT_FAILURE);
    }

    mputrace_dump_trace_hdr_write(trace, watch_pc, watch_data,
                                  pipeline, stage, mpu, fp);
    mputrace_dump_trace_info_write(trace, fp);
}

static inline void
sdptrace_dump_trace_file_write (elb_sdp_csr_sta_sdp_axi_write_control_t ctl_ptr,
				elb_sdp_csr_sta_sdp_axi_write_phv_t phv_ptr,
				elb_sdp_csr_cfg_sdp_axi_t sdp_axi, 
				elb_sdp_csr_cfg_sdp_axi_sw_reset_t &sdp_axi_sw_reset,
				elb_sdp_csr_cfg_sdp_trace_trigger_t &sdp_trace_trigger,
                                int pipeline, int stage)
{
    static FILE *fp = fopen(g_state.ELBTRACE_DUMP_FILE, "wb");

    sdp_axi.read();
    ctl_ptr.read();
    phv_ptr.read();
    if (fp == NULL) {
        std::cerr << "Failed to open dump file for writing - "
                  << g_state.ELBTRACE_DUMP_FILE << std::endl;
        exit(EXIT_FAILURE);
    }

    sdptrace_dump_trace_hdr_write(ctl_ptr, phv_ptr, 
				  sdp_axi, sdp_axi_sw_reset,
				  sdp_trace_trigger,
                                  pipeline, stage, fp);
    sdptrace_dump_trace_info_write(sdp_axi, fp);
}

static inline void
dmatrace_dump_trace_file_write (elb_top_csr_t &elb0,
                                int pipeline,
				std::string mod_name)
{
    static FILE *fp = fopen(g_state.ELBTRACE_DUMP_FILE, "wb");

    if (fp == NULL) {
        std::cerr << "Failed to open dump file for writing - "
                  << g_state.ELBTRACE_DUMP_FILE << std::endl;
        exit(EXIT_FAILURE);
    }

    dmatrace_dump_trace_hdr_write(elb0, pipeline,
				  fp, mod_name);
    dmatrace_dump_trace_info_write(elb0, fp, 
				   mod_name);
}

static inline void
mputrace_dump_pipeline_info_fetch (int stage_count, elb_stg_csr_t *stg_ptr,
                                   int pipeline)
{
    for (int stage = 0; stage < stage_count; stage++) {
        for (int mpu = 0; mpu < MPUTRACE_MAX_MPU; mpu++) {
      if (mputrace_util_is_enabled(stg_ptr[stage].mpu[mpu].trace,
                       stg_ptr[stage].mpu[mpu].watch_pc,
                       stg_ptr[stage].mpu[mpu].watch_data)) {
                mputrace_dump_trace_file_write(stg_ptr[stage].mpu[mpu].trace,
                                               stg_ptr[stage].mpu[mpu].watch_pc,
                                               stg_ptr[stage].mpu[mpu].watch_data,
                                               pipeline, stage, mpu);
            }
        }
	//        stg_ptr++;
    }
}

static inline void
sdptrace_dump_pipeline_info_fetch (int stage_count, elb_stg_csr_t *stg_ptr,
                                   int pipeline)
{

  for (int stage = 0; stage < stage_count; stage++) {
    
    stg_ptr[stage].sdp.cfg_sdp_axi.read();
    if (stg_ptr[stage].sdp.cfg_sdp_axi.int_var__enable) {
      sdptrace_dump_trace_file_write(stg_ptr[stage].sdp.sta_sdp_axi_write_control,
				     stg_ptr[stage].sdp.sta_sdp_axi_write_phv,
				     stg_ptr[stage].sdp.cfg_sdp_axi,
				     stg_ptr[stage].sdp.cfg_sdp_axi_sw_reset,
				     stg_ptr[stage].sdp.cfg_sdp_trace_trigger,
				     pipeline, stage);
    }
  }
}
  
static inline void
mputrace_dump_all_pipelines (void)
{
    int stage_count = 0;
    elb_top_csr_t &elb0 = ELB_BLK_REG_MODEL_ACCESS(elb_top_csr_t, 0, 0);

    ELBTRACE_FOR_EACH_PIPELINE(elb0.pct, mputrace_dump_pipeline_info_fetch, 0);
    ELBTRACE_FOR_EACH_PIPELINE(elb0.pcr, mputrace_dump_pipeline_info_fetch, 1);
    ELBTRACE_FOR_EACH_PIPELINE(elb0.sgi, mputrace_dump_pipeline_info_fetch, 2);
    ELBTRACE_FOR_EACH_PIPELINE(elb0.sge, mputrace_dump_pipeline_info_fetch, 3);
}

static inline void
sdptrace_dump_all_pipelines (void)
{
    int stage_count = 0;
    elb_top_csr_t &elb0 = ELB_BLK_REG_MODEL_ACCESS(elb_top_csr_t, 0, 0);

    ELBTRACE_FOR_EACH_PIPELINE(elb0.pct, sdptrace_dump_pipeline_info_fetch, 0);
    ELBTRACE_FOR_EACH_PIPELINE(elb0.pcr, sdptrace_dump_pipeline_info_fetch, 1);
    ELBTRACE_FOR_EACH_PIPELINE(elb0.sgi, sdptrace_dump_pipeline_info_fetch, 2);
    ELBTRACE_FOR_EACH_PIPELINE(elb0.sge, sdptrace_dump_pipeline_info_fetch, 3);
}

static inline void
dmatrace_dump_all_pipelines (void)
{
    int stage_count = 0;
    elb_top_csr_t &elb0 = ELB_BLK_REG_MODEL_ACCESS(elb_top_csr_t, 0, 0);

    dmatrace_dump_trace_file_write(elb0, 0, "prd"); 
    dmatrace_dump_trace_file_write(elb0, 1, "ptd"); 
}

void
elbtrace_dump (const char *dump_file, 
	       std::string mod_name)
{
    snprintf(g_state.ELBTRACE_DUMP_FILE, ELBTRACE_STR_NAME_LEN, "%s",
             dump_file);
    if ( !((mod_name == "mpu") || 
	   (mod_name == "sdp") || 
	   (mod_name == "dma")) ) {
      cout << "Module name did not match mpu, sdp or dma. Incorrect argument." << endl;
    }
    else {
      if (mod_name == "mpu") {
	mputrace_dump_all_pipelines();
      } 
      else if (mod_name == "sdp") {
	sdptrace_dump_all_pipelines();
      }
      else if (mod_name == "dma") {
	dmatrace_dump_all_pipelines();
      }
      cout << "'elbtrace dump " << g_state.ELBTRACE_DUMP_FILE << "' success!"
	   << endl;
    }
}


//
// mputrace reset routines
//

static inline void
mputrace_reset_buffer_erase (elb_mpu_csr_trace_t trace,
                             elb_mpu_csr_watch_pc_t watch_pc,
                             elb_mpu_csr_watch_data_t watch_data)
{
    mem_addr_t trace_addr = 0;
    uint32_t trace_size = 0;

    trace.read();
    watch_pc.read();
    watch_data.read();
    if (mputrace_util_is_enabled(trace, watch_pc, watch_data)) {
        trace_addr = mputrace_util_base_addr_get(trace);
        trace_size = mputrace_util_size_get(trace);
        sdk::lib::pal_mem_set(trace_addr, 0, trace_size * TRACE_ENTRY_SIZE);
    }
}

static inline void
mputrace_reset_pipeline (int stage_count, elb_stg_csr_t *stg_ptr,
                         mputrace_cfg_inst_t *cfg_inst)
{
    for (int s = 0; s < stage_count; s++) {
        for (int mpu = 0; mpu < MPUTRACE_MAX_MPU; mpu++) {
      mputrace_reset_buffer_erase(stg_ptr[s].mpu[mpu].trace,
                      stg_ptr[s].mpu[mpu].watch_pc,
                      stg_ptr[s].mpu[mpu].watch_data);
            mputrace_util_reg_program(stg_ptr[s].mpu[mpu].trace,
                      stg_ptr[s].mpu[mpu].watch_pc,
                      stg_ptr[s].mpu[mpu].watch_data,
                      cfg_inst, 0, false);
	    //        printf("%10" PRIu32
	    //" %10" PRIu32 "\n",
            //   s,
	    //mpu);
        }
        //stg_ptr++;
    }
}

static inline void
mputrace_reset_all_pipelines (elb_top_csr_t &elb0)
{
    int stage_count = 0;
    mputrace_cfg_inst_t cfg_inst;

    memset(&cfg_inst, 0, sizeof(cfg_inst));
    cfg_inst.settings.reset = true;
    cout << "SGI" << endl;
    ELBTRACE_FOR_EACH_PIPELINE(elb0.sgi, mputrace_reset_pipeline, &cfg_inst);
    cout << "SGE" << endl;
    ELBTRACE_FOR_EACH_PIPELINE(elb0.sge, mputrace_reset_pipeline, &cfg_inst);
    cout << "PCT" << endl;
    ELBTRACE_FOR_EACH_PIPELINE(elb0.pct, mputrace_reset_pipeline, &cfg_inst);
    cout << "PCR" << endl;
    ELBTRACE_FOR_EACH_PIPELINE(elb0.pcr, mputrace_reset_pipeline, &cfg_inst);
}

//
// sdptrace reset routines
//

static inline void
sdptrace_reset_buffer_erase (elb_sdp_csr_cfg_sdp_axi_t sdp_axi)
{
    mem_addr_t phv_base_addr = 0;
    mem_addr_t ctl_base_addr = 0;
    uint32_t ring_size = 0;

    sdp_axi.read();

    if ( (int)sdp_axi.int_var__enable) {
        phv_base_addr = (mem_addr_t)(sdp_axi.phv_base_addr() << 6);
        ctl_base_addr = (mem_addr_t)(sdp_axi.ctl_base_addr() << 6);
	//        ring_size = (ELBTRACE_ONE << (uint32_t)sdp_axi.ring_size());
        ring_size = (uint32_t)sdp_axi.ring_size();
        sdk::lib::pal_mem_set(phv_base_addr, 0, ring_size * SDPTRACE_PHV_ENTRY_SIZE);
        sdk::lib::pal_mem_set(ctl_base_addr, 0, ring_size * SDPTRACE_CTL_ENTRY_SIZE);
    }
}

static inline void
sdptrace_reset_pipeline (int stage_count, elb_stg_csr_t *stg_ptr,
                         sdptrace_cfg_inst_t *cfg_inst)
{
    for (int s = 0; s < stage_count; s++) {
      //      cout << "SDP reset pipeline 1" << endl;
      sdptrace_reset_buffer_erase(stg_ptr[s].sdp.cfg_sdp_axi);
      //      cout << "SDP reset pipeline 2" << endl;
      sdptrace_util_reg_program(stg_ptr[s].sdp.cfg_sdp_axi,
				stg_ptr[s].sdp.cfg_sdp_axi_sw_reset,
				stg_ptr[s].sdp.cfg_sdp_trace_trigger,
				cfg_inst, 0, 0, false);
      //      cout << "SDP reset pipeline 3" << endl;
      //        printf("%10" PRIu32"\n", s);
    }
}

static inline void
sdptrace_reset_all_pipelines (elb_top_csr_t &elb0)
{
    int stage_count = 0;
    sdptrace_cfg_inst_t cfg_inst;

    memset(&cfg_inst, 0, sizeof(cfg_inst));
    cfg_inst.settings.sw_reset_enable = true;
    cfg_inst.ctrl.trace_trigger_enable = false;
    //    cout << "SGI SDP" << endl;
    ELBTRACE_FOR_EACH_PIPELINE(elb0.sgi, sdptrace_reset_pipeline, &cfg_inst);
    //    cout << "SGE SDP" << endl;
    ELBTRACE_FOR_EACH_PIPELINE(elb0.sge, sdptrace_reset_pipeline, &cfg_inst);
    //    cout << "PCT SDP" << endl;
    ELBTRACE_FOR_EACH_PIPELINE(elb0.pct, sdptrace_reset_pipeline, &cfg_inst);
    //    cout << "PCR SDP" << endl;
    ELBTRACE_FOR_EACH_PIPELINE(elb0.pcr, sdptrace_reset_pipeline, &cfg_inst);
}

//
// dmatrace reset routines
//

static inline void
dmatrace_reset_buffer_erase (elb_top_csr_t &elb0, 
			     std::string mod_name)
{
    mem_addr_t base_addr = 0;
    uint32_t   buf_size = 0;
    
    elb_prd_csr_cfg_trace_t prd_trace = elb0.pr.pr.prd.cfg_trace;
    elb_ptd_csr_cfg_trace_t ptd_trace = elb0.pt.pt.ptd.cfg_trace;

    if (mod_name == "prd") {
      prd_trace.read();
      
      if ( (int)prd_trace.int_var__enable) {
        base_addr = (mem_addr_t)(prd_trace.base_addr() << 6);
        buf_size = (ELBTRACE_ONE << (uint32_t)prd_trace.buf_size());
        sdk::lib::pal_mem_set(base_addr, 0, buf_size * DMATRACE_ENTRY_SIZE);
      }
    }
    else if (mod_name == "ptd") {
      ptd_trace.read();
      
      if ( (int)ptd_trace.int_var__enable) {
        base_addr = (mem_addr_t)(ptd_trace.base_addr() << 6);
        buf_size = (ELBTRACE_ONE << (uint32_t)ptd_trace.buf_size());
        sdk::lib::pal_mem_set(base_addr, 0, buf_size * DMATRACE_ENTRY_SIZE);
      }
    }
}

static inline void
dmatrace_reset_pipeline (elb_top_csr_t &elb0,
                         dmatrace_cfg_inst_t *cfg_inst,
			 std::string mod_name)
{
  cout << "entering dmatrace_trace_buffer_erase" << endl;
  dmatrace_reset_buffer_erase(elb0, mod_name);
  cout << "leaving dmatrace_trace_buffer_erase" << endl;
  dmatrace_util_reg_program(elb0, cfg_inst, 0, false, mod_name);
  cout << "leaving dmatrace_util_reg_program" << endl;
}

static inline void
dmatrace_reset_all_pipelines (elb_top_csr_t &elb0)
{
    int stage_count = 0;
    dmatrace_cfg_inst_t cfg_inst;

    memset(&cfg_inst, 0, sizeof(cfg_inst));
    cfg_inst.settings.reset = true;
    cout << "PTD" << endl;
    dmatrace_reset_pipeline(elb0, &cfg_inst, "ptd");
    cout << "PRD" << endl;
    dmatrace_reset_pipeline(elb0, &cfg_inst, "prd");
}

void
mputrace_reset (void)
{
    elb_top_csr_t &elb0 = ELB_BLK_REG_MODEL_ACCESS(elb_top_csr_t, 0, 0);
    mputrace_reset_all_pipelines(elb0);
    cout << "elbtrace MPU reset success" << endl;
}

void
sdptrace_reset (void)
{
    elb_top_csr_t &elb0 = ELB_BLK_REG_MODEL_ACCESS(elb_top_csr_t, 0, 0);
    sdptrace_reset_all_pipelines(elb0);
    cout << "elbtrace SDP reset success" << endl;
}

void
dmatrace_reset (void)
{
    elb_top_csr_t &elb0 = ELB_BLK_REG_MODEL_ACCESS(elb_top_csr_t, 0, 0);
    dmatrace_reset_all_pipelines(elb0);
    cout << "elbtrace DMA reset success" << endl;
}

//
// mputrace show routines
//

static inline void
mputrace_show_pipeline_internal_0 (elb_mpu_csr_trace_t trace,
                                 elb_mpu_csr_watch_pc_t watch_pc,
                                 elb_mpu_csr_watch_data_t watch_data,
                                 mputrace_cfg_inst_t *cfg_inst, int stage,
				   int mpu, int pipeline)
{
    trace.read();
    watch_pc.read();
    watch_data.read();
    //    cout << "elbtrace show_pipeline_internal" << endl;


    if (mputrace_util_is_enabled(trace, watch_pc, watch_data)) {
      printf("%10s"
	     "%10" PRIu32
	     " %10" PRIu32
	     " %10" PRIu8
	     " %10" PRIu8
	     " %10" PRIu8
	     " %10" PRIu8
	     " %10" PRIu8
	     " %10" PRIu8
	     " %10" PRIu8
	     " %10" PRIu8
	     "  "
	     " 0x%08" PRIx64
	     " %8" PRIu32
	     " %10" PRIu32 "\n",
	     // selectors
	     mputrace_pipeline_str_get(pipeline).c_str(),
	     stage,
	     mpu,
	     // ctrl
	     (uint32_t)trace.enable(),
	     (uint32_t)trace.wrap(),
	     (uint32_t)trace.rst(),
	     // ctrl
	     (uint32_t)trace.trace_enable(),
	     (uint32_t)trace.phv_debug(),
	     (uint32_t)trace.phv_error(),
	     // content
	     (uint32_t)trace.table_and_key(),
	     (uint32_t)trace.instructions(),
	     // location
	     (long unsigned int)(trace.base_addr() << 6),
	     (uint32_t)(ELBTRACE_ONE << (uint32_t)trace.buf_size()),
	     (uint32_t)(TRACE_ENTRY_SIZE *
			(ELBTRACE_ONE << (uint32_t)trace.buf_size())));
      
    };

}

static inline void
mputrace_show_pipeline_internal_1 (elb_mpu_csr_trace_t trace,
                                 elb_mpu_csr_watch_pc_t watch_pc,
                                 elb_mpu_csr_watch_data_t watch_data,
                                 mputrace_cfg_inst_t *cfg_inst, int stage,
				   int mpu, int pipeline)
{
    trace.read();
    watch_pc.read();
    watch_data.read();
    //    cout << "elbtrace show_pipeline_internal" << endl;



    if (mputrace_util_is_enabled(trace, watch_pc, watch_data)) {
      printf("%10s"
	     "%10" PRIu32
	     " %10" PRIu32
	     " %10" PRIu8
	     " %10" PRIu8
	     " %10" PRIu8
	     " %10" PRIu8
	     " %10" PRIu8
	     "    "
	     " 0x%08" PRIx64
	     " 0x%08" PRIx64 "\n",
	     // selectors
	     mputrace_pipeline_str_get(pipeline).c_str(),
	     stage,
	     mpu,
	     //WATCH_PC
	     (uint32_t)watch_pc.trace(),
	     (uint32_t)watch_pc.count(),
	     (uint32_t)watch_pc.intr(),
	     (uint32_t)watch_pc.stop(),
	     (uint32_t)watch_pc.exception(),
	     (uint64_t)(watch_pc.addr_lo() << 3),
	     (uint64_t)(watch_pc.addr_hi() << 3));
    };

}

static inline void
mputrace_show_pipeline_internal_2 (elb_mpu_csr_trace_t trace,
                                 elb_mpu_csr_watch_pc_t watch_pc,
                                 elb_mpu_csr_watch_data_t watch_data,
                                 mputrace_cfg_inst_t *cfg_inst, int stage,
				   int mpu, int pipeline)
{
    trace.read();
    watch_pc.read();
    watch_data.read();
    //    cout << "elbtrace show_pipeline_internal" << endl;




    if (mputrace_util_is_enabled(trace, watch_pc, watch_data)) {
      printf("%10s"
	     "%10" PRIu32
	     " %10" PRIu32
	     " %10" PRIu8
	     " %10" PRIu8
	     " %10" PRIu8
	     " %10" PRIu8
	     " %10" PRIu8
	     " %10" PRIu8
	     " %10" PRIu8
	     " %10" PRIu8
	     "       "
	     " 0x%08" PRIx64
	     " 0x%08" PRIx64 "\n",
	     // selectors
	     mputrace_pipeline_str_get(pipeline).c_str(),
	     stage,
	     mpu,
	     ///WATCH_DATA
	     (uint32_t)watch_data.rtrace(),
	     (uint32_t)watch_data.wtrace(),
	     (uint32_t)watch_data.rintr(),
	     (uint32_t)watch_data.wintr(),
	     (uint32_t)watch_data.rstop(),
	     (uint32_t)watch_data.wstop(),
	     (uint32_t)watch_data.rexception(),
	     (uint32_t)watch_data.wexception(),
	     (uint64_t)watch_data.addr_lo(),
	     (uint64_t)watch_data.addr_hi());
    };
}

static inline void
mputrace_show_pipeline_0 (int stage_count, elb_stg_csr_t *stg_ptr,
			  mputrace_cfg_inst_t *cfg_inst, int pipeline)
{
    g_state.pipeline_index++;
    for (int stage = 0; stage < stage_count; stage++) {
        for (int mpu = 0; mpu < MPUTRACE_MAX_MPU; mpu++) {
            mputrace_show_pipeline_internal_0(stg_ptr[stage].mpu[mpu].trace,
                        stg_ptr[stage].mpu[mpu].watch_pc,
                        stg_ptr[stage].mpu[mpu].watch_data,
                        cfg_inst,
					      stage, mpu, pipeline);
        }
    }
}

static inline void
mputrace_show_pipeline_1 (int stage_count, elb_stg_csr_t *stg_ptr,
			  mputrace_cfg_inst_t *cfg_inst, int pipeline)
{
    g_state.pipeline_index++;
    for (int stage = 0; stage < stage_count; stage++) {
        for (int mpu = 0; mpu < MPUTRACE_MAX_MPU; mpu++) {
            mputrace_show_pipeline_internal_1(stg_ptr[stage].mpu[mpu].trace,
                        stg_ptr[stage].mpu[mpu].watch_pc,
                        stg_ptr[stage].mpu[mpu].watch_data,
                        cfg_inst,
					      stage, mpu, pipeline);
        }
    }
}

static inline void
mputrace_show_pipeline_2 (int stage_count, elb_stg_csr_t *stg_ptr,
			  mputrace_cfg_inst_t *cfg_inst, int pipeline)
{
    g_state.pipeline_index++;
    for (int stage = 0; stage < stage_count; stage++) {
        for (int mpu = 0; mpu < MPUTRACE_MAX_MPU; mpu++) {
            mputrace_show_pipeline_internal_2(stg_ptr[stage].mpu[mpu].trace,
                        stg_ptr[stage].mpu[mpu].watch_pc,
                        stg_ptr[stage].mpu[mpu].watch_data,
                        cfg_inst,
					      stage, mpu, pipeline);
        }
    }
}

static inline void
mputrace_show_all_pipelines (void)
{
    int stage_count = 0;
    elb_top_csr_t &elb0 = ELB_BLK_REG_MODEL_ACCESS(elb_top_csr_t, 0, 0);
    mputrace_cfg_inst_t cfg_inst;

    printf("%10s %10s %10s %10s %10s %10s %10s %10s %10s %10s "
           "%10s %10s %10s %10s\n",
           "pipeline", "stage", "mpu", "enable", "wrap",
           "reset", "trace", "phv_debug", "phv_error", 
	   "table_kd", "instr", "trace_addr", "trace_nent",
           "trace_sz");

    ELBTRACE_FOR_EACH_PIPELINE_SHOW(elb0.sgi, mputrace_show_pipeline_0, &cfg_inst, 0);
    ELBTRACE_FOR_EACH_PIPELINE_SHOW(elb0.sge, mputrace_show_pipeline_0, &cfg_inst, 1);
    ELBTRACE_FOR_EACH_PIPELINE_SHOW(elb0.pcr, mputrace_show_pipeline_0, &cfg_inst, 2);
    ELBTRACE_FOR_EACH_PIPELINE_SHOW(elb0.pct, mputrace_show_pipeline_0, &cfg_inst, 3);

    printf("\n%10s %10s %10s %10s %10s %10s %10s %10s %11s %11s\n",
	   "pipeline", "stage", "mpu", "wpc_trace", "wpc_count", "wpc_intr", "wpc_stop", 
	   "wpc_exception", "wpc_addr_lo", "wpc_addr_hi");
    
    ELBTRACE_FOR_EACH_PIPELINE_SHOW(elb0.sgi, mputrace_show_pipeline_1, &cfg_inst, 0);
    ELBTRACE_FOR_EACH_PIPELINE_SHOW(elb0.sge, mputrace_show_pipeline_1, &cfg_inst, 1);
    ELBTRACE_FOR_EACH_PIPELINE_SHOW(elb0.pcr, mputrace_show_pipeline_1, &cfg_inst, 2);
    ELBTRACE_FOR_EACH_PIPELINE_SHOW(elb0.pct, mputrace_show_pipeline_1, &cfg_inst, 3);

    printf("\n%10s %10s %10s %10s %10s %10s %10s %10s %10s %10s %10s %10s %10s\n",
	   "pipeline", "stage", "mpu", "wd_rtrace",  "wd_wtrace", "wd_rintr", "wd_wintr",
	   "wd_rstop", "wd_wstop", "wd_rexception", "wd_wexception", 
	   "wd_addr_lo", "wd_addr_hi");
    
    ELBTRACE_FOR_EACH_PIPELINE_SHOW(elb0.sgi, mputrace_show_pipeline_2, &cfg_inst, 0);
    ELBTRACE_FOR_EACH_PIPELINE_SHOW(elb0.sge, mputrace_show_pipeline_2, &cfg_inst, 1);
    ELBTRACE_FOR_EACH_PIPELINE_SHOW(elb0.pcr, mputrace_show_pipeline_2, &cfg_inst, 2);
    ELBTRACE_FOR_EACH_PIPELINE_SHOW(elb0.pct, mputrace_show_pipeline_2, &cfg_inst, 3);
}

void
mputrace_show (void)
{
    mputrace_show_all_pipelines();
    cout << "elbtrace show success" << endl;
}

//
// sdptrace show routines
//

static inline void
sdptrace_show_pipeline_internal (elb_sdp_csr_cfg_sdp_axi_t &sdp_axi,
				 elb_sdp_csr_cfg_sdp_axi_sw_reset_t &sdp_axi_sw_reset,
				 elb_sdp_csr_cfg_sdp_trace_trigger_t &sdp_trace_trigger,
                                 sdptrace_cfg_inst_t *cfg_inst, int stage)
{
    sdp_axi.read();
    sdp_axi_sw_reset.read();
    sdp_trace_trigger.read();
    //    cout << "elbtrace show_pipeline_internal" << endl;

    if ( (int)sdp_axi.int_var__enable ) {
        printf("%10s"
               "%10" PRIu32
           " %10" PRIu8
           " %10" PRIu8
           " %10" PRIu8
           " %12" PRIu8
           " %18" PRIu8
	       //           " 0x%08" PRIx512    todo
	       //           " 0x%08" PRIx512    todo
           "        "
           " 0x%08" PRIx64
           "   "
           " 0x%08" PRIx64
           " %10" PRIu32 "\n",

               mputrace_pipeline_str_get(g_state.pipeline_index - 1).c_str(),
               stage,
	       (uint32_t)sdp_axi.enable(),
	       (uint32_t)sdp_trace_trigger.enable(),
	       (uint32_t)sdp_axi_sw_reset.enable(),
	       (uint32_t)sdp_axi.no_trace_when_full(),
	       (uint32_t)sdp_axi.stop_when_full(),
	       //todo:
	       //	       (uint512_t)sdp_trace_trigger.data(),
	       //(uint512_t)sdp_trace_trigger.mask(),
	       (long unsigned int)(sdp_axi.ctl_base_addr() << 6),
	       (long unsigned int)(sdp_axi.phv_base_addr() << 6),
               (uint32_t)(sdp_axi.ring_size()));
	       //               (uint32_t)(ELBTRACE_ONE << sdp_axi.ring_size()));

    }
}

static inline void
sdptrace_show_pipeline (int stage_count, elb_stg_csr_t *stg_ptr,
                        sdptrace_cfg_inst_t *cfg_inst)
{
    g_state.pipeline_index++;
    for (int stage = 0; stage < stage_count; stage++) {
      sdptrace_show_pipeline_internal(stg_ptr[stage].sdp.cfg_sdp_axi,	    
				      stg_ptr[stage].sdp.cfg_sdp_axi_sw_reset, 
				      stg_ptr[stage].sdp.cfg_sdp_trace_trigger,
				      cfg_inst,
				      stage);
    }
}

static inline void
sdptrace_show_all_pipelines (void)
{
    int stage_count = 0;
    elb_top_csr_t &elb0 = ELB_BLK_REG_MODEL_ACCESS(elb_top_csr_t, 0, 0);
    sdptrace_cfg_inst_t cfg_inst;

    ELBTRACE_FOR_EACH_PIPELINE(elb0.sgi, sdptrace_show_pipeline, &cfg_inst);
    ELBTRACE_FOR_EACH_PIPELINE(elb0.sge, sdptrace_show_pipeline, &cfg_inst);
    ELBTRACE_FOR_EACH_PIPELINE(elb0.pcr, sdptrace_show_pipeline, &cfg_inst);
    ELBTRACE_FOR_EACH_PIPELINE(elb0.pct, sdptrace_show_pipeline, &cfg_inst);
}

void
sdptrace_show (void)
{
  //todo: print trigger_data/mask
    printf("%10s %10s %10s %10s %10s %10s %10s %10s "
           "%10s %10s\n",
           "pipeline", "stage", "enable", "trace_trig_en",
           "sw_reset_en", "no_trace_when_full", "stop_when_full", "phv_base_addr", "ctl_base_addr",
           "ring_sz");

    sdptrace_show_all_pipelines();

    //   cout << "size of  sdptrace header is " << dec << sizeof(sdptrace_trace_hdr_t) << endl;

    cout << "elbtrace show success" << endl;
}


//
// dmatrace show routines
//

static inline void
dmatrace_show_pipeline_internal (elb_top_csr_t &elb0,
				 std::string mod_name)
{
  elb_prd_csr_cfg_trace_t prd_trace = elb0.pr.pr.prd.cfg_trace;
  elb_ptd_csr_cfg_trace_t ptd_trace = elb0.pt.pt.ptd.cfg_trace;

    prd_trace.read();
    ptd_trace.read();
    //    cout << "elbtrace show_pipeline_internal" << endl;

    if (mod_name == "prd") {
      if ( (int)prd_trace.int_var__enable ) {
        printf("%10s"
               "%10" PRIu32
           " %10" PRIu8
           " %10" PRIu8
           " %10" PRIu8
           " %15" PRIu8
           " %23" PRIu8
           " %10" PRIu8
	   "  "    
           " 0x%08" PRIx64
           " %7" PRIu32 "\n",
               "prd",
	       (uint32_t)prd_trace.enable(),
	       (uint32_t)prd_trace.phv_enable(),
	       (uint32_t)prd_trace.capture_all(),
	       (uint32_t)prd_trace.axi_err_enable(),
	       (uint32_t)prd_trace.pkt_phv_sync_err_enable(),
	       (uint32_t)prd_trace.wrap(),
	       (uint32_t)prd_trace.rst(),
	       (long unsigned int)(prd_trace.base_addr() << 6),
               (uint32_t)(prd_trace.buf_size()));
	//               (uint32_t)(ELBTRACE_ONE << (uint32_t)prd_trace.buf_size()));
      }

    }
    else {
      if ( (int)prd_trace.int_var__enable ) {
        printf("%10s"
               "%10" PRIu32
           " %10" PRIu8
           " %10" PRIu8
           " %10" PRIu8
           " %15s"
           " %23" PRIu8
           " %10" PRIu8
	   "  "    
           " 0x%08" PRIx64
           " %7" PRIu32 "\n",
               "ptd",
	       (uint32_t)ptd_trace.enable(),
	       (uint32_t)ptd_trace.phv_enable(),
	       (uint32_t)ptd_trace.capture_all(),
	       (uint32_t)ptd_trace.axi_err_enable(),
	       "NA",
	       (uint32_t)ptd_trace.wrap(),
	       (uint32_t)ptd_trace.rst(),
	       (long unsigned int)(ptd_trace.base_addr() << 6),
               (uint32_t)(ptd_trace.buf_size()));
	//               (uint32_t)(ELBTRACE_ONE << (uint32_t)ptd_trace.buf_size()));
      }
      
    }
}


void
dmatrace_show (void)
{
  elb_top_csr_t &elb0 = ELB_BLK_REG_MODEL_ACCESS(elb_top_csr_t, 0, 0);

  printf("%10s %10s %10s %10s %10s %10s %10s %10s %10s %10s\n",
	 "pipeline", "enable", "phv_enable",
	 "capture_all", "axi_err_enable", "pkt_phv_sync_err_enable", 
	 "wrap", "reset", "trace_addr",
	 "buf_sz");
  
  dmatrace_show_pipeline_internal(elb0, "prd");
  dmatrace_show_pipeline_internal(elb0, "ptd");
  
    cout << "elbtrace show success" << endl;
}



}    // end namespace platform
}    // end namespace sdk
