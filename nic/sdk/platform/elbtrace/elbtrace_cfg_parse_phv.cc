//
// {C} Copyright 2020 Pensando Systems Inc. All rights reserved
//
//===----------------------------------------------------------------------===//
///
/// \file
/// This file contains implementation to parse mputrace cfg from a json file
///
//===----------------------------------------------------------------------===//

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <regex>
#include <string>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "platform/elbtrace/elbtrace.hpp"

using boost::property_tree::ptree;
using std::string;
using namespace std;

namespace sdk {
namespace platform {

#define MPUTRACE_MATCH_ALL ".*"

  /*
#define SDPTRACE_COPY_TRIGGER(cfg_inst, cfg_phv_data, cfg_phv_mask)			\
    for (int i=0; i<8; i++) {                                                                              \
      cfg_inst->capture.trigger_data[8-i-1] = *((uint64_t*) (&cfg_phv_data + i*8)); \
      cout << "data " << i << " is " << hex << cfg_inst->capture.trigger_data[i] << endl;\
    }      

#define SDPTRACE_COPY_CHECK(cfg_inst, cfg_phv_data, cfg_phv_mask)			\
  for (int i=0; i < 64; i+=8) {                                                         \
  *((size_t*) (((char*) &cfg_phv_data + i))) = cfg_inst->capture.trigger_data[i/8];     \
  *((size_t*) (((char*) &cfg_phv_mask + i))) = cfg_inst->capture.trigger_mask[i/8];     \
}

  */

static inline void
sdptrace_json_phv_global_parse (ptree &pt, elb_phv_intr_global_c_t *cfg_inst)
{
    boost::optional<bool> val;
    boost::optional<std::string> str;

    //Intialize the fields
      cfg_inst->tm_iport = 0;
      cfg_inst->tm_oport = 0;
      cfg_inst->tm_iq = 0;
      cfg_inst->lif = 0;
      cfg_inst->timestamp = 0;
      cfg_inst->tm_span_session = 0;
      cfg_inst->tm_replicate_ptr = 0;
      cfg_inst->tm_replicate_en = 0;
      cfg_inst->tm_cpu = 0;
      cfg_inst->tm_q_depth = 0;
      cfg_inst->drop = 0;
      cfg_inst->bypass = 0;
      cfg_inst->hw_error = 0;
      cfg_inst->tm_oq = 0;
      cfg_inst->debug_trace = 0;
      cfg_inst->csum_err = 0;
      cfg_inst->error_bits = 0;
      cfg_inst->tm_instance_type = 0;

    // Get fields from ptree and populate the struct
    str = pt.get_optional<std::string>("tm_iport");
    if (str) {
      cfg_inst->tm_iport = strtoul(str.get().data(), NULL, 16);
    }
  
    str = pt.get_optional<std::string>("tm_oport");
    if (str) {
      cfg_inst->tm_oport = strtoul(str.get().data(), NULL, 16);
    }
  
    str = pt.get_optional<std::string>("tm_iq");
    if (str) {
      cfg_inst->tm_iq = strtoul(str.get().data(), NULL, 16);
    }
  
    str = pt.get_optional<std::string>("lif");
    if (str) {
      cfg_inst->lif = strtoul(str.get().data(), NULL, 16);
    }
  
    str = pt.get_optional<std::string>("timestamp");
    if (str) {
      cfg_inst->timestamp = strtoul(str.get().data(), NULL, 16);
    }
  
    str = pt.get_optional<std::string>("tm_span_session");
    if (str) {
      cfg_inst->tm_span_session = strtoul(str.get().data(), NULL, 16);
    }
  
    str = pt.get_optional<std::string>("tm_replicate_ptr");
    if (str) {
      cfg_inst->tm_replicate_ptr = strtoul(str.get().data(), NULL, 16);
    }
  
    str = pt.get_optional<std::string>("tm_replicate_en");
    if (str) {
      cfg_inst->tm_replicate_en = strtoul(str.get().data(), NULL, 16);
    }
  
    str = pt.get_optional<std::string>("tm_cpu");
    if (str) {
      cfg_inst->tm_cpu = strtoul(str.get().data(), NULL, 16);
    }
  
    str = pt.get_optional<std::string>("tm_q_depth");
    if (str) {
      cfg_inst->tm_q_depth = strtoul(str.get().data(), NULL, 16);
    }
  
    str = pt.get_optional<std::string>("drop");
    if (str) {
      cfg_inst->drop = strtoul(str.get().data(), NULL, 16);
    }
  
    str = pt.get_optional<std::string>("bypass");
    if (str) {
      cfg_inst->bypass = strtoul(str.get().data(), NULL, 16);
    }
  
    str = pt.get_optional<std::string>("hw_error");
    if (str) {
      cfg_inst->hw_error = strtoul(str.get().data(), NULL, 16);
    }
  
    str = pt.get_optional<std::string>("tm_oq");
    if (str) {
      cfg_inst->tm_oq = strtoul(str.get().data(), NULL, 16);
    }
  
    str = pt.get_optional<std::string>("debug_trace");
    if (str) {
      cfg_inst->debug_trace = strtoul(str.get().data(), NULL, 16);
    }
  
    str = pt.get_optional<std::string>("csum_err");
    if (str) {
      cfg_inst->csum_err = strtoul(str.get().data(), NULL, 16);
    }
  
    str = pt.get_optional<std::string>("error_bits");
    if (str) {
      cfg_inst->error_bits = strtoul(str.get().data(), NULL, 16);
    }
  
    str = pt.get_optional<std::string>("tm_instance_type");
    if (str) {
      cfg_inst->tm_instance_type = strtoul(str.get().data(), NULL, 16);
    }

}

static inline void
sdptrace_json_phv_p4_parse (ptree &pt, elb_phv_intr_p4_c_t *cfg_inst, elb_phv_intr_global_c_t cfg_inst_global)
{
    boost::optional<bool> val;
    boost::optional<std::string> str;

    //Intialize the fields
      cfg_inst->crc_err = 0;
      cfg_inst->len_err = 0;
      cfg_inst->recirc_count = 0;
      cfg_inst->parser_err = 0;
      cfg_inst->crypto_hdr = 0;
      cfg_inst->frame_size = 0;
      cfg_inst->no_data = 0;
      cfg_inst->recirc = 0;
      cfg_inst->packet_len = 0;
      cfg_inst->csum_error_hi = 0;
      cfg_inst->deparser_csum_disable = 0;
      cfg_inst->deparser_crc_disable = 0;
      cfg_inst->roce_table_ptr = 0;
      cfg_inst->roce_csum_table = 0;
      cfg_inst->roce_enable = 0;
      cfg_inst->payload_start = 0;
      cfg_inst->payload_size = 0;
      cfg_inst->auth_result = 0;
      cfg_inst->elb_align_pad = 0;
      cfg_inst->stage_skip = 0;
      cfg_inst->phv_id = 0;
      cfg_inst->app_type = 0;
      cfg_inst->elb_p4_rsv = 0;
      cfg_inst->padding_1 = 0;
      cfg_inst->padding_0 = 0;
      cfg_inst->hdr_vld_1 = 0;
      cfg_inst->hdr_vld_0 = 0;

      cfg_inst->phv_global = cfg_inst_global;
      
    str = pt.get_optional<std::string>("crc_err");
    if (str) {
      cfg_inst->crc_err = strtoul(str.get().data(), NULL, 16);
    }
  
    str = pt.get_optional<std::string>("len_err");
    if (str) {
      cfg_inst->len_err = strtoul(str.get().data(), NULL, 16);
    }
  
    str = pt.get_optional<std::string>("recirc_count");
    if (str) {
      cfg_inst->recirc_count = strtoul(str.get().data(), NULL, 16);
    }
  
    str = pt.get_optional<std::string>("parser_err");
    if (str) {
      cfg_inst->parser_err = strtoul(str.get().data(), NULL, 16);
    }
  
    str = pt.get_optional<std::string>("crypto_hdr");
    if (str) {
      cfg_inst->crypto_hdr = strtoul(str.get().data(), NULL, 16);
    }
  
    str = pt.get_optional<std::string>("frame_size");
    if (str) {
      cfg_inst->frame_size = strtoul(str.get().data(), NULL, 16);
    }
  
    str = pt.get_optional<std::string>("no_data");
    if (str) {
      cfg_inst->no_data = strtoul(str.get().data(), NULL, 16);
    }
  
    str = pt.get_optional<std::string>("recirc");
    if (str) {
      cfg_inst->recirc = strtoul(str.get().data(), NULL, 16);
    }
  
    str = pt.get_optional<std::string>("packet_len");
    if (str) {
      cfg_inst->packet_len = strtoul(str.get().data(), NULL, 16);
    }
  
    str = pt.get_optional<std::string>("csum_error_hi");
    if (str) {
      cfg_inst->csum_error_hi = strtoul(str.get().data(), NULL, 16);
    }
  
    str = pt.get_optional<std::string>("deparser_csum_disable");
    if (str) {
      cfg_inst->deparser_csum_disable = strtoul(str.get().data(), NULL, 16);
    }
  
    str = pt.get_optional<std::string>("deparser_crc_disable");
    if (str) {
      cfg_inst->deparser_crc_disable = strtoul(str.get().data(), NULL, 16);
    }
  
    str = pt.get_optional<std::string>("roce_table_ptr");
    if (str) {
      cfg_inst->roce_table_ptr = strtoul(str.get().data(), NULL, 16);
    }
  
    str = pt.get_optional<std::string>("roce_csum_table");
    if (str) {
      cfg_inst->roce_csum_table = strtoul(str.get().data(), NULL, 16);
    }
  
    str = pt.get_optional<std::string>("roce_enable");
    if (str) {
      cfg_inst->roce_enable = strtoul(str.get().data(), NULL, 16);
    }
  
    str = pt.get_optional<std::string>("payload_start");
    if (str) {
      cfg_inst->payload_start = strtoul(str.get().data(), NULL, 16);
    }
  
    str = pt.get_optional<std::string>("payload_size");
    if (str) {
      cfg_inst->payload_size = strtoul(str.get().data(), NULL, 16);
    }
  
    str = pt.get_optional<std::string>("auth_result");
    if (str) {
      cfg_inst->auth_result = strtoul(str.get().data(), NULL, 16);
    }
  
    str = pt.get_optional<std::string>("elb_align_pad");
    if (str) {
      cfg_inst->elb_align_pad = strtoul(str.get().data(), NULL, 16);
    }
  
    str = pt.get_optional<std::string>("stage_skip");
    if (str) {
      cfg_inst->stage_skip = strtoul(str.get().data(), NULL, 16);
    }
  
    str = pt.get_optional<std::string>("phv_id");
    if (str) {
      cfg_inst->phv_id = strtoul(str.get().data(), NULL, 16);
    }
  
    str = pt.get_optional<std::string>("app_type");
    if (str) {
      cfg_inst->app_type = strtoul(str.get().data(), NULL, 16);
    }
  
    str = pt.get_optional<std::string>("elb_p4_rsv");
    if (str) {
      cfg_inst->elb_p4_rsv = strtoul(str.get().data(), NULL, 16);
    }
  
    str = pt.get_optional<std::string>("padding_1");
    if (str) {
      cfg_inst->padding_1 = strtoul(str.get().data(), NULL, 16);
    }
  
    str = pt.get_optional<std::string>("padding_0");
    if (str) {
      cfg_inst->padding_0 = strtoul(str.get().data(), NULL, 16);
    }
  
    str = pt.get_optional<std::string>("hdr_vld_0");
    if (str) {
      cfg_inst->hdr_vld_0 = strtoul(str.get().data(), NULL, 16);
    }

    str = pt.get_optional<std::string>("hdr_vld_1");
    if (str) {
      cfg_inst->hdr_vld_1 = strtoul(str.get().data(), NULL, 16);
    }
}

static inline void
sdptrace_json_phv_txdma_parse (ptree &pt, elb_phv_intr_txdma_c_t *cfg_inst, elb_phv_intr_global_c_t cfg_inst_global)
{
    boost::optional<bool> val;
    boost::optional<std::string> str;

    //Intialize the fields
      cfg_inst->crc_err = 0;
      cfg_inst->len_err = 0;
      cfg_inst->recirc_count = 0;
      cfg_inst->parser_err = 0;
      cfg_inst->crypto_hdr = 0;
      cfg_inst->frame_size = 0;
      cfg_inst->no_data = 0;
      cfg_inst->recirc = 0;
      cfg_inst->packet_len = 0;
      cfg_inst->qid = 0;
      cfg_inst->dma_cmd_ptr = 0;
      cfg_inst->qstate_addr = 0;
      cfg_inst->qtype = 0;
      cfg_inst->txdma_rsv = 0;
      cfg_inst->stage_skip = 0;
      cfg_inst->phv_id = 0;
      cfg_inst->app_type = 0;
      cfg_inst->elb_txdma_rsv = 0;
      cfg_inst->padding_3 = 0;
      cfg_inst->padding_2 = 0;
      cfg_inst->padding_1 = 0;
      cfg_inst->padding_0 = 0;
  
      cfg_inst->phv_global = cfg_inst_global;

    str = pt.get_optional<std::string>("crc_err");
    if (str) {
      cfg_inst->crc_err = strtoul(str.get().data(), NULL, 16);
    }
  
    str = pt.get_optional<std::string>("len_err");
    if (str) {
      cfg_inst->len_err = strtoul(str.get().data(), NULL, 16);
    }
  
    str = pt.get_optional<std::string>("recirc_count");
    if (str) {
      cfg_inst->recirc_count = strtoul(str.get().data(), NULL, 16);
    }
  
    str = pt.get_optional<std::string>("parser_err");
    if (str) {
      cfg_inst->parser_err = strtoul(str.get().data(), NULL, 16);
    }
  
    str = pt.get_optional<std::string>("crypto_hdr");
    if (str) {
      cfg_inst->crypto_hdr = strtoul(str.get().data(), NULL, 16);
    }
  
    str = pt.get_optional<std::string>("frame_size");
    if (str) {
      cfg_inst->frame_size = strtoul(str.get().data(), NULL, 16);
    }
  
    str = pt.get_optional<std::string>("no_data");
    if (str) {
      cfg_inst->no_data = strtoul(str.get().data(), NULL, 16);
    }
  
    str = pt.get_optional<std::string>("recirc");
    if (str) {
      cfg_inst->recirc = strtoul(str.get().data(), NULL, 16);
    }
  
    str = pt.get_optional<std::string>("packet_len");
    if (str) {
      cfg_inst->packet_len = strtoul(str.get().data(), NULL, 16);
    }
  
    str = pt.get_optional<std::string>("qid");
    if (str) {
      cfg_inst->qid = strtoul(str.get().data(), NULL, 16);
    }
  
    str = pt.get_optional<std::string>("dma_cmd_ptr");
    if (str) {
      cfg_inst->dma_cmd_ptr = strtoul(str.get().data(), NULL, 16);
    }
  
    str = pt.get_optional<std::string>("qstate_addr");
    if (str) {
      cfg_inst->qstate_addr = strtoul(str.get().data(), NULL, 16);
    }
  
    str = pt.get_optional<std::string>("qtype");
    if (str) {
      cfg_inst->qtype = strtoul(str.get().data(), NULL, 16);
    }
  
    str = pt.get_optional<std::string>("txdma_rsv");
    if (str) {
      cfg_inst->txdma_rsv = strtoul(str.get().data(), NULL, 16);
    }
  
    str = pt.get_optional<std::string>("stage_skip");
    if (str) {
      cfg_inst->stage_skip = strtoul(str.get().data(), NULL, 16);
    }
  
    str = pt.get_optional<std::string>("phv_id");
    if (str) {
      cfg_inst->phv_id = strtoul(str.get().data(), NULL, 16);
    }
  
    str = pt.get_optional<std::string>("app_type");
    if (str) {
      cfg_inst->app_type = strtoul(str.get().data(), NULL, 16);
    }
  
    str = pt.get_optional<std::string>("elb_txdma_rsv");
    if (str) {
      cfg_inst->elb_txdma_rsv = strtoul(str.get().data(), NULL, 16);
    }

    str = pt.get_optional<std::string>("padding_3");
    if (str) {
      cfg_inst->padding_3 = strtoul(str.get().data(), NULL, 16);
    }

    str = pt.get_optional<std::string>("padding_2");
    if (str) {
      cfg_inst->padding_2 = strtoul(str.get().data(), NULL, 16);
    }

    str = pt.get_optional<std::string>("padding_1");
    if (str) {
      cfg_inst->padding_1 = strtoul(str.get().data(), NULL, 16);
    }

    str = pt.get_optional<std::string>("padding_0");
    if (str) {
      cfg_inst->padding_0 = strtoul(str.get().data(), NULL, 16);
    }

}

static inline void
sdptrace_json_phv_rxdma_parse (ptree &pt, elb_phv_intr_rxdma_c_t *cfg_inst, elb_phv_intr_global_c_t cfg_inst_global)
{
    boost::optional<bool> val;
    boost::optional<std::string> str;

    //Intialize the fields
      cfg_inst->crc_err = 0;
      cfg_inst->len_err = 0;
      cfg_inst->recirc_count = 0;
      cfg_inst->parser_err = 0;
      cfg_inst->crypto_hdr = 0;
      cfg_inst->frame_size = 0;
      cfg_inst->no_data = 0;
      cfg_inst->recirc = 0;
      cfg_inst->packet_len = 0;
      cfg_inst->qid = 0;
      cfg_inst->dma_cmd_ptr = 0;
      cfg_inst->qstate_addr = 0;
      cfg_inst->qtype = 0;
      cfg_inst->rx_splitter_offset = 0;
      cfg_inst->rxdma_rsv = 0;
      cfg_inst->stage_skip = 0;
      cfg_inst->phv_id = 0;
      cfg_inst->app_type = 0;
      cfg_inst->elb_rxdma_rsv = 0;
      cfg_inst->padding_3 = 0;
      cfg_inst->padding_2 = 0;
      cfg_inst->padding_1 = 0;
      cfg_inst->padding_0 = 0;

      cfg_inst->phv_global = cfg_inst_global;

    str = pt.get_optional<std::string>("crc_err");
    if (str) {
      cfg_inst->crc_err = strtoul(str.get().data(), NULL, 16);
    }
  
    str = pt.get_optional<std::string>("len_err");
    if (str) {
      cfg_inst->len_err = strtoul(str.get().data(), NULL, 16);
    }
  
    str = pt.get_optional<std::string>("recirc_count");
    if (str) {
      cfg_inst->recirc_count = strtoul(str.get().data(), NULL, 16);
    }
  
    str = pt.get_optional<std::string>("parser_err");
    if (str) {
      cfg_inst->parser_err = strtoul(str.get().data(), NULL, 16);
    }
  
    str = pt.get_optional<std::string>("crypto_hdr");
    if (str) {
      cfg_inst->crypto_hdr = strtoul(str.get().data(), NULL, 16);
    }
  
    str = pt.get_optional<std::string>("frame_size");
    if (str) {
      cfg_inst->frame_size = strtoul(str.get().data(), NULL, 16);
    }
  
    str = pt.get_optional<std::string>("no_data");
    if (str) {
      cfg_inst->no_data = strtoul(str.get().data(), NULL, 16);
    }
  
    str = pt.get_optional<std::string>("recirc");
    if (str) {
      cfg_inst->recirc = strtoul(str.get().data(), NULL, 16);
    }
  
    str = pt.get_optional<std::string>("packet_len");
    if (str) {
      cfg_inst->packet_len = strtoul(str.get().data(), NULL, 16);
    }
  
    str = pt.get_optional<std::string>("qid");
    if (str) {
      cfg_inst->qid = strtoul(str.get().data(), NULL, 16);
    }
  
    str = pt.get_optional<std::string>("dma_cmd_ptr");
    if (str) {
      cfg_inst->dma_cmd_ptr = strtoul(str.get().data(), NULL, 16);
    }
  
    str = pt.get_optional<std::string>("qstate_addr");
    if (str) {
      cfg_inst->qstate_addr = strtoul(str.get().data(), NULL, 16);
    }
  
    str = pt.get_optional<std::string>("qtype");
    if (str) {
      cfg_inst->qtype = strtoul(str.get().data(), NULL, 16);
    }
  
    str = pt.get_optional<std::string>("rx_splitter_offset");
    if (str) {
      cfg_inst->rx_splitter_offset = strtoul(str.get().data(), NULL, 16);
    }
  
    str = pt.get_optional<std::string>("rxdma_rsv");
    if (str) {
      cfg_inst->rxdma_rsv = strtoul(str.get().data(), NULL, 16);
    }
  
    str = pt.get_optional<std::string>("stage_skip");
    if (str) {
      cfg_inst->stage_skip = strtoul(str.get().data(), NULL, 16);
    }
  
    str = pt.get_optional<std::string>("phv_id");
    if (str) {
      cfg_inst->phv_id = strtoul(str.get().data(), NULL, 16);
    }
  
    str = pt.get_optional<std::string>("app_type");
    if (str) {
      cfg_inst->app_type = strtoul(str.get().data(), NULL, 16);
    }
  
    str = pt.get_optional<std::string>("elb_rxdma_rsv");
    if (str) {
      cfg_inst->elb_rxdma_rsv = strtoul(str.get().data(), NULL, 16);
    }

    str = pt.get_optional<std::string>("padding_3");
    if (str) {
      cfg_inst->padding_3 = strtoul(str.get().data(), NULL, 16);
    }

    str = pt.get_optional<std::string>("padding_2");
    if (str) {
      cfg_inst->padding_2 = strtoul(str.get().data(), NULL, 16);
    }

    str = pt.get_optional<std::string>("padding_1");
    if (str) {
      cfg_inst->padding_1 = strtoul(str.get().data(), NULL, 16);
    }

    str = pt.get_optional<std::string>("padding_0");
    if (str) {
      cfg_inst->padding_0 = strtoul(str.get().data(), NULL, 16);
    }

}


  ///////////
  /// Below code is to check if copying from 
  /// PHV structs to trigger_data/mask array worked correctly
  //////////

void
sdptrace_json_capture_parse_verify (sdptrace_cfg_inst_t *cfg_inst)
{
    boost::optional<bool> val;
    boost::optional<std::string> str;

    elb_phv_intr_p4_c_t     cfg_phv_p4_data;
    elb_phv_intr_txdma_c_t  cfg_phv_txdma_data;
    elb_phv_intr_rxdma_c_t  cfg_phv_rxdma_data;

    elb_phv_intr_p4_c_t     cfg_phv_p4_mask;
    elb_phv_intr_txdma_c_t  cfg_phv_txdma_mask;
    elb_phv_intr_rxdma_c_t  cfg_phv_rxdma_mask;

    memset(&cfg_phv_p4_data, 0, sizeof(cfg_phv_p4_data));
    memset(&cfg_phv_txdma_data, 0, sizeof(cfg_phv_txdma_data));
    memset(&cfg_phv_rxdma_data, 0, sizeof(cfg_phv_rxdma_data));

    memset(&cfg_phv_p4_mask, 0, sizeof(cfg_phv_p4_mask));
    memset(&cfg_phv_txdma_mask, 0, sizeof(cfg_phv_txdma_mask));
    memset(&cfg_phv_rxdma_mask, 0, sizeof(cfg_phv_rxdma_mask));

    
    //    SDPTRACE_COPY_CHECK(cfg_inst, cfg_phv_p4_data, cfg_phv_p4_mask);
    //SDPTRACE_COPY_CHECK(cfg_inst, cfg_phv_txdma_data, cfg_phv_txdma_mask);
    //SDPTRACE_COPY_CHECK(cfg_inst, cfg_phv_rxdma_data, cfg_phv_rxdma_mask);

    //global is common. Print from any data/mask
      printf("global data tm_iport:0x%lx\n", cfg_phv_p4_data.phv_global.tm_iport);
      printf("global mask tm_iport:0x%lx\n", cfg_phv_p4_mask.phv_global.tm_iport);
      printf("global data tm_oport:0x%lx\n", cfg_phv_p4_data.phv_global.tm_oport);
      printf("global mask tm_oport:0x%lx\n", cfg_phv_p4_mask.phv_global.tm_oport);
      printf("global data tm_iq:0x%lx\n", cfg_phv_p4_data.phv_global.tm_iq);
      printf("global mask tm_iq:0x%lx\n", cfg_phv_p4_mask.phv_global.tm_iq);
      printf("global data lif:0x%lx\n", cfg_phv_p4_data.phv_global.lif);
      printf("global mask lif:0x%lx\n", cfg_phv_p4_mask.phv_global.lif);
      printf("global data timestamp:0x%lx\n", cfg_phv_p4_data.phv_global.timestamp);
      printf("global mask timestamp:0x%lx\n", cfg_phv_p4_mask.phv_global.timestamp);
      printf("global data tm_span_session:0x%lx\n", cfg_phv_p4_data.phv_global.tm_span_session);
      printf("global mask tm_span_session:0x%lx\n", cfg_phv_p4_mask.phv_global.tm_span_session);
      printf("global data tm_replicate_ptr:0x%lx\n", cfg_phv_p4_data.phv_global.tm_replicate_ptr);
      printf("global mask tm_replicate_ptr:0x%lx\n", cfg_phv_p4_mask.phv_global.tm_replicate_ptr);
      printf("global data tm_replicate_en:0x%lx\n", cfg_phv_p4_data.phv_global.tm_replicate_en);
      printf("global mask tm_replicate_en:0x%lx\n", cfg_phv_p4_mask.phv_global.tm_replicate_en);
      printf("global data tm_cpu:0x%lx\n", cfg_phv_p4_data.phv_global.tm_cpu);
      printf("global mask tm_cpu:0x%lx\n", cfg_phv_p4_mask.phv_global.tm_cpu);
      printf("global data tm_q_depth:0x%lx\n", cfg_phv_p4_data.phv_global.tm_q_depth);
      printf("global mask tm_q_depth:0x%lx\n", cfg_phv_p4_mask.phv_global.tm_q_depth);
      printf("global data drop:0x%lx\n", cfg_phv_p4_data.phv_global.drop);
      printf("global mask drop:0x%lx\n", cfg_phv_p4_mask.phv_global.drop);
      printf("global data bypass:0x%lx\n", cfg_phv_p4_data.phv_global.bypass);
      printf("global mask bypass:0x%lx\n", cfg_phv_p4_mask.phv_global.bypass);
      printf("global data hw_error:0x%lx\n", cfg_phv_p4_data.phv_global.hw_error);
      printf("global mask hw_error:0x%lx\n", cfg_phv_p4_mask.phv_global.hw_error);
      printf("global data tm_oq:0x%lx\n", cfg_phv_p4_data.phv_global.tm_oq);
      printf("global mask tm_oq:0x%lx\n", cfg_phv_p4_mask.phv_global.tm_oq);
      printf("global data debug_trace:0x%lx\n", cfg_phv_p4_data.phv_global.debug_trace);
      printf("global mask debug_trace:0x%lx\n", cfg_phv_p4_mask.phv_global.debug_trace);
      printf("global data csum_err:0x%lx\n", cfg_phv_p4_data.phv_global.csum_err);
      printf("global mask csum_err:0x%lx\n", cfg_phv_p4_mask.phv_global.csum_err);
      printf("global data error_bits:0x%lx\n", cfg_phv_p4_data.phv_global.error_bits);
      printf("global mask error_bits:0x%lx\n", cfg_phv_p4_mask.phv_global.error_bits);
      printf("global data tm_instance_type:0x%lx\n", cfg_phv_p4_data.phv_global.tm_instance_type);
      printf("global mask tm_instance_type:0x%lx\n", cfg_phv_p4_mask.phv_global.tm_instance_type);


    if ( (cfg_inst->pipeline_str == "p4ig") || 
	 (cfg_inst->pipeline_str == "p4eg") ) {
  
      cout << "cfg_parse_phv_verif pipeline is  " << cfg_inst->pipeline_str << endl; \
      printf("p4 data crc_err:0x%lx\n", cfg_phv_p4_data.crc_err);
      printf("p4 mask crc_err:0x%lx\n", cfg_phv_p4_mask.crc_err);
      printf("p4 data len_err:0x%lx\n", cfg_phv_p4_data.len_err);
      printf("p4 mask len_err:0x%lx\n", cfg_phv_p4_mask.len_err);
      printf("p4 data recirc_count:0x%lx\n", cfg_phv_p4_data.recirc_count);
      printf("p4 mask recirc_count:0x%lx\n", cfg_phv_p4_mask.recirc_count);
      printf("p4 data parser_err:0x%lx\n", cfg_phv_p4_data.parser_err);
      printf("p4 mask parser_err:0x%lx\n", cfg_phv_p4_mask.parser_err);
      printf("p4 data crypto_hdr:0x%lx\n", cfg_phv_p4_data.crypto_hdr);
      printf("p4 mask crypto_hdr:0x%lx\n", cfg_phv_p4_mask.crypto_hdr);
      printf("p4 data frame_size:0x%lx\n", cfg_phv_p4_data.frame_size);
      printf("p4 mask frame_size:0x%lx\n", cfg_phv_p4_mask.frame_size);
      printf("p4 data no_data:0x%lx\n", cfg_phv_p4_data.no_data);
      printf("p4 mask no_data:0x%lx\n", cfg_phv_p4_mask.no_data);
      printf("p4 data recirc:0x%lx\n", cfg_phv_p4_data.recirc);
      printf("p4 mask recirc:0x%lx\n", cfg_phv_p4_mask.recirc);
      printf("p4 data packet_len:0x%lx\n", cfg_phv_p4_data.packet_len);
      printf("p4 mask packet_len:0x%lx\n", cfg_phv_p4_mask.packet_len);
      printf("p4 data csum_error_hi:0x%lx\n", cfg_phv_p4_data.csum_error_hi);
      printf("p4 mask csum_error_hi:0x%lx\n", cfg_phv_p4_mask.csum_error_hi);
      printf("p4 data deparser_csum_disable:0x%lx\n", cfg_phv_p4_data.deparser_csum_disable);
      printf("p4 mask deparser_csum_disable:0x%lx\n", cfg_phv_p4_mask.deparser_csum_disable);
      printf("p4 data deparser_crc_disable:0x%lx\n", cfg_phv_p4_data.deparser_crc_disable);
      printf("p4 mask deparser_crc_disable:0x%lx\n", cfg_phv_p4_mask.deparser_crc_disable);
      printf("p4 data roce_table_ptr:0x%lx\n", cfg_phv_p4_data.roce_table_ptr);
      printf("p4 mask roce_table_ptr:0x%lx\n", cfg_phv_p4_mask.roce_table_ptr);
      printf("p4 data roce_csum_table:0x%lx\n", cfg_phv_p4_data.roce_csum_table);
      printf("p4 mask roce_csum_table:0x%lx\n", cfg_phv_p4_mask.roce_csum_table);
      printf("p4 data roce_enable:0x%lx\n", cfg_phv_p4_data.roce_enable);
      printf("p4 mask roce_enable:0x%lx\n", cfg_phv_p4_mask.roce_enable);
      printf("p4 data payload_start:0x%lx\n", cfg_phv_p4_data.payload_start);
      printf("p4 mask payload_start:0x%lx\n", cfg_phv_p4_mask.payload_start);
      printf("p4 data payload_size:0x%lx\n", cfg_phv_p4_data.payload_size);
      printf("p4 mask payload_size:0x%lx\n", cfg_phv_p4_mask.payload_size);
      printf("p4 data auth_result:0x%lx\n", cfg_phv_p4_data.auth_result);
      printf("p4 mask auth_result:0x%lx\n", cfg_phv_p4_mask.auth_result);
      printf("p4 data elb_align_pad:0x%lx\n", cfg_phv_p4_data.elb_align_pad);
      printf("p4 mask elb_align_pad:0x%lx\n", cfg_phv_p4_mask.elb_align_pad);
      printf("p4 data stage_skip:0x%lx\n", cfg_phv_p4_data.stage_skip);
      printf("p4 mask stage_skip:0x%lx\n", cfg_phv_p4_mask.stage_skip);
      printf("p4 data phv_id:0x%lx\n", cfg_phv_p4_data.phv_id);
      printf("p4 mask phv_id:0x%lx\n", cfg_phv_p4_mask.phv_id);
      printf("p4 data app_type:0x%lx\n", cfg_phv_p4_data.app_type);
      printf("p4 mask app_type:0x%lx\n", cfg_phv_p4_mask.app_type);
      printf("p4 data elb_p4_rsv:0x%lx\n", cfg_phv_p4_data.elb_p4_rsv);
      printf("p4 mask elb_p4_rsv:0x%lx\n", cfg_phv_p4_mask.elb_p4_rsv);
      printf("p4 data padding_0:0x%lx\n", cfg_phv_p4_data.padding_0);
      printf("p4 mask padding_0:0x%lx\n", cfg_phv_p4_mask.padding_0);
      printf("p4 data padding_1:0x%lx\n", cfg_phv_p4_data.padding_1);
      printf("p4 mask padding_1:0x%lx\n", cfg_phv_p4_mask.padding_1);
      printf("p4 data hdr_vld_0:0x%lx\n", cfg_phv_p4_data.hdr_vld_0);
      printf("p4 mask hdr_vld_0:0x%lx\n", cfg_phv_p4_mask.hdr_vld_0);
      printf("p4 data hdr_vld_1:0x%lx\n", cfg_phv_p4_data.hdr_vld_1);
      printf("p4 mask hdr_vld_1:0x%lx\n", cfg_phv_p4_mask.hdr_vld_1);

    }
    else if (cfg_inst->pipeline_str == "txdma") {
      cout << "cfg_parse_phv_verif pipeline is  " << cfg_inst->pipeline_str << endl;
      printf("txdma data crc_err:0x%lx\n", cfg_phv_txdma_data.crc_err);
      printf("txdma mask crc_err:0x%lx\n", cfg_phv_txdma_mask.crc_err);
      printf("txdma data len_err:0x%lx\n", cfg_phv_txdma_data.len_err);
      printf("txdma mask len_err:0x%lx\n", cfg_phv_txdma_mask.len_err);
      printf("txdma data recirc_count:0x%lx\n", cfg_phv_txdma_data.recirc_count);
      printf("txdma mask recirc_count:0x%lx\n", cfg_phv_txdma_mask.recirc_count);
      printf("txdma data parser_err:0x%lx\n", cfg_phv_txdma_data.parser_err);
      printf("txdma mask parser_err:0x%lx\n", cfg_phv_txdma_mask.parser_err);
      printf("txdma data crypto_hdr:0x%lx\n", cfg_phv_txdma_data.crypto_hdr);
      printf("txdma mask crypto_hdr:0x%lx\n", cfg_phv_txdma_mask.crypto_hdr);
      printf("txdma data frame_size:0x%lx\n", cfg_phv_txdma_data.frame_size);
      printf("txdma mask frame_size:0x%lx\n", cfg_phv_txdma_mask.frame_size);
      printf("txdma data no_data:0x%lx\n", cfg_phv_txdma_data.no_data);
      printf("txdma mask no_data:0x%lx\n", cfg_phv_txdma_mask.no_data);
      printf("txdma data recirc:0x%lx\n", cfg_phv_txdma_data.recirc);
      printf("txdma mask recirc:0x%lx\n", cfg_phv_txdma_mask.recirc);
      printf("txdma data packet_len:0x%lx\n", cfg_phv_txdma_data.packet_len);
      printf("txdma mask packet_len:0x%lx\n", cfg_phv_txdma_mask.packet_len);
      printf("txdma data qid:0x%lx\n", cfg_phv_txdma_data.qid);
      printf("txdma mask qid:0x%lx\n", cfg_phv_txdma_mask.qid);
      printf("txdma data dma_cmd_ptr:0x%lx\n", cfg_phv_txdma_data.dma_cmd_ptr);
      printf("txdma mask dma_cmd_ptr:0x%lx\n", cfg_phv_txdma_mask.dma_cmd_ptr);
      printf("txdma data qstate_addr:0x%lx\n", cfg_phv_txdma_data.qstate_addr);
      printf("txdma mask qstate_addr:0x%lx\n", cfg_phv_txdma_mask.qstate_addr);
      printf("txdma data qtype:0x%lx\n", cfg_phv_txdma_data.qtype);
      printf("txdma mask qtype:0x%lx\n", cfg_phv_txdma_mask.qtype);
      printf("txdma data txdma_rsv:0x%lx\n", cfg_phv_txdma_data.txdma_rsv);
      printf("txdma mask txdma_rsv:0x%lx\n", cfg_phv_txdma_mask.txdma_rsv);
      printf("txdma data stage_skip:0x%lx\n", cfg_phv_txdma_data.stage_skip);
      printf("txdma mask stage_skip:0x%lx\n", cfg_phv_txdma_mask.stage_skip);
      printf("txdma data phv_id:0x%lx\n", cfg_phv_txdma_data.phv_id);
      printf("txdma mask phv_id:0x%lx\n", cfg_phv_txdma_mask.phv_id);
      printf("txdma data app_type:0x%lx\n", cfg_phv_txdma_data.app_type);
      printf("txdma mask app_type:0x%lx\n", cfg_phv_txdma_mask.app_type);
      printf("txdma data elb_txdma_rsv:0x%lx\n", cfg_phv_txdma_data.elb_txdma_rsv);
      printf("txdma mask elb_txdma_rsv:0x%lx\n", cfg_phv_txdma_mask.elb_txdma_rsv);

      //      printf("txdma data padding_0:0x%lx0x%lx\n", (uint64_t) (cfg_phv_txdma_data.padding_0>>64), (uint64_t) (cfg_phv_txdma_data.padding_0));
      //      printf("txdma mask padding_0:0x%lx0x%lx\n", (uint64_t) (cfg_phv_txdma_mask.padding_0>>64), (uint64_t) (cfg_phv_txdma_mask.padding_0));

      printf("txdma data padding_3:0x%lx\n", cfg_phv_txdma_data.padding_3);
      printf("txdma mask padding_3:0x%lx\n", cfg_phv_txdma_mask.padding_3);

      printf("txdma data padding_2:0x%lx\n", cfg_phv_txdma_data.padding_2);
      printf("txdma mask padding_2:0x%lx\n", cfg_phv_txdma_mask.padding_2);

      printf("txdma data padding_0:0x%lx\n", cfg_phv_txdma_data.padding_0);
      printf("txdma mask padding_0:0x%lx\n", cfg_phv_txdma_mask.padding_0);

      printf("txdma data padding_1:0x%lx\n", cfg_phv_txdma_data.padding_1);
      printf("txdma mask padding_1:0x%lx\n", cfg_phv_txdma_mask.padding_1);

    }
    else if (cfg_inst->pipeline_str == "rxdma") {
      cout << "cfg_parse_phv_verif pipeline is  " << cfg_inst->pipeline_str << endl;

      printf("rxdma data crc_err:0x%lx\n", cfg_phv_rxdma_data.crc_err);
      printf("rxdma mask crc_err:0x%lx\n", cfg_phv_rxdma_mask.crc_err);
      printf("rxdma data len_err:0x%lx\n", cfg_phv_rxdma_data.len_err);
      printf("rxdma mask len_err:0x%lx\n", cfg_phv_rxdma_mask.len_err);
      printf("rxdma data recirc_count:0x%lx\n", cfg_phv_rxdma_data.recirc_count);
      printf("rxdma mask recirc_count:0x%lx\n", cfg_phv_rxdma_mask.recirc_count);
      printf("rxdma data parser_err:0x%lx\n", cfg_phv_rxdma_data.parser_err);
      printf("rxdma mask parser_err:0x%lx\n", cfg_phv_rxdma_mask.parser_err);
      printf("rxdma data crypto_hdr:0x%lx\n", cfg_phv_rxdma_data.crypto_hdr);
      printf("rxdma mask crypto_hdr:0x%lx\n", cfg_phv_rxdma_mask.crypto_hdr);
      printf("rxdma data frame_size:0x%lx\n", cfg_phv_rxdma_data.frame_size);
      printf("rxdma mask frame_size:0x%lx\n", cfg_phv_rxdma_mask.frame_size);
      printf("rxdma data no_data:0x%lx\n", cfg_phv_rxdma_data.no_data);
      printf("rxdma mask no_data:0x%lx\n", cfg_phv_rxdma_mask.no_data);
      printf("rxdma data recirc:0x%lx\n", cfg_phv_rxdma_data.recirc);
      printf("rxdma mask recirc:0x%lx\n", cfg_phv_rxdma_mask.recirc);
      printf("rxdma data packet_len:0x%lx\n", cfg_phv_rxdma_data.packet_len);
      printf("rxdma mask packet_len:0x%lx\n", cfg_phv_rxdma_mask.packet_len);
      printf("rxdma data qid:0x%lx\n", cfg_phv_rxdma_data.qid);
      printf("rxdma mask qid:0x%lx\n", cfg_phv_rxdma_mask.qid);
      printf("rxdma data dma_cmd_ptr:0x%lx\n", cfg_phv_rxdma_data.dma_cmd_ptr);
      printf("rxdma mask dma_cmd_ptr:0x%lx\n", cfg_phv_rxdma_mask.dma_cmd_ptr);
      printf("rxdma data qstate_addr:0x%lx\n", cfg_phv_rxdma_data.qstate_addr);
      printf("rxdma mask qstate_addr:0x%lx\n", cfg_phv_rxdma_mask.qstate_addr);
      printf("rxdma data qtype:0x%lx\n", cfg_phv_rxdma_data.qtype);
      printf("rxdma mask qtype:0x%lx\n", cfg_phv_rxdma_mask.qtype);
      printf("rxdma data rx_splitter_offset:0x%lx\n", cfg_phv_rxdma_data.rx_splitter_offset);
      printf("rxdma mask rx_splitter_offset:0x%lx\n", cfg_phv_rxdma_mask.rx_splitter_offset);
      printf("rxdma data rxdma_rsv:0x%lx\n", cfg_phv_rxdma_data.rxdma_rsv);
      printf("rxdma mask rxdma_rsv:0x%lx\n", cfg_phv_rxdma_mask.rxdma_rsv);
      printf("rxdma data stage_skip:0x%lx\n", cfg_phv_rxdma_data.stage_skip);
      printf("rxdma mask stage_skip:0x%lx\n", cfg_phv_rxdma_mask.stage_skip);
      printf("rxdma data phv_id:0x%lx\n", cfg_phv_rxdma_data.phv_id);
      printf("rxdma mask phv_id:0x%lx\n", cfg_phv_rxdma_mask.phv_id);
      printf("rxdma data app_type:0x%lx\n", cfg_phv_rxdma_data.app_type);
      printf("rxdma mask app_type:0x%lx\n", cfg_phv_rxdma_mask.app_type);
      printf("rxdma data elb_rxdma_rsv:0x%lx\n", cfg_phv_rxdma_data.elb_rxdma_rsv);
      printf("rxdma mask elb_rxdma_rsv:0x%lx\n", cfg_phv_rxdma_mask.elb_rxdma_rsv);
      printf("txdma data padding_3:0x%lx\n", cfg_phv_txdma_data.padding_3);
      printf("txdma mask padding_3:0x%lx\n", cfg_phv_txdma_mask.padding_3);
      printf("txdma data padding_2:0x%lx\n", cfg_phv_txdma_data.padding_2);
      printf("txdma mask padding_2:0x%lx\n", cfg_phv_txdma_mask.padding_2);
      printf("rxdma data padding_0:0x%lx\n", cfg_phv_rxdma_data.padding_0);
      printf("rxdma mask padding_0:0x%lx\n", cfg_phv_rxdma_mask.padding_0);
      printf("rxdma data padding_1:0x%lx\n", cfg_phv_rxdma_data.padding_1);
      printf("rxdma mask padding_1:0x%lx\n", cfg_phv_rxdma_mask.padding_1);
    } 
    else {
      //      cout << cfg_inst->pipeline_str << "is not a valid pipeline" << endl;
    }
    

}



static inline cpp_int
sdptrace_pack_global (cpp_int glb_data, elb_phv_intr_global_c_t *cfg_inst)
{
    boost::optional<bool> val;
    boost::optional<std::string> str;

    //    cpp_int glb_data;
    glb_data = 0;
    cpp_int_helper hlp;


    glb_data = hlp.set_slc(glb_data, cfg_inst->tm_iport, 132, 132+4-1);
    glb_data = hlp.set_slc(glb_data, cfg_inst->tm_oport, 128, 128+4-1);
    glb_data = hlp.set_slc(glb_data, cfg_inst->tm_iq, 123, 123+5-1);
    glb_data = hlp.set_slc(glb_data, cfg_inst->lif, 112, 112+11-1);
    cout << "timestamp " << hex << cfg_inst->timestamp << endl;
    //    glb_data = hlp.set_slc(glb_data, (cfg_inst->timestamp >> 32), 64+32, 64+48-1);
    //glb_data = hlp.set_slc(glb_data, cfg_inst->timestamp, 64, 64+32-1);
    cpp_int set_val_cpp_int(cfg_inst->timestamp);
    glb_data = hlp.set_slc(glb_data, set_val_cpp_int, 64, 64+48-1);
    cout << "glb_data " << hex << glb_data << endl;
    glb_data = hlp.set_slc(glb_data, cfg_inst->tm_span_session, 56, 56+8-1);
    glb_data = hlp.set_slc(glb_data, cfg_inst->tm_replicate_ptr, 40, 40+16-1);
    glb_data = hlp.set_slc(glb_data, cfg_inst->tm_replicate_en, 39, 39+1-1);
    glb_data = hlp.set_slc(glb_data, cfg_inst->tm_cpu, 38, 38+1-1);
    glb_data = hlp.set_slc(glb_data, cfg_inst->tm_q_depth, 24, 24+14-1);
    glb_data = hlp.set_slc(glb_data, cfg_inst->drop, 23, 23+1-1);
    glb_data = hlp.set_slc(glb_data, cfg_inst->bypass, 22, 22+1-1);
    glb_data = hlp.set_slc(glb_data, cfg_inst->hw_error, 21, 21+1-1);
    glb_data = hlp.set_slc(glb_data, cfg_inst->tm_oq, 16, 16+5-1);
    glb_data = hlp.set_slc(glb_data, cfg_inst->debug_trace, 15, 15+1-1);
    glb_data = hlp.set_slc(glb_data, cfg_inst->csum_err, 10, 10+5-1);
    glb_data = hlp.set_slc(glb_data, cfg_inst->error_bits, 4, 4+6-1);
    glb_data = hlp.set_slc(glb_data, cfg_inst->tm_instance_type, 0, 0+4-1);

    cout << "justina glb_pack " << hex << glb_data << endl;

    return (glb_data);
}

static inline cpp_int
sdptrace_pack_p4 (cpp_int p4_data, cpp_int glb_data, elb_phv_intr_p4_c_t *cfg_inst)
{
    boost::optional<bool> val;
    boost::optional<std::string> str;

    //    cpp_int glb_data;
    p4_data = 0;
    cpp_int_helper hlp;
    //copy global to P4
    p4_data = glb_data << 376;

    p4_data = hlp.set_slc(p4_data, cfg_inst->crc_err, 375, 375+1-1);
    p4_data = hlp.set_slc(p4_data, cfg_inst->len_err, 371, 371+4-1);
    p4_data = hlp.set_slc(p4_data, cfg_inst->recirc_count, 368, 368+3-1);
    p4_data = hlp.set_slc(p4_data, cfg_inst->parser_err, 367, 367+1-1);
    p4_data = hlp.set_slc(p4_data, cfg_inst->crypto_hdr, 366, 366+1-1);
    p4_data = hlp.set_slc(p4_data, cfg_inst->frame_size, 352, 352+14-1);
    p4_data = hlp.set_slc(p4_data, cfg_inst->no_data, 351, 351+1-1);
    p4_data = hlp.set_slc(p4_data, cfg_inst->recirc, 350, 350+1-1);
    p4_data = hlp.set_slc(p4_data, cfg_inst->packet_len, 336, 336+14-1);
    p4_data = hlp.set_slc(p4_data, cfg_inst->csum_error_hi, 334, 334+2-1);
    p4_data = hlp.set_slc(p4_data, cfg_inst->deparser_csum_disable, 327, 327+7-1);
    p4_data = hlp.set_slc(p4_data, cfg_inst->deparser_crc_disable, 326, 326+1-1);
    p4_data = hlp.set_slc(p4_data, cfg_inst->roce_table_ptr, 317, 317+9-1);
    p4_data = hlp.set_slc(p4_data, cfg_inst->roce_csum_table, 311, 311+6-1);
    p4_data = hlp.set_slc(p4_data, cfg_inst->roce_enable, 310, 310+1-1);
    p4_data = hlp.set_slc(p4_data, cfg_inst->payload_start, 296, 296+14-1);
    p4_data = hlp.set_slc(p4_data, cfg_inst->payload_size, 282, 282+14-1);
    p4_data = hlp.set_slc(p4_data, cfg_inst->auth_result, 281, 281+1-1);
    p4_data = hlp.set_slc(p4_data, cfg_inst->elb_align_pad, 256, 256+25-1);
    p4_data = hlp.set_slc(p4_data, cfg_inst->stage_skip, 248, 248+8-1);
    p4_data = hlp.set_slc(p4_data, cfg_inst->phv_id, 238, 238+10-1);
    p4_data = hlp.set_slc(p4_data, cfg_inst->app_type, 234, 234+4-1);
    p4_data = hlp.set_slc(p4_data, cfg_inst->elb_p4_rsv, 232, 232+2-1);
    cpp_int set_val_cpp_int0(cfg_inst->padding_1);
    p4_data = hlp.set_slc(p4_data, set_val_cpp_int0, 192, 192+40-1);
    cpp_int set_val_cpp_int1(cfg_inst->padding_0);
    p4_data = hlp.set_slc(p4_data, set_val_cpp_int1, 128, 128+64-1);
    cpp_int set_val_cpp_int2(cfg_inst->hdr_vld_1);
    p4_data = hlp.set_slc(p4_data, set_val_cpp_int2, 64, 64+64-1);
    cpp_int set_val_cpp_int3(cfg_inst->hdr_vld_0);
    p4_data = hlp.set_slc(p4_data, set_val_cpp_int3, 0, 0+64-1);

    return (p4_data);

}

static inline cpp_int
sdptrace_pack_tx (cpp_int tx_data, cpp_int glb_data, elb_phv_intr_txdma_c_t *cfg_inst)
{
    boost::optional<bool> val;
    boost::optional<std::string> str;

    //    cpp_int glb_data;
    tx_data = 0;
    cpp_int_helper hlp;
    //copy global to tx
    tx_data = glb_data << 376;

    tx_data = hlp.set_slc(tx_data, cfg_inst->crc_err, 375, 375+1-1);
    tx_data = hlp.set_slc(tx_data, cfg_inst->len_err, 371, 371+4-1);
    tx_data = hlp.set_slc(tx_data, cfg_inst->recirc_count, 368, 368+3-1);
    tx_data = hlp.set_slc(tx_data, cfg_inst->parser_err, 367, 367+1-1);
    tx_data = hlp.set_slc(tx_data, cfg_inst->crypto_hdr, 366, 366+1-1);
    tx_data = hlp.set_slc(tx_data, cfg_inst->frame_size, 352, 352+14-1);
    tx_data = hlp.set_slc(tx_data, cfg_inst->no_data, 351, 351+1-1);
    tx_data = hlp.set_slc(tx_data, cfg_inst->recirc, 350, 350+1-1);
    tx_data = hlp.set_slc(tx_data, cfg_inst->packet_len, 336, 336+14-1);
    tx_data = hlp.set_slc(tx_data, cfg_inst->qid, 312, 312+24-1);
    tx_data = hlp.set_slc(tx_data, cfg_inst->dma_cmd_ptr, 306, 306+6-1);
    tx_data = hlp.set_slc(tx_data, cfg_inst->qstate_addr, 272, 272+34-1);
    tx_data = hlp.set_slc(tx_data, cfg_inst->qtype, 269, 269+3-1);
    tx_data = hlp.set_slc(tx_data, cfg_inst->txdma_rsv, 264, 264+5-1);
    tx_data = hlp.set_slc(tx_data, cfg_inst->elb_align_pad, 256, 256+8-1);
    tx_data = hlp.set_slc(tx_data, cfg_inst->stage_skip, 248, 248+8-1);
    tx_data = hlp.set_slc(tx_data, cfg_inst->phv_id, 238, 238+10-1);
    tx_data = hlp.set_slc(tx_data, cfg_inst->app_type, 234, 234+4-1);
    tx_data = hlp.set_slc(tx_data, cfg_inst->elb_txdma_rsv, 232, 232+2-1);

    cpp_int set_val_cpp_int0(cfg_inst->padding_3);
    cpp_int set_val_cpp_int1(cfg_inst->padding_2);
    cpp_int set_val_cpp_int2(cfg_inst->padding_1);
    cpp_int set_val_cpp_int3(cfg_inst->padding_0);

    tx_data = hlp.set_slc(tx_data, set_val_cpp_int0, 192, 192+40-1);
    tx_data = hlp.set_slc(tx_data, set_val_cpp_int1, 128, 128+64-1);
    tx_data = hlp.set_slc(tx_data, set_val_cpp_int2, 64, 64+64-1);
    tx_data = hlp.set_slc(tx_data, set_val_cpp_int3, 0, 0+64-1);

    cout << "justina txdma_pack " << hex << tx_data << endl;

    return (tx_data);
}

static inline cpp_int
sdptrace_pack_rx (cpp_int rx_data, cpp_int glb_data, elb_phv_intr_rxdma_c_t *cfg_inst)
{
    boost::optional<bool> val;
    boost::optional<std::string> str;

    rx_data = 0;
    cpp_int_helper hlp;
    //copy global to rx
    rx_data = glb_data << 376;

    rx_data = hlp.set_slc(rx_data, cfg_inst->crc_err, 375, 375+1-1);
    rx_data = hlp.set_slc(rx_data, cfg_inst->len_err, 371, 371+4-1);
    rx_data = hlp.set_slc(rx_data, cfg_inst->recirc_count, 368, 368+3-1);
    rx_data = hlp.set_slc(rx_data, cfg_inst->parser_err, 367, 367+1-1);
    rx_data = hlp.set_slc(rx_data, cfg_inst->crypto_hdr, 366, 366+1-1);
    rx_data = hlp.set_slc(rx_data, cfg_inst->frame_size, 352, 352+14-1);
    rx_data = hlp.set_slc(rx_data, cfg_inst->no_data, 351, 351+1-1);
    rx_data = hlp.set_slc(rx_data, cfg_inst->recirc, 350, 350+1-1);
    rx_data = hlp.set_slc(rx_data, cfg_inst->packet_len, 336, 336+14-1);
    rx_data = hlp.set_slc(rx_data, cfg_inst->qid, 312, 312+24-1);
    rx_data = hlp.set_slc(rx_data, cfg_inst->dma_cmd_ptr, 306, 306+6-1);
    rx_data = hlp.set_slc(rx_data, cfg_inst->qstate_addr, 272, 272+34-1);
    rx_data = hlp.set_slc(rx_data, cfg_inst->qtype, 269, 269+3-1);
    rx_data = hlp.set_slc(rx_data, cfg_inst->rx_splitter_offset, 259, 259+10-1);
    rx_data = hlp.set_slc(rx_data, cfg_inst->rxdma_rsv, 256, 256+3-1);
    rx_data = hlp.set_slc(rx_data, cfg_inst->stage_skip, 248, 248+8-1);
    rx_data = hlp.set_slc(rx_data, cfg_inst->phv_id, 238, 238+10-1);
    rx_data = hlp.set_slc(rx_data, cfg_inst->app_type, 234, 234+4-1);
    rx_data = hlp.set_slc(rx_data, cfg_inst->elb_rxdma_rsv, 232, 232+2-1);

    cpp_int set_val_cpp_int0(cfg_inst->padding_3);
    cpp_int set_val_cpp_int1(cfg_inst->padding_2);
    cpp_int set_val_cpp_int2(cfg_inst->padding_1);
    cpp_int set_val_cpp_int3(cfg_inst->padding_0);

    rx_data = hlp.set_slc(rx_data, set_val_cpp_int0, 192, 192+40-1);
    rx_data = hlp.set_slc(rx_data, set_val_cpp_int1, 128, 128+64-1);
    rx_data = hlp.set_slc(rx_data, set_val_cpp_int2, 64, 64+64-1);
    rx_data = hlp.set_slc(rx_data, set_val_cpp_int3, 0, 0+64-1);

    return (rx_data);
}

void
sdptrace_json_capture_parse (ptree &pt, sdptrace_cfg_inst_t *cfg_inst)
{
    boost::optional<bool> val;
    boost::optional<std::string> str;

    elb_phv_intr_global_c_t cfg_phv_global_data;
    elb_phv_intr_p4_c_t     cfg_phv_p4_data;
    elb_phv_intr_txdma_c_t  cfg_phv_txdma_data;
    elb_phv_intr_rxdma_c_t  cfg_phv_rxdma_data;

    elb_phv_intr_global_c_t cfg_phv_global_mask;
    elb_phv_intr_p4_c_t     cfg_phv_p4_mask;
    elb_phv_intr_txdma_c_t  cfg_phv_txdma_mask;
    elb_phv_intr_rxdma_c_t  cfg_phv_rxdma_mask;

    memset(&cfg_phv_global_data, 0, sizeof(cfg_phv_global_data));
    memset(&cfg_phv_p4_data, 0, sizeof(cfg_phv_p4_data));
    memset(&cfg_phv_txdma_data, 0, sizeof(cfg_phv_txdma_data));
    memset(&cfg_phv_rxdma_data, 0, sizeof(cfg_phv_rxdma_data));

    memset(&cfg_phv_global_mask, 0, sizeof(cfg_phv_global_mask));
    memset(&cfg_phv_p4_mask, 0, sizeof(cfg_phv_p4_mask));
    memset(&cfg_phv_txdma_mask, 0, sizeof(cfg_phv_txdma_mask));
    memset(&cfg_phv_rxdma_mask, 0, sizeof(cfg_phv_rxdma_mask));


    for (ptree::iterator pos = pt.begin(); pos != pt.end();) {
        if (strcmp(pos->first.data(), "trigger_data") == 0) {
	  sdptrace_json_phv_global_parse(pos->second, &cfg_phv_global_data);
	  sdptrace_json_phv_p4_parse(pos->second, &cfg_phv_p4_data, cfg_phv_global_data);
	  sdptrace_json_phv_txdma_parse(pos->second, &cfg_phv_txdma_data, cfg_phv_global_data);
	  sdptrace_json_phv_rxdma_parse(pos->second, &cfg_phv_rxdma_data, cfg_phv_global_data);
        }
        else if (strcmp(pos->first.data(), "trigger_mask") == 0) {
	  sdptrace_json_phv_global_parse(pos->second, &cfg_phv_global_mask);
	  sdptrace_json_phv_p4_parse(pos->second, &cfg_phv_p4_mask, cfg_phv_global_mask);
	  sdptrace_json_phv_txdma_parse(pos->second, &cfg_phv_txdma_mask, cfg_phv_global_mask);
	  sdptrace_json_phv_rxdma_parse(pos->second, &cfg_phv_rxdma_mask, cfg_phv_global_mask);
        }
        pos++;
    }

    cpp_int glb_data = 0;
    cpp_int hdr_data = 0;
    cpp_int glb_mask = 0;
    cpp_int hdr_mask = 0;
    //assign phv back to sdptrace_cfg_inst.trigger_data and mask
    if ( !( (cfg_inst->pipeline_str == "p4ig") || 
	    (cfg_inst->pipeline_str == "p4eg") ||
	    (cfg_inst->pipeline_str == "txdma")||
	    (cfg_inst->pipeline_str == "rxdma")) ) {

      //todo: fix this. This shdn't be hit
      cout << cfg_inst->pipeline_str << "is not a valid pipeline jk" << endl;

    }
    else {

      if ( (cfg_inst->pipeline_str == "p4ig") || 
	   (cfg_inst->pipeline_str == "p4eg") ) {
	
	cout << "cfg_parse_phv pipeline is  " << cfg_inst->pipeline_str << endl; \
	glb_data = sdptrace_pack_global(glb_data, &cfg_phv_p4_data.phv_global);
	hdr_data = sdptrace_pack_p4(hdr_data, glb_data, &cfg_phv_p4_data);
	glb_mask = sdptrace_pack_global(glb_mask, &cfg_phv_p4_mask.phv_global);
	hdr_mask = sdptrace_pack_p4(hdr_mask, glb_mask, &cfg_phv_p4_mask);
      }
      else if (cfg_inst->pipeline_str == "txdma") {
	cout << "cfg_parse_phv pipeline is  " << cfg_inst->pipeline_str << endl;
	glb_data = sdptrace_pack_global(glb_data, &cfg_phv_txdma_data.phv_global);
	hdr_data = sdptrace_pack_tx(hdr_data, glb_data, &cfg_phv_txdma_data);
	glb_mask = sdptrace_pack_global(glb_mask, &cfg_phv_txdma_mask.phv_global);
	hdr_mask = sdptrace_pack_tx(hdr_mask, glb_mask, &cfg_phv_txdma_mask);
      }
      else if (cfg_inst->pipeline_str == "rxdma") {
	cout << "cfg_parse_phv pipeline is  " << cfg_inst->pipeline_str << endl;
	glb_data = sdptrace_pack_global(glb_data, &cfg_phv_rxdma_data.phv_global);
	hdr_data = sdptrace_pack_rx(hdr_data, glb_data, &cfg_phv_rxdma_data);
	glb_mask = sdptrace_pack_global(glb_mask, &cfg_phv_rxdma_mask.phv_global);
	hdr_mask = sdptrace_pack_rx(hdr_mask, glb_mask, &cfg_phv_rxdma_mask);
      } 
      //assign hdr_data and hdr_mask to cfg_inst.trigger_data/mask
      cfg_inst->capture.trigger_data = hdr_data;
      cfg_inst->capture.trigger_mask = hdr_mask;
      
      cout << "parse phv trigger data " << hex << cfg_inst->capture.trigger_data << endl;
      cout << "parse phv trigger mask " << hex << cfg_inst->capture.trigger_mask << endl;
    }
    //check if copying worked correctly
    //    sdptrace_json_capture_parse_verify(cfg_inst);

}
  

}    // end namespace platform
}    // end namespace sdk
