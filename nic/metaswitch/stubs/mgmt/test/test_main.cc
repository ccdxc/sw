#include <iostream>
#include <memory>
#include <string>

#include <grpc++/grpc++.h>

#include <gen/proto/bgp.pb.h>
#include <gen/proto/bgp.grpc.pb.h>

#include <chrono>

#include <stdlib.h>
#include <unistd.h>

class Client {
 public:
    Client(std::shared_ptr<grpc::Channel> channel)
        : stub_(pds::BGPSvc::NewStub(channel)) {}

    void peerCreate() {
        pds::BGPPeerRequest req;
        pds::BGPResponse res;
        grpc::ClientContext context;

        auto ent = req.add_request();
        auto peeraddr = ent->mutable_peeraddr();
        peeraddr->set_af(types::IP_AF_INET);
        peeraddr->set_v4addr(0);
        ent->set_vrfid(1);
        ent->set_adminen(pds::BGP_ADMIN_UP);
        ent->set_peerport(3);
        auto localaddr = ent->mutable_localaddr();
        localaddr->set_af(types::IP_AF_INET);
        localaddr->set_v4addr(0);
        ent->set_localport(3);
        ent->set_ifid(0);
        ent->set_remoteasn(33);
        ent->set_localasn(33);
        ent->set_connectretry(1);
        ent->set_sendcomm(pds::BOOL_TRUE);
        ent->set_sendextcomm(pds::BOOL_TRUE);
        //ent->set_localnm(1);
        grpc::Status status = stub_->BGPPeerSpecCreate(&context, req, &res);

        if (status.ok()) {
            std::cout << "created 1 entry" << std::endl;
        } else {
            std::cout << status.error_code() << ": " << status.error_message()
                      << std::endl;
        }
    }

    void peerGet() {
        pds::BGPPeerRequest req;
        pds::BGPPeerSpecResponse res;
        grpc::ClientContext context;

        auto ent = req.add_request();
        auto peeraddr = ent->mutable_peeraddr();
        peeraddr->set_af(types::IP_AF_INET);
        peeraddr->set_v4addr(0);
        ent->set_vrfid(1);
        ent->set_peerport(3);
        auto localaddr = ent->mutable_localaddr();
        localaddr->set_af(types::IP_AF_INET);
        localaddr->set_v4addr(0);
        ent->set_localport(3);
        ent->set_ifid(0);
        grpc::Status status = stub_->BGPPeerSpecGet(&context, req, &res);

        if (status.ok()) {
            std::cout << "got 1 entry" << std::endl;
        } else {
            std::cout << status.error_code() << ": " << status.error_message()
                      << std::endl;
        }
    }

    void peerGetAll() {
        pds::BGPPeerRequest req;
        pds::BGPPeerSpecResponse res;
        grpc::ClientContext context;

        auto ent = req.add_request();
        auto peeraddr = ent->mutable_peeraddr();
        peeraddr->set_af(types::IP_AF_INET);
        peeraddr->set_v4addr(0);
        ent->set_vrfid(1);
        ent->set_peerport(3);
        auto localaddr = ent->mutable_localaddr();
        localaddr->set_af(types::IP_AF_INET);
        localaddr->set_v4addr(0);
        ent->set_localport(3);
        ent->set_ifid(0);
        grpc::Status status = stub_->BGPPeerSpecGetAll(&context, req, &res);

        if (status.ok()) {
            std::cout << "got entire table" << std::endl;
        } else {
            std::cout << status.error_code() << ": " << status.error_message()
                      << std::endl;
        }
    }
 private:
    std::unique_ptr<pds::BGPSvc::Stub> stub_;
};

int main(int argc, char** argv) {
    Client client(grpc::CreateChannel("localhost:50057",
            grpc::InsecureChannelCredentials()));


    client.peerCreate();
    sleep(5);
    client.peerGet();
    sleep(25);
    client.peerGetAll();

    return 0;
}

