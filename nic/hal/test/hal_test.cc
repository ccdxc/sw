#include <iostream>
#include <grpc++/grpc++.h>
#include <types.grpc.pb.h>
#include <tenant.grpc.pb.h>
#include <l2segment.grpc.pb.h>

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using tenant::Tenant;
using tenant::TenantSpec;
using tenant::TenantRequestMsg;
using tenant::TenantResponse;
using tenant::TenantResponseMsg;

using l2segment::L2Segment;
using l2segment::L2SegmentSpec;
using l2segment::L2SegmentRequestMsg;
using l2segment::L2SegmentResponse;
using l2segment::L2SegmentResponseMsg;

const std::string&    hal_svc_endpoint_("localhost:50052");

class hal_client {
public:
    hal_client(std::shared_ptr<Channel> channel) : tenant_stub_(Tenant::NewStub(channel)),
    l2seg_stub_(L2Segment::NewStub(channel)) {}

    uint64_t tenant_create(uint32_t tenant_id) {
        TenantSpec           *spec;
        TenantRequestMsg     req_msg;
        TenantResponse       *rsp;
        TenantResponseMsg    rsp_msg;
        ClientContext        context;
        Status               status;

        spec = req_msg.add_request();
        spec->mutable_key_or_handle()->set_tenant_id(tenant_id);

        status = tenant_stub_->TenantCreate(&context, req_msg, &rsp_msg);
        if (status.ok()) {
            assert(rsp_msg.response(0).api_status() == types::API_STATUS_OK);
            std::cout << "Tenant create succeeded, handle = "
                      << rsp_msg.response(0).tenant_status().tenant_handle()
                      << std::endl;
            return rsp_msg.response(0).tenant_status().tenant_handle();
        }
        std::cout << "Tenant create failed, error = "
                  << rsp_msg.response(0).api_status()
                  << std::endl;
        return 0;
    }

    uint64_t l2segment_create(uint32_t tenant_id,
                              uint64_t l2segment_id,
                              uint64_t l4_profile_handle) {
        L2SegmentSpec           *spec;
        L2SegmentRequestMsg     req_msg;
        L2SegmentResponse       *rsp;
        L2SegmentResponseMsg    rsp_msg;
        ClientContext           context;
        Status                  status;

        spec = req_msg.add_request();
        spec->mutable_meta()->set_tenant_id(tenant_id);
        spec->mutable_key_or_handle()->set_segment_id(l2segment_id);
        spec->mutable_access_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
        spec->mutable_access_encap()->set_encap_value(100);

        status = l2seg_stub_->L2SegmentCreate(&context, req_msg, &rsp_msg);
        if (status.ok()) {
            assert(rsp_msg.response(0).api_status() == types::API_STATUS_OK);
            std::cout << "L2 segment create succeeded, handle = "
                      << rsp_msg.response(0).l2segment_status().l2segment_handle()
                      << std::endl;
            return rsp_msg.response(0).l2segment_status().l2segment_handle();
        }
        std::cout << "L2 segment create failed, error = "
                  << rsp_msg.response(0).api_status()
                  << std::endl;
        return 0;
    }

private:
    std::unique_ptr<Tenant::Stub> tenant_stub_;
    std::unique_ptr<L2Segment::Stub> l2seg_stub_;
};

// main test driver
int
main (int argc, char** argv)
{
    hal_client hclient(grpc::CreateChannel(hal_svc_endpoint_,
                                           grpc::InsecureChannelCredentials()));

    // create a tenant
    assert(hclient.tenant_create(1) != 0);
    assert(hclient.l2segment_create(1, 1, 1) != 0);

    return 0;
}
