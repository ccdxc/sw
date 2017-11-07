#include <iostream>
#include <thread>
#include <grpc++/grpc++.h>
#include "nic/gen/proto/hal/types.grpc.pb.h"
#include "nic/gen/proto/hal/tenant.grpc.pb.h"
#include "nic/gen/proto/hal/l2segment.grpc.pb.h"
#include "nic/gen/proto/hal/port.grpc.pb.h"
#include "nic/gen/proto/hal/event.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using tenant::Tenant;
using tenant::TenantSpec;
using tenant::TenantRequestMsg;
using tenant::TenantResponse;
using tenant::TenantResponseMsg;
using tenant::TenantGetRequestMsg;
using tenant::TenantGetRequest;
using tenant::TenantGetResponseMsg;
using tenant::TenantDeleteRequestMsg;
using tenant::TenantDeleteRequest;
using tenant::TenantDeleteResponseMsg;

using l2segment::L2Segment;
using l2segment::L2SegmentSpec;
using l2segment::L2SegmentRequestMsg;
using l2segment::L2SegmentResponse;
using l2segment::L2SegmentResponseMsg;

using port::Port;
using port::PortSpec;
using port::PortRequestMsg;
using port::PortResponse;
using port::PortResponseMsg;
using port::PortGetRequest;
using port::PortGetRequestMsg;
using port::PortGetResponse;
using port::PortGetResponseMsg;
using port::PortDeleteRequest;
using port::PortDeleteRequestMsg;
using port::PortDeleteResponseMsg;

using event::Event;
using event::EventRequest;
using event::EventResponse;

const std::string&    hal_svc_endpoint_("localhost:50054");

class hal_client {
public:
    hal_client(std::shared_ptr<Channel> channel) : tenant_stub_(Tenant::NewStub(channel)),
    l2seg_stub_(L2Segment::NewStub(channel)), port_stub_(Port::NewStub(channel)),
    event_stub_(Event::NewStub(channel)) {}

    bool port_handle_api_status(types::ApiStatus api_status,
                                uint32_t port_id) {
        switch(api_status) {
            case types::API_STATUS_OK:
                return true;

            case types::API_STATUS_PORT_NOT_FOUND:
                std::cout << "Port "
                          << port_id
                          << " not found"
                          << std::endl;
                return false;

            case types::API_STATUS_EXISTS_ALREADY:
                std::cout << "Port "
                          << port_id
                          << " exists already"
                          << std::endl;
                return false;

            default:
                assert(0);
        }

        return true;
    }

    int port_create(uint32_t tenant_id, uint32_t port_id) {
        PortSpec            *spec;
        PortRequestMsg      req_msg;
        PortResponseMsg     rsp_msg;
        ClientContext       context;
        Status               status;

        spec = req_msg.add_request();
        spec->mutable_key_or_handle()->set_port_id(port_id);
        spec->mutable_meta()->set_tenant_id(tenant_id);
        spec->set_port_speed(::port::PORT_SPEED_25G);
        spec->set_num_lanes(1);
        spec->set_port_type(::port::PORT_TYPE_ETH);
        spec->set_admin_state(::port::PORT_ADMIN_STATE_UP);

        status = port_stub_->PortCreate(&context, req_msg, &rsp_msg);
        if (status.ok()) {
            if (port_handle_api_status(
                    rsp_msg.response(0).api_status(), port_id) == true) {
                std::cout << "Port create succeeded for port "
                          << port_id
                          << std::endl;
            }

            return 0;
        }

        std::cout << "Port create failed for port "
                  << port_id
                  << " , error = " << rsp_msg.response(0).api_status()
                  << std::endl;
        return -1;
    }

    int port_update(uint32_t tenant_id,
                    uint32_t port_id,
                    ::port::PortSpeed speed,
                    ::port::PortAdminState admin_state) {
        PortSpec            *spec;
        PortRequestMsg      req_msg;
        PortResponseMsg     rsp_msg;
        ClientContext       context;
        Status               status;

        spec = req_msg.add_request();
        spec->mutable_key_or_handle()->set_port_id(port_id);
        spec->mutable_meta()->set_tenant_id(tenant_id);
        spec->set_port_speed(speed);
        spec->set_admin_state(admin_state);

        status = port_stub_->PortUpdate(&context, req_msg, &rsp_msg);
        if (status.ok()) {
            if (port_handle_api_status(
                    rsp_msg.response(0).api_status(), port_id) == true) {
                std::cout << "Port update succeeded for port "
                          << port_id
                          << std::endl;
            }

            return 0;
        }

        std::cout << "Port update failed for port "
                  << port_id
                  << " , error = "
                  << rsp_msg.response(0).api_status()
                  << std::endl;
        return -1;
    }

    int port_get(uint32_t tenant_id, uint32_t port_id) {
        PortGetRequest      *req;
        PortGetRequestMsg   req_msg;
        PortGetResponseMsg  rsp_msg;
        ClientContext       context;
        Status              status;

        req = req_msg.add_request();
        req->mutable_key_or_handle()->set_port_id(port_id);
        req->mutable_meta()->set_tenant_id(tenant_id);

        // port get
        status = port_stub_->PortGet(&context, req_msg, &rsp_msg);
        if (status.ok()) {
            if (port_handle_api_status(
                    rsp_msg.response(0).api_status(), port_id) == true) {
                std::cout << "Port Get succeeded for port "
                          << port_id << std::endl
                          << " Port oper status: "
                          << rsp_msg.response(0).status() << std::endl
                          << " Port type: "
                          << rsp_msg.response(0).spec().port_type() << std::endl
                          << " Admin state: "
                          << rsp_msg.response(0).spec().admin_state() << std::endl
                          << " Port speed: "
                          << rsp_msg.response(0).spec().port_speed() << std::endl
                          << " MAC ID: "
                          << rsp_msg.response(0).spec().mac_id() << std::endl
                          << " MAC channel: "
                          << rsp_msg.response(0).spec().mac_ch() << std::endl
                          << " Num lanes: "
                          << rsp_msg.response(0).spec().num_lanes() << std::endl;
            }

            return 0;
        }

        std::cout << "Port Get failed for port "
                  << port_id
                  << " , error = "
                  << rsp_msg.response(0).api_status()
                  << std::endl;
        return -1;
    }

    int port_delete(uint32_t tenant_id, uint32_t port_id) {
        PortDeleteRequest      *req;
        PortDeleteRequestMsg   req_msg;
        PortDeleteResponseMsg  rsp_msg;
        ClientContext          context;
        Status                 status;

        req = req_msg.add_request();
        req->mutable_key_or_handle()->set_port_id(port_id);
        req->mutable_meta()->set_tenant_id(tenant_id);

        // port get
        status = port_stub_->PortDelete(&context, req_msg, &rsp_msg);
        if (status.ok()) {
            if (port_handle_api_status(
                    rsp_msg.api_status(0), port_id) == true) {
                std::cout << "Port Delete succeeded for port "
                          << port_id << std::endl;
            }

            return 0;
        }

        std::cout << "Port Delete failed for port"
                  << port_id
                  << " , error = "
                  << rsp_msg.api_status(0)
                  << std::endl;
        return -1;
    }

    uint64_t tenant_create(uint32_t tenant_id) {
        TenantSpec           *spec;
        TenantRequestMsg     req_msg;
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

    uint64_t tenant_get_by_id(uint32_t id) {
        TenantGetRequestMsg     req_msg;
        TenantGetRequest        *req;
        TenantGetResponseMsg    rsp_msg;
        ClientContext           context;
        Status                  status;

        req = req_msg.add_request();
        req->mutable_key_or_handle()->set_tenant_id(id);
        status = tenant_stub_->TenantGet(&context, req_msg, &rsp_msg);
        if (status.ok()) {
            std::cout << "Tenant get succeeded, handle = "
                      << rsp_msg.response(0).status().tenant_handle()
                      << std::endl;
            return rsp_msg.response(0).status().tenant_handle();
        }
        std::cout << "Tenant get failed, error = "
                  << rsp_msg.response(0).api_status()
                  << std::endl;
        return 0;
    }

    uint64_t tenant_get_by_handle(uint64_t hal_handle) {
        TenantGetRequestMsg     req_msg;
        TenantGetRequest        *req;
        TenantGetResponseMsg    rsp_msg;
        ClientContext           context;
        Status                  status;

        req = req_msg.add_request();
        req->mutable_key_or_handle()->set_tenant_handle(hal_handle);
        status = tenant_stub_->TenantGet(&context, req_msg, &rsp_msg);
        if (status.ok()) {
            std::cout << "Tenant get succeeded, handle = "
                      << rsp_msg.response(0).status().tenant_handle()
                      << std::endl;
            return rsp_msg.response(0).status().tenant_handle();
        }
        std::cout << "Tenant get failed, error = "
                  << rsp_msg.response(0).api_status()
                  << std::endl;
        return 0;
    }

    void tenant_delete_by_id(uint32_t id) {
        TenantDeleteRequestMsg     req_msg;
        TenantDeleteRequest        *req;
        TenantDeleteResponseMsg    rsp_msg;
        ClientContext              context;
        Status                     status;

        req = req_msg.add_request();
        req->mutable_key_or_handle()->set_tenant_id(id);

        status = tenant_stub_->TenantDelete(&context, req_msg, &rsp_msg);
        if (status.ok()) {
            std::cout << "Tenant delete succeeded" << std::endl;
            return;
        } else {
            std::cout << "Tenant delete failed" << std::endl;
        }
        return;
    }

    void tenant_delete_by_handle(uint64_t hal_handle) {
        TenantDeleteRequestMsg     req_msg;
        TenantDeleteRequest        *req;
        TenantDeleteResponseMsg    rsp_msg;
        ClientContext              context;
        Status                     status;

        req = req_msg.add_request();
        req->mutable_key_or_handle()->set_tenant_handle(hal_handle);

        status = tenant_stub_->TenantDelete(&context, req_msg, &rsp_msg);
        if (status.ok()) {
            std::cout << "Tenant delete succeeded" << std::endl;
            return;
        }
        std::cout << "Tenant delete failed" << std::endl;
        return;
    }

    uint64_t l2segment_create(uint32_t tenant_id,
                              uint64_t l2segment_id,
                              uint64_t l4_profile_handle) {
        L2SegmentSpec           *spec;
        L2SegmentRequestMsg     req_msg;
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

    void event_test(void) {
        ClientContext context;
        std::shared_ptr<grpc::ClientReaderWriter<EventRequest, EventResponse> > stream(
            event_stub_->EventListen(&context));

        std::thread writer([stream]() {
            std::vector<EventRequest>    events(6);

            events[0].set_event_id(event::EVENT_ID_PORT);
            events[0].set_event_operation(event::EVENT_OP_UNSUBSCRIBE);
            events[1].set_event_id(event::EVENT_ID_ENDPOINT);
            events[1].set_event_operation(event::EVENT_OP_UNSUBSCRIBE);
            events[2].set_event_id(event::EVENT_ID_ENDPOINT);
            events[2].set_event_operation(event::EVENT_OP_SUBSCRIBE);
            events[3].set_event_id(event::EVENT_ID_PORT);
            events[3].set_event_operation(event::EVENT_OP_SUBSCRIBE);
            events[4].set_event_id(event::EVENT_ID_PORT);
            events[4].set_event_operation(event::EVENT_OP_UNSUBSCRIBE);
            events[5].set_event_id(event::EVENT_ID_ENDPOINT);
            events[5].set_event_operation(event::EVENT_OP_UNSUBSCRIBE);
            for (const EventRequest& event : events) {
                std::cout << "Subscribing to event " << event.event_id() << std::endl;
                stream->Write(event);
            }
            stream->WritesDone();
        });

        EventResponse event_response;
        while (stream->Read(&event_response)) {
            std::cout << "Got event " << event_response.event_id() << std::endl;
        }
        writer.join();
        Status status = stream->Finish();
        if (!status.ok()) {
            std::cout << "Event test failed" << std::endl;
        }
    }

private:
    std::unique_ptr<Tenant::Stub> tenant_stub_;
    std::unique_ptr<L2Segment::Stub> l2seg_stub_;
    std::unique_ptr<Port::Stub> port_stub_;
    std::unique_ptr<Event::Stub> event_stub_;
};

int port_enable(hal_client *hclient, int tenant_id, int port)
{
    std::cout <<  "*********** Port "
              << port
              << " enable and get"
              << " **********"
              << std::endl;

    hclient->port_update(
                     tenant_id, port,
                     ::port::PORT_SPEED_NONE, ::port::PORT_ADMIN_STATE_UP);

    hclient->port_get(tenant_id, port);

    return 0;
}

int ports_enable(hal_client *hclient, int tenant_id)
{
    int port = 0;

    for (port = 1; port <= 4; ++port) {
        port_enable (hclient, tenant_id, port);
    }

    return 0;
}

int port_get(hal_client *hclient, int tenant_id, int port)
{
    std::cout <<  "*********** Port "
              << port
              << " get"
              << " **********"
              << std::endl;

    hclient->port_get(tenant_id, port);

    return 0;
}

int ports_get(hal_client *hclient, int tenant_id)
{
    int port = 0;

    for (port = 1; port <= 4; ++port) {
        port_get (hclient, tenant_id, port);
    }

    return 0;
}

int port_test(hal_client *hclient, int tenant_id)
{
    int port = 1;

    // port 1: create and get
    std::cout <<  "*********** Port "
              << port
              << " create, enable and get"
              << " **********"
              << std::endl;
    hclient->port_create(tenant_id, port);
    hclient->port_get(tenant_id, port);

    // port 1: update speed and get
    std::cout <<  "*********** Port "
              << port
              << " update speed and get"
              << " **********"
              << std::endl;
    hclient->port_update(tenant_id, port,
                     ::port::PORT_SPEED_10G, ::port::PORT_ADMIN_STATE_NONE);
    hclient->port_get(tenant_id, port);

    // port 1: delete
    std::cout <<  "*********** Port "
              << port
              << " delete"
              << " **********"
              << std::endl;
    hclient->port_delete(tenant_id, port);

    port = 2;

    // port 2: create and get
    std::cout <<  "*********** Port "
              << port
              << " create, enable and get"
              << " **********"
              << std::endl;
    hclient->port_create(tenant_id, port);
    hclient->port_get(tenant_id, port);

    // port 2: update speed and get
    std::cout <<  "*********** Port "
              << port
              << " update speed and get"
              << " **********"
              << std::endl;
    hclient->port_update(tenant_id, port,
                     ::port::PORT_SPEED_10G, ::port::PORT_ADMIN_STATE_NONE);
    hclient->port_get(tenant_id, port);

    port = 1;

    // port 1: create and get
    std::cout <<  "*********** Port "
              << port
              << " create, enable and get"
              << " **********"
              << std::endl;
    hclient->port_create(tenant_id, port);
    hclient->port_get(tenant_id, port);

    // port 1: update speed and get
    std::cout <<  "*********** Port "
              << port
              << " update speed and get"
              << " **********"
              << std::endl;
    hclient->port_update(tenant_id, port,
                     ::port::PORT_SPEED_10G, ::port::PORT_ADMIN_STATE_NONE);
    hclient->port_get(tenant_id, port);

    // port 1: delete and get
    std::cout <<  "*********** Port "
              << port
              << " delete and get"
              << " **********"
              << std::endl;
    hclient->port_delete(tenant_id, port);
    hclient->port_get(tenant_id, port);

    port = 2;

    // port 2: disable and get
    std::cout <<  "*********** Port "
              << port
              << " disable and get"
              << " **********"
              << std::endl;
    hclient->port_update(tenant_id, port,
                     ::port::PORT_SPEED_NONE, ::port::PORT_ADMIN_STATE_DOWN);
    hclient->port_get(tenant_id, port);

    // port 2: create and get
    std::cout <<  "*********** Port "
              << port
              << " create, enable and get"
              << " **********"
              << std::endl;
    hclient->port_create(tenant_id, port);
    hclient->port_get(tenant_id, port);

    // port 2: delete and get
    std::cout <<  "*********** Port "
              << port
              << " delete and get"
              << " **********"
              << std::endl;
    hclient->port_delete(tenant_id, port);
    hclient->port_get(tenant_id, port);

    return 0;
}

// main test driver
int
main (int argc, char** argv)
{
    uint64_t    hal_handle;
    int         tenant_id = 1;
    hal_client hclient(grpc::CreateChannel(hal_svc_endpoint_,
                                           grpc::InsecureChannelCredentials()));

    //port_test(&hclient, tenant_id);
    //ports_enable(&hclient, tenant_id);
    ports_get(&hclient, tenant_id);

    // delete a non-existent tenant
    hclient.tenant_delete_by_id(1);

    // create a tenant and perform GETs
    hal_handle = hclient.tenant_create(1);
    assert(hal_handle != 0);
    assert(hclient.tenant_get_by_handle(hal_handle) != 0);
    assert(hclient.tenant_get_by_id(1) != 0);
    hclient.tenant_delete_by_handle(hal_handle);
    assert(hclient.tenant_get_by_id(1) == 0);   // should fail

    // recreate the tenant
    hal_handle = hclient.tenant_create(1);
    assert(hal_handle != 0);

    // create L2 segment
    assert(hclient.l2segment_create(1, 1, 1) != 0);
    hclient.tenant_delete_by_id(1);

    // subscribe and listen to HAL events
    hclient.event_test();
    return 0;
}
