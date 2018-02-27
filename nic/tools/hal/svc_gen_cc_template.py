//:: import os
//:: import importlib
//:: import sys
//::
#include "nic/include/hal_cfg.hpp"
#include "nic/include/base.h"
#include "nic/include/trace.hpp" 
//::
//::  hdr_file = fileName.replace('_pb2.py', '') + '_svc_gen.hpp'
//::
#include "nic/gen/hal/svc/${hdr_file}"
//::
//:: if 'WS_TOP' not in os.environ:
//::     # This should have been set, before invoking the template.
//::     assert False
//:: #endif
//:: # Add the proto directory to the path.
//:: ws_top = os.environ['WS_TOP']
//:: fullpath = ws_top + '/nic/gen/proto/hal/'
//:: sys.path.insert(0, fullpath)
//::
//:: def convert_to_snake_case(name, fileName, input_name, output_name):
//::     import re
//::     import os
//::     global ws_top
//::     s1 = re.sub('([A-Z])(.)', r'\1_\2', name[::-1], 1).lower()
//::     s1 = s1[::-1]
//::     s2 = re.sub('([a-z0-9])([A-Z])', r'\1_\2', name).lower()
//::     hal_src_path = ws_top + '/nic/hal/src/'
//::     for file_name in os.listdir(hal_src_path):
//::         if file_name.endswith('.hpp') and fileName.replace('_pb2.py', '') in file_name:
//::             contents = open(os.path.join(hal_src_path, file_name)).read()
//::             #search_str = 'hal_ret_t ' + s1 + '.*' + input_name + '.*\n.*' + output_name + '.*\);'
//::             #re.search(search_str, contents) == None:
//::             if s1 in contents:
//::                 return s1
//::             #endif
//::             if s2 in contents:
//::                 print ('******************************* using ' + s2 + ' in ' + file_name + ' proto: ' + fileName)
//::                 return s2
//::             #endif
//::             print ('******************************* could not find method ' + s1 + ' or ' + s2 + ' in ' + file_name + ' proto: ' + fileName)
//::         #endif
//::     #endfor
//:: #enddef
//::
//:: fileModule = importlib.import_module(fileName[:-3])
//:: includeFileName = fileName[:-7]
//::
#include "nic/hal/src/${includeFileName}.hpp"

//:: # Remove the _pb2.py from file and store it for now.
//:: for service in fileModule.DESCRIPTOR.services_by_name.items():
//::     pkg = fileModule.DESCRIPTOR.package.lower()
//::
using grpc::ServerContext;
using grpc::Status;
using ${pkg}::${service[0]};
//::
//::     for method in service[1].methods_by_name.items():
//::         input_name  = pkg + '::' + method[1].input_type.name
//::         output_name = pkg + '::' + method[1].output_type.name
//::         repeated_field = False
//::         for field in method[1].output_type.fields:
//::             field_name = field.name
//::             if field.type == 11:
//::                 repeated_field = True
//::             else:
//::                 print ('*********** RESPONSE NOT REPEATED: ' + pkg)
//::             #endif
//::         #endfor
//::         hal_name = convert_to_snake_case(method[0], fileName, input_name, output_name)
//::         if 'Get' in method[0]:
//::             op = 'CFG_OP_READ'
//::             repeated_field = False
//::         else:
//::             op = 'CFG_OP_WRITE'
//::         #endif
//::
Status
${service[0]}ServiceImpl::${method[0]}(ServerContext *context,
                                       const ${input_name} *req,
                                       ${output_name} *rsp)
{
    uint32_t    i, nreqs = req->request_size();
    HAL_TRACE_DEBUG("Rcvd ${method[0]}");
    if (nreqs == 0) {
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Empty Request");
    }

    hal::hal_cfg_db_open(hal::${op});
    for (i = 0; i < nreqs; i++) {
        auto request = req->request(i);
//::
//::             if repeated_field == True:
//::
        auto response = rsp->add_response();
        hal::${hal_name}(request, response);
//::
//::             else:
//::
        hal::${hal_name}(request, rsp);
//::
//::             #endif
//::
    }
    hal::hal_cfg_db_close();
    return Status::OK;
}
//::
//::     #endfor
//:: #endfor
