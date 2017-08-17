#ifndef __ACL_SVC_HPP__
#define __ACL_SVC_HPP__

#include <base.h>
#include <grpc++/grpc++.h>
#include <types.pb.h>
#include <acl.grpc.pb.h>

using grpc::ServerContext;
using grpc::Status;

using acl::Acl;
using acl::AclSpec;
using acl::AclKeyHandle;
using acl::AclRequestMsg;
using acl::AclResponse;
using acl::AclResponseMsg;
using acl::AclDeleteRequestMsg;
using acl::AclDeleteResponseMsg;
using acl::AclGetRequestMsg;
using acl::AclGetResponseMsg;

class AclServiceImpl final : public Acl::Service {
public:
    Status AclCreate(ServerContext *context,
                     const AclRequestMsg *req,
                     AclResponseMsg *rsp) override;

    Status AclUpdate(ServerContext *context,
                     const AclRequestMsg *req,
                     AclResponseMsg *rsp) override;

    Status AclDelete(ServerContext *context,
                     const AclDeleteRequestMsg *req,
                     AclDeleteResponseMsg *rsp) override;

    Status AclGet(ServerContext *context,
                  const AclGetRequestMsg *req,
                  AclGetResponseMsg *rsp) override;

};

#endif // __ACL_SVC_HPP__
