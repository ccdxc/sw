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

    void specCreate() {
        pds::BGPRequest req;
        pds::BGPResponse res;
        grpc::ClientContext context;

        auto ent = req.mutable_request();
        ent->set_id(std::to_string(11));
        ent->set_localasn(2);
        ent->set_routerid(3);
        grpc::Status status = stub_->BGPCreate(&context, req, &res);

        if (status.ok()) {
            std::cout << "inserted 1 more entry into rment table" << std::endl;
        } else {
            std::cout << status.error_code() << ": " << status.error_message()
                      << std::endl;
        }
    }

    void peerCreate() {
        pds::BGPPeerRequest req;
        pds::BGPPeerResponse res;
        grpc::ClientContext context;

        auto ent = req.add_request();
        auto peeraddr = ent->mutable_peeraddr();
        peeraddr->set_af(types::IP_AF_INET);
        peeraddr->set_v4addr(1);
        ent->set_id(std::to_string(1));
        ent->set_state(types::ADMIN_STATE_ENABLE);
        auto localaddr = ent->mutable_localaddr();
        localaddr->set_af(types::IP_AF_INET);
        localaddr->set_v4addr(1);
        ent->set_remoteasn(1);
        ent->set_connectretry(1);
        ent->set_sendcomm(true);
        ent->set_sendextcomm(true);
        //ent->set_localnm(1);
        grpc::Status status = stub_->BGPPeerCreate(&context, req, &res);

        if (status.ok()) {
            std::cout << "inserted 1 more entry into peer table" << std::endl;
        } else {
            std::cout << status.error_code() << ": " << status.error_message()
                      << std::endl;
        }
    }

    void peerGet() {
        pds::BGPPeerGetRequest req;
        pds::BGPPeerGetResponse res;
        grpc::ClientContext context;

        auto keyh = req.add_request();
        auto ent = keyh->mutable_key();
        auto peeraddr = ent->mutable_peeraddr();
        peeraddr->set_af(types::IP_AF_INET);
        peeraddr->set_v4addr(1);
        auto localaddr = ent->mutable_localaddr();
        localaddr->set_af(types::IP_AF_INET);
        localaddr->set_v4addr(1);

        grpc::Status status = stub_->BGPPeerGet(&context, req, &res);

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
                std::cout << "state: " << resp.state() << std::endl;
                std::cout << "sendcomm: " << resp.sendcomm() << std::endl;
                std::cout << "remoteasn: " << resp.remoteasn() << std::endl;
            }
        } else {
            std::cout << status.error_code() << ": " << status.error_message()
                      << std::endl;
        }
    }

    void peerDelete() {
        pds::BGPPeerDeleteRequest req;
        pds::BGPPeerDeleteResponse res;
        grpc::ClientContext context;

        auto keyh = req.add_request();
        auto ent = keyh->mutable_key();
        auto peeraddr = ent->mutable_peeraddr();
        peeraddr->set_af(types::IP_AF_INET);
        peeraddr->set_v4addr(1);
        auto localaddr = ent->mutable_localaddr();
        localaddr->set_af(types::IP_AF_INET);
        localaddr->set_v4addr(1);
        grpc::Status status = stub_->BGPPeerDelete(&context, req, &res);

        if (status.ok()) {
            std::cout << "deleted 1 entry" << std::endl;
        } else {
            std::cout << status.error_code() << ": " << status.error_message()
                      << std::endl;
        }
    }

    void peerGetAll() {
        pds::BGPPeerGetRequest req;
        pds::BGPPeerGetResponse res;
        grpc::ClientContext context;

        grpc::Status status = stub_->BGPPeerGet(&context, req, &res);

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
                std::cout << "state: " << resp.state() << std::endl;
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
    Client client(grpc::CreateChannel("localhost:50054",
            grpc::InsecureChannelCredentials()));


    client.peerGetAll();
    client.peerCreate();
    client.specCreate();
    client.peerGet();
//    sleep(25);
    client.peerGetAll();
    client.peerGetAll();

    return 0;
}

