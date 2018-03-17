//:: import os, pdb
//:: from collections import OrderedDict
//:: pddict = _context['pddict']
//:: if pddict['p4plus']:
//::    p4prog = pddict['p4program'] + '_'
//::    hdrdir = pddict['p4program']
//::    caps_p4prog = '_' + pddict['p4program'].upper() + '_'
//::    prefix = 'p4pd_' + pddict['p4program']
//::    if pddict['p4program'] == 'common_rxdma_actions':
//::        start_table_base = 101
//::    elif pddict['p4program'] == 'common_txdma_actions':
//::	    start_table_base = 201
//::    else:
//::	    start_table_base = 301
//::    #endif
//:: else:
//::    p4prog = ''
//::    hdrdir = pddict['p4program']
//::    caps_p4prog = ''
//::    prefix = 'p4pd'
//::	start_table_base = 1
//:: #endif
//:: #define here any constants needed.
//:: ACTION_PC_LEN = 8 # in bits
/*
 * p4pd.c
 * Pensando Systems
 */
#include "nic/gen/proto/hal/debug.pb.h"
#include "nic/gen/proto/hal/debug.grpc.pb.h"
#include <grpc++/grpc++.h>
#include "nic/gen/${hdrdir}/include/${p4prog}p4pd.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using ::debug::Debug;
using ::debug::DebugRequestMsg;
using ::debug::DebugSpec;
using ::debug::DebugKeyHandle;
using ::debug::DebugResponseMsg;
//::
//::     tabledict = OrderedDict() # key=table-name
//::     tableid = start_table_base
//::     for table in pddict['tables']:
//::        tabledict[table] = tableid
//::        tableid += 1
//::     #endfor
//::
//::    if len(tabledict):
//::        if pddict['p4plus']:
//::            api_prefix = 'p4pd_' + pddict['p4program']
//::        else:
//::            api_prefix = 'p4pd'
//::        #endif
//::    #endif

static std::shared_ptr<Channel>     channel;
static std::unique_ptr<Debug::Stub> stub;

p4pd_error_t
${api_prefix}_cli_init(char *grpc_server_port)
{
    grpc_init();

    channel =
        grpc::CreateChannel(grpc_server_port, grpc::InsecureChannelCredentials());

    stub    = ::debug::Debug::NewStub(channel);

    return (P4PD_SUCCESS);
}

p4pd_error_t
${api_prefix}_entry_write(uint32_t tableid,
                          uint32_t index,
                          void     *swkey,
                          void     *swkey_mask,
                          void     *actiondata)
{
    DebugRequestMsg  debug_req_msg;
    DebugResponseMsg debug_rsp_msg;
    ClientContext    context;
    DebugSpec        *debug_spec    = NULL;
    DebugKeyHandle   *key_or_handle = NULL;

    debug_spec = debug_req_msg.add_request();

    key_or_handle = debug_spec->mutable_key_or_handle();

    key_or_handle->set_table_id(tableid);

    debug_spec->set_index(index);
    debug_spec->set_opn_type(::debug::DEBUG_OP_TYPE_WRITE);

    switch (tableid) {
        //::        for table, tid in tabledict.items():
        //::            caps_tablename = table.upper()
        //::            if pddict['tables'][table]['hash_overflow'] and not pddict['tables'][table]['otcam']:
        //::                continue
        //::            #endif
        case P4${caps_p4prog}TBL_ID_${caps_tablename}: /* p4-table '${table}' */
        //::            if len(pddict['tables'][table]['hash_overflow_tbl']):
        //::                tbl_ = pddict['tables'][table]['hash_overflow_tbl']
        //::                caps_tbl_ = tbl_.upper()
        case P4${caps_p4prog}TBL_ID_${caps_tbl_}: /* p4-table '${tbl_}' */
        //::            #endif
        //::            if pddict['tables'][table]['type'] == 'Index' or pddict['tables'][table]['type'] == 'Mpu':
            debug_spec->set_actiondata(
                        std::string ((char*)(${table}_actiondata*)actiondata,
                                     sizeof(${table}_actiondata)));
            break;

        //::            #endif
        //::            if pddict['tables'][table]['type'] == 'Hash' or pddict['tables'][table]['type'] == 'Hash_OTcam':
            debug_spec->set_swkey(
                        std::string ((char*)(${table}_swkey_t*)swkey,
                                     sizeof(${table}_swkey_t)));

            debug_spec->set_actiondata(
                        std::string ((char*)(${table}_actiondata*)actiondata,
                                     sizeof(${table}_actiondata)));
            break;

        //::            #endif
        //::            if pddict['tables'][table]['type'] == 'Ternary':
            debug_spec->set_swkey(
                        std::string ((char*)(${table}_swkey_t*)swkey,
                                     sizeof(${table}_swkey_t)));

            debug_spec->set_swkey_mask(
                        std::string ((char*)(${table}_swkey_mask_t*)swkey_mask,
                                     sizeof(${table}_swkey_mask_t)));

            debug_spec->set_actiondata(
                        std::string ((char*)(${table}_actiondata*)actiondata,
                                     sizeof(${table}_actiondata)));
            break;

        //::            #endif
        //::        #endfor
        default:
            // Invalid tableid
            return (P4PD_FAIL);
            break;
    }

    Status status = stub->DebugInvoke(&context, debug_req_msg, &debug_rsp_msg);

    return (P4PD_SUCCESS);
}

p4pd_error_t
${api_prefix}_entry_read(uint32_t  tableid,
                         uint32_t  index,
                         void      *swkey, 
                         void      *swkey_mask,
                         void      *actiondata)
{
    DebugRequestMsg  debug_req_msg;
    DebugResponseMsg debug_rsp_msg;
    ClientContext    context;
    DebugSpec        *debug_spec    = NULL;
    DebugKeyHandle   *key_or_handle = NULL;

    debug_spec = debug_req_msg.add_request();

    key_or_handle = debug_spec->mutable_key_or_handle();

    key_or_handle->set_table_id(tableid);

    debug_spec->set_opn_type(::debug::DEBUG_OP_TYPE_READ);
    debug_spec->set_index(index);

    switch (tableid) {
        //::        for table, tid in tabledict.items():
        //::            caps_tablename = table.upper()
        //::            if pddict['tables'][table]['hash_overflow'] and not pddict['tables'][table]['otcam']:
        //::                continue
        //::            #endif
        case P4${caps_p4prog}TBL_ID_${caps_tablename}: /* p4-table '${table}' */
            //::            if len(pddict['tables'][table]['hash_overflow_tbl']):
            //::                tbl_ = pddict['tables'][table]['hash_overflow_tbl']
            //::                caps_tbl_ = tbl_.upper()
        case P4${caps_p4prog}TBL_ID_${caps_tbl_}: /* p4-table '${tbl_}' */
            //::            #endif
            //::            if pddict['tables'][table]['type'] == 'Index' or pddict['tables'][table]['type'] == 'Mpu':
            debug_spec->set_actiondata(
                        std::string ((char*)(${table}_actiondata*)actiondata,
                                     sizeof(${table}_actiondata)));
            break;

            //::            #endif
            //::            if pddict['tables'][table]['type'] == 'Hash' or pddict['tables'][table]['type'] == 'Hash_OTcam':
            debug_spec->set_actiondata(
                        std::string ((char*)(${table}_actiondata*)actiondata,
                                     sizeof(${table}_actiondata)));
            break;

            //::            #endif
            //::            if pddict['tables'][table]['type'] == 'Ternary':
            debug_spec->set_actiondata(
                        std::string ((char*)(${table}_actiondata*)actiondata,
                                     sizeof(${table}_actiondata)));
            break;
            //::            #endif
            //::        #endfor
        default:
            // Invalid tableid
            return (P4PD_FAIL);
            break;
    }

    Status status = stub->DebugInvoke(&context, debug_req_msg, &debug_rsp_msg);

    switch (tableid) {
        //::        for table, tid in tabledict.items():
        //::            caps_tablename = table.upper()
        //::            if pddict['tables'][table]['hash_overflow'] and not pddict['tables'][table]['otcam']:
        //::                continue
        //::            #endif
        case P4${caps_p4prog}TBL_ID_${caps_tablename}: /* p4-table '${table}' */
            //::            if len(pddict['tables'][table]['hash_overflow_tbl']):
            //::                tbl_ = pddict['tables'][table]['hash_overflow_tbl']
            //::                caps_tbl_ = tbl_.upper()
        case P4${caps_p4prog}TBL_ID_${caps_tbl_}: /* p4-table '${tbl_}' */
            //::            #endif
            //::            if pddict['tables'][table]['type'] == 'Index' or pddict['tables'][table]['type'] == 'Mpu':
            memcpy(actiondata, (void*)debug_rsp_msg.response(0).spec().actiondata().c_str(),
                   sizeof(${table}_actiondata));
            break;

            //::            #endif
            //::            if pddict['tables'][table]['type'] == 'Hash' or pddict['tables'][table]['type'] == 'Hash_OTcam':
            memcpy(swkey, (void*)debug_rsp_msg.response(0).spec().swkey().c_str(),
                   sizeof(${table}_swkey_t));

            memcpy(actiondata, (void*)debug_rsp_msg.response(0).spec().actiondata().c_str(),
                   sizeof(${table}_actiondata));
            break;

            //::            #endif
            //::            if pddict['tables'][table]['type'] == 'Ternary':
            memcpy(swkey, (void*)debug_rsp_msg.response(0).spec().swkey().c_str(),
                   sizeof(${table}_swkey_t));

            memcpy(swkey_mask, (void*)debug_rsp_msg.response(0).spec().swkey_mask().c_str(),
                   sizeof(${table}_swkey_mask_t));

            memcpy(actiondata, (void*)debug_rsp_msg.response(0).spec().actiondata().c_str(),
                   sizeof(${table}_actiondata));
            break;
            //
            //::            #endif
            //::        #endfor
        default:
            // Invalid tableid
            return (P4PD_FAIL);
            break;
    }
    return (P4PD_SUCCESS);
}

void
p4pd_register_entry_read(std::string block_name, std::string   reg_name, std::string filename)
{
    DebugRequestMsg    debug_req_msg;
    DebugResponseMsg   debug_rsp_msg;
    ClientContext      context;
    DebugSpec          *debug_spec = NULL;
    DebugKeyHandle     *key_or_handle = NULL;

    debug_spec = debug_req_msg.add_request();

    key_or_handle = debug_spec->mutable_key_or_handle();

    key_or_handle->set_reg_name(reg_name);
    key_or_handle->set_block_name(block_name);
    debug_spec->set_mem_type(::debug::DEBUG_MEM_TYPE_REG);

    debug_spec->set_opn_type(::debug::DEBUG_OP_TYPE_READ);

    auto channel =
        grpc::CreateChannel("localhost:50054", grpc::InsecureChannelCredentials());

    auto stub = ::debug::Debug::NewStub(channel);

    Status status = stub->DebugInvoke(&context, debug_req_msg, &debug_rsp_msg);

    int reg_data_sz  = debug_rsp_msg.response(0).data_size();
    //*len = reg_data_sz;
    FILE *reg_fd = 0;
    if (!filename.empty()) {
        reg_fd = fopen(filename.c_str(), "wb+");
        if (!reg_fd) {
            std::cout <<"Null file descriptor";
            return;
        }
    }

    for (uint32_t i = 0; i  < reg_data_sz; i++) {
        //data[i].reg_name = debug_rsp_msg.response(0).data(i).reg_name();
        //data[i].offset = debug_rsp_msg.response(0).data(i).address();
        //data[i].value = debug_rsp_msg.response(0).data(i).value();
        if (filename.empty()) {
            std::cout << "RegName: " << debug_rsp_msg.response(0).data(i).reg_name() << "Address(Offset): " <<  debug_rsp_msg.response(0).data(i).address() << "Value: "<< debug_rsp_msg.response(0).data(i).value() << "\n";
        } else {

            std::fprintf(reg_fd, "RegName:  %s Address: %s Value: %s \n", debug_rsp_msg.response(0).data(i).reg_name().c_str(), debug_rsp_msg.response(0).data(i).address().c_str(),  debug_rsp_msg.response(0).data(i).value().c_str());
            std::fflush(reg_fd);
        }
    }
    return;;

}


void
p4pd_register_list(std::string block_name, std::string   reg_name, std::string filename)
{
    DebugRequestMsg    debug_req_msg;
    DebugResponseMsg   debug_rsp_msg;
    ClientContext      context;
    DebugSpec          *debug_spec = NULL;
    DebugKeyHandle     *key_or_handle = NULL;

    debug_spec = debug_req_msg.add_request();

    key_or_handle = debug_spec->mutable_key_or_handle();

    key_or_handle->set_reg_name(reg_name);
    key_or_handle->set_block_name(block_name);
    debug_spec->set_mem_type(::debug::DEBUG_MEM_TYPE_REG);

    debug_spec->set_opn_type(::debug::DEBUG_OP_TYPE_READ);

    auto channel =
        grpc::CreateChannel("localhost:50054", grpc::InsecureChannelCredentials());

    auto stub = ::debug::Debug::NewStub(channel);

    Status status = stub->DebugInvoke(&context, debug_req_msg, &debug_rsp_msg);

    int reg_data_sz  = debug_rsp_msg.response(0).data_size();
    FILE *reg_fd = 0;
    if (!filename.empty()) {
        reg_fd = fopen(filename.c_str(), "wb+");
        if (!reg_fd) {
            std::cout <<"Null file descriptor";
            return;
        }
    }

    for (uint32_t i = 0; i  < reg_data_sz; i++) {
        if (filename.empty()) {
            std::cout << "RegName: " << debug_rsp_msg.response(0).data(i).reg_name() << "\n";
        } else {

            std::fprintf(reg_fd, "RegName:  %s \n", debug_rsp_msg.response(0).data(i).reg_name().c_str()); 
            std::fflush(reg_fd);
        }
    }
    return;;

}

