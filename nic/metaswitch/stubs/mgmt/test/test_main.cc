#include <iostream>
#include <memory>
#include <string>
#include <grpc++/grpc++.h>
#include <gen/proto/bgp.pb.h>
#include <gen/proto/bgp.grpc.pb.h>
#include <chrono>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

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
        peeraddr->set_v4addr(1);
        ent->set_id(std::to_string(1));
        ent->set_adminen(pds::ADMIN_UP);
        auto localaddr = ent->mutable_localaddr();
        localaddr->set_af(types::IP_AF_INET);
        localaddr->set_v4addr(1);
        ent->set_ifid(0);
        ent->set_remoteasn(1);
        ent->set_connectretry(1);
        ent->set_sendcomm(pds::BOOL_TRUE);
        ent->set_sendextcomm(pds::BOOL_TRUE);
        //ent->set_localnm(1);
        grpc::Status status = stub_->BGPPeerSpecCreate(&context, req, &res);

        if (status.ok()) {
            std::cout << "inserted 1 more entry into peer table" << std::endl;
        } else {
            std::cout << status.error_code() << ": " << status.error_message()
                      << std::endl;
        }
    }

    void peerGet() {
        pds::BGPPeerRequest req;
        pds::BGPPeerGetResponse res;
        grpc::ClientContext context;

        auto ent = req.add_request();
        auto peeraddr = ent->mutable_peeraddr();
        peeraddr->set_af(types::IP_AF_INET);
        peeraddr->set_v4addr(1);
        ent->set_id(std::to_string(1));
        ent->set_adminen(pds::ADMIN_UP);
        auto localaddr = ent->mutable_localaddr();
        localaddr->set_af(types::IP_AF_INET);
        localaddr->set_v4addr(1);
        ent->set_ifid(0);
        ent->set_remoteasn(1);

        grpc::Status status = stub_->BGPPeerSpecGet(&context, req, &res);

        if (status.ok()) {
            std::cout << "got 1 entry" << std::endl;
            for (int i=0; i<res.response_size(); i++) {
                auto resp = res.response(i).spec();
                std::cout << "===== Entry #" << i << " =====" << std::endl;
                std::cout << "vrfid: " << resp.id() << std::endl;
                auto paddr = resp.peeraddr().v4addr();
                struct in_addr ip_addr;
                ip_addr.s_addr = paddr;
                std::cout << "peeraf: " << resp.peeraddr().af() << " peeraddr: " << inet_ntoa(ip_addr) << std::endl;
                paddr = resp.localaddr().v4addr();
                ip_addr.s_addr = paddr;
                std::cout << "localaf: " << resp.localaddr().af() << " localaddr: " << inet_ntoa(ip_addr) << std::endl;
                std::cout << "ifid: " << resp.ifid() << std::endl;
                std::cout << "adminen: " << resp.adminen() << std::endl;
                std::cout << "sendcomm: " << resp.sendcomm() << std::endl;
                std::cout << "remoteasn: " << resp.remoteasn() << std::endl;
            }
        } else {
            std::cout << status.error_code() << ": " << status.error_message()
                      << std::endl;
        }
    }

    void peerDelete() {
        pds::BGPPeerRequest req;
        pds::BGPResponse res;
        grpc::ClientContext context;

        auto ent = req.add_request();
        auto peeraddr = ent->mutable_peeraddr();
        peeraddr->set_af(types::IP_AF_INET);
        peeraddr->set_v4addr(1);
        ent->set_id(std::to_string(1));
        ent->set_adminen(pds::ADMIN_UP);
        auto localaddr = ent->mutable_localaddr();
        localaddr->set_af(types::IP_AF_INET);
        localaddr->set_v4addr(1);
        ent->set_ifid(0);
        ent->set_remoteasn(1);
        grpc::Status status = stub_->BGPPeerSpecDelete(&context, req, &res);

        if (status.ok()) {
            std::cout << "deleted 1 entry" << std::endl;
        } else {
            std::cout << status.error_code() << ": " << status.error_message()
                      << std::endl;
        }
    }

    void peerGetAll() {
        pds::BGPPeerRequest req;
        pds::BGPPeerGetResponse res;
        grpc::ClientContext context;

        grpc::Status status = stub_->BGPPeerSpecGet(&context, req, &res);

        if (status.ok()) {
            std::cout << "got peer status table with " << res.response_size() << " entry" << std::endl;
            for (int i=0; i<res.response_size(); i++) {
                auto resp = res.response(i).spec();
		std::cout << "===== Entry #" << i << " =====" << std::endl;
                std::cout << "vrfid: " << resp.id() << std::endl;
		auto paddr = resp.peeraddr().v4addr();
		struct in_addr ip_addr;
		ip_addr.s_addr = paddr;
                std::cout << "peeraf: " << resp.peeraddr().af() << " peeraddr: " << inet_ntoa(ip_addr) << std::endl;
		paddr = resp.localaddr().v4addr();
		ip_addr.s_addr = paddr;
                std::cout << "localaf: " << resp.localaddr().af() << " localaddr: " << inet_ntoa(ip_addr) << std::endl;
                std::cout << "ifid: " << resp.ifid() << std::endl;
                std::cout << "adminen: " << resp.adminen() << std::endl;
                std::cout << "sendcomm: " << resp.sendcomm() << std::endl;
                std::cout << "remoteasn: " << resp.remoteasn() << std::endl;
            }
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


    client.peerGetAll();
    client.peerCreate();
    //client.peerGet();
//    sleep(25);
    client.peerGetAll();
//    client.peerDelete();
    client.peerGetAll();

    return 0;
}

