#include <iostream>
#include <memory>
#include <fstream>
#include <string>
#include <grpc++/grpc++.h>
#include "net.grpc.pb.h"

using namespace std;

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using net::VlanRequest;
using net::VlanResponse;
using net::Net;

const std::string&    nwAgentEndpoint("localhost:50051");

class VlanClient {
public:
    VlanClient(std::shared_ptr<Channel> channel)
        : stub_(Net::NewStub(channel)) {}

    int VlanCreate(types::L2SegmentType type,
                   uint32_t vlan_id,
                   types::EncapInfo *acc_encap,
                   types::EncapInfo *fab_encap) {
        VlanRequest         vlan_request;
        VlanResponse        vlan_response;
        ClientContext       context;

        // set up the vlan request message
        vlan_request.set_type(type);
        vlan_request.set_vlan_id(vlan_id);
        vlan_request.set_allocated_access_encap(acc_encap);
        vlan_request.set_allocated_fabric_encap(fab_encap);

        // create a vlan now
        Status status = stub_->VlanCreate(&context,
                                          vlan_request, &vlan_response);

        if (status.ok()) {
            return vlan_response.status();
        } else {
            std::cout << status.error_code() << " : " << status.error_message()
                      << std::endl;
            return -1;
        }
    }

private:
    std::unique_ptr<Net::Stub> stub_;
};

int main (int argc, char** argv)
{
    int                 rv;
    types::EncapInfo    *acc_encap, *fab_encap;

    VlanClient client(grpc::CreateChannel(nwAgentEndpoint,
                                          grpc::InsecureChannelCredentials()));

    acc_encap = new types::EncapInfo();
    acc_encap->set_encap_type(types::ENCAP_TYPE_DOT1Q);
    acc_encap->set_encap_value(100);

    fab_encap = new types::EncapInfo();
    fab_encap->set_encap_type(types::ENCAP_TYPE_VXLAN);
    fab_encap->set_encap_value(20000);

    rv = client.VlanCreate(types::L2_SEGMENT_TYPE_TENANT, 100,
                           acc_encap, fab_encap);
    std::cout << "Result received : " << rv << std::endl;

    return 0;
}
