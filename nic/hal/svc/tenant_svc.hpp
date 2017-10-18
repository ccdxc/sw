#ifndef __TENANT_SVC_HPP__
#define __TENANT_SVC_HPP__

#include "nic/include/base.h"
#include "grpc++/grpc++.h"
#include "nic/gen/proto/hal/types.pb.h"
#include "nic/gen/proto/hal/tenant.grpc.pb.h"

using grpc::ServerContext;
using grpc::Status;

using tenant::Tenant;
using tenant::TenantSpec;
using tenant::TenantStatus;
using tenant::TenantResponse;
using tenant::TenantKeyHandle;
using tenant::TenantRequestMsg;
using tenant::TenantResponseMsg;
using tenant::TenantDeleteRequest;
using tenant::TenantDeleteRequestMsg;
using tenant::TenantDeleteResponseMsg;
using tenant::TenantGetRequest;
using tenant::TenantGetRequestMsg;
using tenant::TenantGetResponse;
using tenant::TenantGetResponseMsg;

class TenantServiceImpl final : public Tenant::Service {
public:
    Status TenantCreate(ServerContext *context,
                        const TenantRequestMsg *req,
                        TenantResponseMsg *rsp) override;

    Status TenantUpdate(ServerContext *context,
                        const TenantRequestMsg *req,
                        TenantResponseMsg *rsp) override;


    Status TenantDelete(ServerContext *context,
                        const TenantDeleteRequestMsg *req,
                        TenantDeleteResponseMsg *rsp) override;

    Status TenantGet(ServerContext *context,
                     const TenantGetRequestMsg *req,
                     TenantGetResponseMsg *rsp) override;
};
#endif    // __TENANT_SVC_HPP__

