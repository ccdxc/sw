#include <grpc++/grpc++.h>
#include <base.h>
#include <nw_svc.hpp>

using grpc::Status;

Status
NetServiceImpl::VlanCreate(ServerContext* context,
                           const VlanRequest* req, VlanResponse* rsp)
{
    std::cout << "Vlan Create Request Rcvd" << std::endl;
    rsp->set_status(HAL_RET_OK);
    return Status::OK;
}

Status
NetServiceImpl::VlanUpdate(ServerContext* context,
                           const VlanRequest* req, VlanResponse* rsp) 
{
    std::cout << "Vlan Delete Request Rcvd" << std::endl;
    rsp->set_status(HAL_RET_OK);
    return Status::OK;
}
