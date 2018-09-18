#ifndef __ACCEL_RGROUP_SVC_HPP__
#define __ACCEL_RGROUP_SVC_HPP__

#include "nic/include/base.hpp"
#include "grpc++/grpc++.h"
#include "nic/gen/proto/hal/types.pb.h"
#include "nic/gen/proto/hal/accel_rgroup.grpc.pb.h"

using grpc::ServerContext;
using grpc::Status;

using accelRGroup::AccelRGroup;
using accelRGroup::AccelRGroupAddRequest;
using accelRGroup::AccelRGroupAddRequestMsg;
using accelRGroup::AccelRGroupAddResponse;
using accelRGroup::AccelRGroupAddResponseMsg;
using accelRGroup::AccelRGroupDelRequest;
using accelRGroup::AccelRGroupDelRequestMsg;
using accelRGroup::AccelRGroupDelResponse;
using accelRGroup::AccelRGroupDelResponseMsg;
using accelRGroup::AccelRGroupRingAddRequest;
using accelRGroup::AccelRGroupRingAddRequestMsg;
using accelRGroup::AccelRGroupRingAddResponse;
using accelRGroup::AccelRGroupRingAddResponseMsg;
using accelRGroup::AccelRGroupRingDelRequest;
using accelRGroup::AccelRGroupRingDelRequestMsg;
using accelRGroup::AccelRGroupRingDelResponse;
using accelRGroup::AccelRGroupRingDelResponseMsg;
using accelRGroup::AccelRGroupResetSetRequest;
using accelRGroup::AccelRGroupResetSetRequestMsg;
using accelRGroup::AccelRGroupResetSetResponse;
using accelRGroup::AccelRGroupResetSetResponseMsg;
using accelRGroup::AccelRGroupEnableSetRequest;
using accelRGroup::AccelRGroupEnableSetRequestMsg;
using accelRGroup::AccelRGroupEnableSetResponse;
using accelRGroup::AccelRGroupEnableSetResponseMsg;
using accelRGroup::AccelRGroupPndxSetRequest;
using accelRGroup::AccelRGroupPndxSetRequestMsg;
using accelRGroup::AccelRGroupPndxSetResponse;
using accelRGroup::AccelRGroupPndxSetResponseMsg;
using accelRGroup::AccelRGroupInfoGetRequest;
using accelRGroup::AccelRGroupInfoGetRequestMsg;
using accelRGroup::AccelRGroupInfoGetResponse;
using accelRGroup::AccelRGroupInfoGetResponseMsg;
using accelRGroup::AccelRGroupIndicesGetRequest;
using accelRGroup::AccelRGroupIndicesGetRequestMsg;
using accelRGroup::AccelRGroupIndicesGetResponse;
using accelRGroup::AccelRGroupIndicesGetResponseMsg;

class AccelRGroupServiceImpl final : public AccelRGroup::Service {

public:
    Status AccelRGroupAdd(ServerContext* context,
                          const AccelRGroupAddRequestMsg* request,
                          AccelRGroupAddResponseMsg* response) override;
    Status AccelRGroupDel(ServerContext *context,
                          const AccelRGroupDelRequestMsg *request,
                          AccelRGroupDelResponseMsg *response) override;
    Status AccelRGroupRingAdd(ServerContext *context,
                              const AccelRGroupRingAddRequestMsg *request,
                              AccelRGroupRingAddResponseMsg *response) override;
    Status AccelRGroupRingDel(ServerContext *context,
                              const AccelRGroupRingDelRequestMsg *request,
                              AccelRGroupRingDelResponseMsg *response) override;
    Status AccelRGroupResetSet(ServerContext *context,
                               const AccelRGroupResetSetRequestMsg *request,
                               AccelRGroupResetSetResponseMsg *response) override;
    Status AccelRGroupEnableSet(ServerContext *context,
                                const AccelRGroupEnableSetRequestMsg *request,
                                AccelRGroupEnableSetResponseMsg *response) override;
    Status AccelRGroupPndxSet(ServerContext *context,
                              const AccelRGroupPndxSetRequestMsg *request,
                              AccelRGroupPndxSetResponseMsg *response) override;
    Status AccelRGroupInfoGet(ServerContext *context,
                              const AccelRGroupInfoGetRequestMsg *request,
                              AccelRGroupInfoGetResponseMsg *response) override;
    Status AccelRGroupIndicesGet(ServerContext *context,
                                 const AccelRGroupIndicesGetRequestMsg *request,
                                 AccelRGroupIndicesGetResponseMsg *response) override;
};

#endif  /* __ACCEL_RGROUP_SVC_HPP__ */
