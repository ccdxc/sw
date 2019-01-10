
#include <cmath>
#include <grpc++/grpc++.h>
#include <inttypes.h>
#include <iostream>
#include <map>
#include <string>
#include <fstream>

#include "gen/proto/debug.grpc.pb.h"

#include "nic/include/mpu_trace.hpp"
#include "nic/sdk/include/sdk/pal.hpp"
#include "gen/platform/mem_regions.hpp"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using debug::Debug;


#define TRACE_BASE  roundup(MEM_REGION_ADDR(MPU_TRACE), 4096)
#define TRACE_END  (MEM_REGION_ADDR(MPU_TRACE) + \
     MEM_REGION_MPU_TRACE_SIZE )
#define TRACE_ENTRY_SIZE    (64)

std::string svc_endpoint = "localhost:50054";

std::string mpu_trace_proto_file_name = "mpu_trace.proto";
std::string mpu_trace_dump_file_name = "mpu_trace.bin";

std::map<std::string, debug::MpuTracePipelineType> pipeline_name_to_id = {
    {"txdma", debug::MPU_TRACE_PIPELINE_P4P_TXDMA},
    {"rxdma", debug::MPU_TRACE_PIPELINE_P4P_RXDMA},
    {"p4ig", debug::MPU_TRACE_PIPELINE_P4_INGRESS},
    {"p4eg", debug::MPU_TRACE_PIPELINE_P4_EGRESS},
};

std::map<debug::MpuTracePipelineType, int> max_stages = {
    {debug::MPU_TRACE_PIPELINE_P4_INGRESS, 5},
    {debug::MPU_TRACE_PIPELINE_P4_EGRESS, 5},
    {debug::MPU_TRACE_PIPELINE_P4P_RXDMA, 7},
    {debug::MPU_TRACE_PIPELINE_P4P_TXDMA, 7}
};

std::map<std::string, int> trace_options = {
    {"trace_enable", 1},
    {"phv_debug", 2},
    {"phv_error", 3},
    {"table_key_enable", 4},
    {"instr_enable", 5},
    {"wrap", 6},
    {"watch_pc", 7},
    {"trace_size", 8},
};

int mpu_trace(debug::MpuTraceRequestMsg &req_msg)
{
    ClientContext context;
    Status status;
    debug::MpuTraceResponseMsg rsp_msg;
    std::shared_ptr<Channel> channel =
        grpc::CreateChannel(svc_endpoint, grpc::InsecureChannelCredentials());

    auto debug_stub_ = Debug::NewStub(channel);
    status = debug_stub_->MpuTraceUpdate(&context, req_msg, &rsp_msg);
    if (status.ok()) {
        for (int i = 0; i < rsp_msg.response().size(); i++) {
            auto rsp = rsp_msg.response(i);
            if (rsp.api_status() != types::API_STATUS_OK) {
                std::cerr << __FUNCTION__ << " API status " << rsp.api_status() << std::endl;
            }
        }
        std::cout<< __FUNCTION__ << " success!" << std::endl;
        return 0;
    } else {
        std::cerr << __FUNCTION__ << ": status = " << status.error_code() << ":" << status.error_message() << std::endl;
    }

    return -1;
}

void parse_options(std::shared_ptr<mpu_trace_record_t> record)
{
    char *option, *value;

    assert(record != NULL);

    record->trace_enable = false;
    record->phv_debug = false;
    record->phv_error = false;
    record->table_key = false;
    record->instructions = false;
    record->wrap = false;
    record->watch_pc = 0;
    record->trace_addr = 0;
    record->trace_size = 4096;

    // Parse and validate options
    while (option != NULL) {

        option = strtok(NULL, "=");
        if (option == NULL) {
            break;
        }
        // printf("option: %s\n", option);

        value = strtok(NULL, ",");
        if (value == NULL) {
            break;
        }
        // printf("value: %s\n", value);

        switch (trace_options[std::string((const char *)option)]) {
        case 0:
            printf("Invalid option '%s' ... ignoring!\n", option);
            break;
        case 1:
            record->trace_enable = atoi(value);
            break;
        case 2:
            record->phv_debug = atoi(value);
            break;
        case 3:
            record->phv_error = atoi(value);
            break;
        case 4:
            record->table_key = atoi(value);
            break;
        case 5:
            record->instructions = atoi(value);
            break;
        case 6:
            record->wrap = atoi(value);
            break;
        case 7:
            record->watch_pc = strtoul(value, NULL, 16);
            break;
        case 8:
            record->trace_size = atoi(value);
            assert(record->trace_size > 0);
            break;
        default:
            printf("Invalid option '%s' ... ignoring!\n", option);
            break;
        }
    }

    return;
}

int mpu_trace_config(const char *cfg_file, std::string cfg_proto)
{
    FILE *fp = NULL;
    int ret = 0;
    char * line = NULL;
    size_t len = 0;
    char *token;
    char *pipeline_name;
    int stage;
    int mpu;
    bool enable;
    uint64_t trace_addr = TRACE_BASE;
    int line_num = -1, col_num = 0;
    std::shared_ptr<mpu_trace_record_t> record;

    debug::MpuTracePipelineType pipeline;
    debug::MpuTraceRequestMsg req_msg;

    fp = fopen(cfg_file, "r");
    if (fp == NULL) {
        std::cerr << "Failed to load config file" << std::endl;
        return -1;
    }

    std::map<std::tuple<int, int, int>, std::shared_ptr<mpu_trace_record_t>> config;

    while ((getline(&line, &len, fp)) != -1) {

        ++line_num;
        col_num = 0;
        if (line[0] == '#') {// comment
            continue;
        }

        // Parse and validate pipeline
        token = strtok(line, ",");
        if (token == NULL) {
            std::cerr << "Truncated line " << line_num << " col " << col_num << std::endl;
            return -1;
        } else if (strncmp(token, "*", 1) == 0) {
            pipeline_name = token;
            pipeline = debug::MPU_TRACE_PIPELINE_NONE;  // all pipelines
        } else {
            pipeline_name = token;
            if (pipeline_name_to_id.find(std::string((const char *)pipeline_name)) == pipeline_name_to_id.end()) {
                std::cout << "Invalid pipeline " << pipeline_name <<
                             " on line " << line_num << " col " << col_num
                             << std::endl;
                return -1;
            }
            pipeline = pipeline_name_to_id[std::string((const char *)token)];
        }
        col_num++;

        // Parse and validate stage
        token = strtok(NULL, ",");
        if (token == NULL) {
            std::cerr << "Truncated line " << line_num << " col " << col_num << std::endl;
            return -1;
        } else if (strncmp(token, "*",  1) == 0) {
            stage = -1; // all stages
        } else {
            stage = atoi(token);
            if (stage > max_stages[pipeline]) {
                std::cout << "Invalid stage " << stage <<
                             " for pipeline " << pipeline_name <<
                             " on line " << line_num << " col " << col_num
                             << std::endl;
                return -1;
            }
        }
        col_num++;

        // Parse and validate mpu
        token = strtok(NULL, ",");
        if (token == NULL) {
            std::cerr << "Truncated line " << line_num << " col " << col_num << std::endl;
            return -1;
        } else if (strncmp(token, "*",  1) == 0) {
            mpu = -1; // all mpus
        } else {
            mpu = atoi(token);
            if (mpu > MAX_MPU) {
                std::cout << "Invalid mpu " << mpu <<
                             " for stage " << stage <<
                             " pipeline " << pipeline_name <<
                             " on line " << line_num << " col " << col_num
                             << std::endl;
                return -1;
            }
        }
        col_num++;

        // Parse and validate options
        record = std::make_shared<mpu_trace_record_t>();
        parse_options(record);

        for (int p = 1; p < MAX_NUM_PIPELINE; p++) {
            for (int s = 0; s <= max_stages[(debug::MpuTracePipelineType)p]; s++) {
                for (int m = 0; m < MAX_MPU; m++) {
                    enable = (pipeline == (int)debug::MPU_TRACE_PIPELINE_NONE || p == pipeline) &&
                                (stage == -1 || s == stage) && (mpu == -1 || m == mpu);
                    std::tuple<int, int, int>key (p, s, m);
                    if (enable) {
                        config[key] = record;
                    } else if (config.find(key) == config.cend()) {
                        config[key] = NULL;
                    }
                }
            }
        }

        line_num++;
    }

    for (auto it = config.cbegin(); it != config.cend(); it++) {
        auto req = req_msg.add_request();
        std::shared_ptr<mpu_trace_record_t> record = it->second;

        req->set_pipeline_type((debug::MpuTracePipelineType)std::get<0>(it->first));
        req->set_stage_id(std::get<1>(it->first));
        req->set_mpu(std::get<2>(it->first));

        if (record) {
            assert(trace_addr + (record->trace_size * TRACE_ENTRY_SIZE) < TRACE_END);
            assert((trace_addr & 0x3f) == 0x0);

            // Erase trace buffer
            sdk::lib::pal_mem_set(trace_addr, 0, record->trace_size * TRACE_ENTRY_SIZE);

            req->mutable_spec()->set_enable(true);
            req->mutable_spec()->set_trace_enable(record->trace_enable);
            req->mutable_spec()->set_phv_debug(record->phv_debug);
            req->mutable_spec()->set_phv_error(record->phv_error);
            req->mutable_spec()->set_watch_pc(record->watch_pc);
            req->mutable_spec()->set_table_key(record->table_key | record->instructions);
            req->mutable_spec()->set_instructions(record->instructions);
            req->mutable_spec()->set_wrap(record->wrap);
            req->mutable_spec()->set_base_addr(trace_addr);
            req->mutable_spec()->set_buf_size(record->trace_size);
            req->mutable_spec()->set_reset(false);
            trace_addr += (record->trace_size * TRACE_ENTRY_SIZE);
        }
    }

    mpu_trace(req_msg);

    std::fstream output(cfg_proto, std::ios::out | std::ios::trunc | std::ios::binary);
    if (!req_msg.SerializeToOstream(&output)) {
        std::cerr << "Failed to write mpu trace config proto!" << std::endl;
        ret = -1;
    }

    fclose(fp);
    if (line) {
        free(line);
    }

    return ret;
}

int mpu_trace_enable(std::string cfg_proto)
{
    debug::MpuTraceRequestMsg req_msg;

    std::fstream input(cfg_proto, std::ios::in | std::ios::binary);
    if (!req_msg.ParseFromIstream(&input)) {
        std::cerr << "Failed to read mpu trace config proto!" << std::endl;
        return -1;
    }

    for (int i = 0; i < req_msg.request().size(); i++) {
        auto req = req_msg.mutable_request(i);
        if (req->has_spec()) {
            req->mutable_spec()->set_enable(true);
        }
    }

    mpu_trace(req_msg);

    std::fstream output(cfg_proto, std::ios::out | std::ios::trunc | std::ios::binary);
    if (!req_msg.SerializeToOstream(&output)) {
        std::cerr << "Failed to write mpu trace config proto!" << std::endl;
        return -1;
    }

    return 0;
}

int mpu_trace_disable(std::string cfg_proto)
{
    debug::MpuTraceRequestMsg req_msg;

    std::fstream input(cfg_proto, std::ios::in | std::ios::binary);
    if (!req_msg.ParseFromIstream(&input)) {
        std::cerr << "Failed to read mpu trace config proto!" << std::endl;
        return -1;
    }

    for (int i = 0; i < req_msg.request().size(); i++) {
        auto req = req_msg.mutable_request(i);
        if (req->has_spec()) {
            req->mutable_spec()->set_enable(false);
        }
    }

    mpu_trace(req_msg);

    std::fstream output(cfg_proto, std::ios::out | std::ios::trunc | std::ios::binary);
    if (!req_msg.SerializeToOstream(&output)) {
        std::cerr << "Failed to write mpu trace config proto!" << std::endl;
        return -1;
    }

    return 0;
}

int mpu_trace_reset(std::string cfg_proto)
{
    debug::MpuTraceRequestMsg req_msg;

    std::fstream input(cfg_proto, std::ios::in | std::ios::binary);
    if (!req_msg.ParseFromIstream(&input)) {
        std::cerr << "Failed to read mpu trace config proto!" << std::endl;
        return -1;
    }

    for (int i = 0; i < req_msg.request().size(); i++) {
        auto req = req_msg.mutable_request(i);
        if (req->has_spec()) {
            req->mutable_spec()->set_enable(false);
            req->mutable_spec()->set_reset(true);
        }
    }

    mpu_trace(req_msg);

    // Erase trace buffer
    for (int i = 0; i < req_msg.request().size(); i++) {
        auto req = req_msg.request(i);
        if (req.has_spec()) {
            auto trace_addr = req.spec().base_addr();
            auto trace_size = req.spec().buf_size() * TRACE_ENTRY_SIZE;
            sdk::lib::pal_mem_set(trace_addr, 0, trace_size);
        }
    }

    for (int i = 0; i < req_msg.request().size(); i++) {
        auto req = req_msg.mutable_request(i);
        if (req->has_spec()) {
            req->mutable_spec()->set_enable(true);
            req->mutable_spec()->set_reset(false);
        }
    }

    mpu_trace(req_msg);

    std::fstream output(cfg_proto, std::ios::out | std::ios::trunc | std::ios::binary);
    if (!req_msg.SerializeToOstream(&output)) {
        std::cerr << "Failed to write mpu trace config proto!" << std::endl;
        return -1;
    }

    return 0;
}

int mpu_trace_dump(const char *dump_file, std::string cfg_proto)
{
    FILE *fp = NULL;
    uint8_t buf[64] = { 0 };
    mpu_trace_record_t record = { 0 };

    debug::MpuTraceRequestMsg req_msg;

    std::fstream input(cfg_proto, std::ios::in | std::ios::binary);
    if (!req_msg.ParseFromIstream(&input)) {
        std::cerr << "Failed to read mpu trace config proto!" << std::endl;
        return -1;
    }

    fp = fopen(dump_file, "wb");
    if (fp == NULL) {
        std::cerr << "Failed to open dump file for writing!" << std::endl;
        return -1;
    }

    for (int i = 0; i < req_msg.request().size(); i++) {
        auto req = req_msg.request(i);

        memset(&record, 0, sizeof(record));

        record.pipeline_type = req.pipeline_type();
        record.stage_id      = req.stage_id();
        record.mpu           = req.mpu();
        record.trace_enable  = req.spec().trace_enable();
        record.phv_debug     = req.spec().phv_debug();
        record.phv_error     = req.spec().phv_error();
        record.watch_pc      = req.spec().watch_pc();
        record.table_key     = req.spec().table_key();
        record.instructions  = req.spec().instructions();
        record.wrap          = req.spec().wrap();
        record.trace_addr    = req.spec().base_addr();
        record.trace_size    = req.spec().buf_size();

        if (req.has_spec()) {
            // Write header
            fwrite(&record, sizeof(uint8_t), sizeof(record), fp);
            auto trace_addr = record.trace_addr;
            // Write trace buffer
            for (unsigned int i = 0; i < record.trace_size; i++) {
                sdk::lib::pal_mem_read(trace_addr, buf, sizeof(buf));
                fwrite(buf, sizeof(buf[0]), sizeof(buf), fp);
                trace_addr += sizeof(buf);
            }
        }
    }

    fclose(fp);

    return 0;
}

int mpu_trace_show(std::string cfg_proto)
{
    debug::MpuTraceRequestMsg req_msg;

    std::fstream input(cfg_proto, std::ios::in | std::ios::binary);
    if (!req_msg.ParseFromIstream(&input)) {
        std::cerr << "Failed to read mpu trace config proto!" << std::endl;
        return -1;
    }

    printf("%10s %10s %10s %10s %10s %10s %10s %10s %10s %10s %10s %10s %10s %10s %10s\n",
            "pipeline", "stage", "mpu",
            "enable", "wrap", "reset", 
            "trace", "phv_debug", "phv_error", "watch_pc",
            "table_kd", "instr",
            "trace_addr", "trace_nent", "trace_sz");

    for (int i = 0; i < req_msg.request().size(); i++) {
        auto req = req_msg.request(i);
        if (req.has_spec()) {
            printf("%10" PRIu8 " %10" PRIu32 " %10" PRIu32
                    " %10" PRIu8 " %10" PRIu8  " %10" PRIu8
                    " %10" PRIu8 " %10" PRIu8  " %10" PRIu8
                    " 0x%08" PRIx64
                    " %10" PRIu8 " %10" PRIu8
                    " 0x%08" PRIx64 " %10" PRIu32
                    " %10" PRIu32 "\n",
                    // selectors
                    req.pipeline_type(), req.stage_id(), req.mpu(),
                    // control
                    req.spec().enable(), req.spec().wrap(), req.spec().reset(),
                    // triggers
                    req.spec().trace_enable(), req.spec().phv_debug(), req.spec().phv_error(),
                    req.spec().watch_pc(),
                    // content
                    req.spec().table_key(), req.spec().instructions(),
                    // location
                    req.spec().base_addr(), req.spec().buf_size(),
                    TRACE_ENTRY_SIZE * req.spec().buf_size()
                );
        }
    }

    return 0;
}

void usage(char *argv[])
{
    std::cerr << "Usage: " << std::endl
              << "       " << argv[0] << " conf <cfg_file>" << std::endl
              << "       " << argv[0] << " dump <out_file>" << std::endl
              << "       " << argv[0] << " show" << std::endl
              << "       " << argv[0] << " enable" << std::endl
              << "       " << argv[0] << " disable" << std::endl
              << "       " << argv[0] << " reset" << std::endl;
}

int main(int argc, char *argv[])
{
    int ret = 0;

    if (argc < 2) {
        usage(argv);
        return -1;
    }

    std::map<std::string, int> oper = {
        {"conf", 1},
        {"enable", 2},
        {"disable", 3},
        {"dump", 4},
        {"reset", 5},
        {"show", 6},
    };

#ifdef __x86_64__
        assert(sdk::lib::pal_init(sdk::types::platform_type_t::PLATFORM_TYPE_SIM) == sdk::lib::PAL_RET_OK);
#elif __aarch64__
        assert(sdk::lib::pal_init(sdk::types::platform_type_t::PLATFORM_TYPE_HW) == sdk::lib::PAL_RET_OK);
#endif

    switch (oper[std::string((const char *)argv[1])]) {
    case 1:
        if (argc < 3) {
            std::cerr << "Specify config filename" << std::endl;
            usage(argv);
            ret = -2;
            break;
        }
        mpu_trace_config(argv[2], mpu_trace_proto_file_name);
        break;
    case 2:
        mpu_trace_enable(mpu_trace_proto_file_name);
        break;
    case 3:
        mpu_trace_disable(mpu_trace_proto_file_name);
        break;
    case 4:
        if (argc < 3) {
            std::cerr << "Specify dump filename" << std::endl;
            usage(argv);
            ret = -2;
            break;
        }
        mpu_trace_dump(argv[2], mpu_trace_proto_file_name);
        break;
    case 5:
        mpu_trace_reset(mpu_trace_proto_file_name);
        break;
    case 6:
        mpu_trace_show(mpu_trace_proto_file_name);
        break;
    default:
        std::cerr << "Invalid operation " << argv[1] << std::endl;
        usage(argv);
        ret = -3;
    }

    return ret;
}
