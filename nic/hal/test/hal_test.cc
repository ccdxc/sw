#include <iostream>
#include <thread>
#include <grpc++/grpc++.h>
#include "nic/gen/proto/hal/types.grpc.pb.h"
#include "nic/gen/proto/hal/vrf.grpc.pb.h"
#include "nic/gen/proto/hal/l2segment.grpc.pb.h"
#include "nic/gen/proto/hal/port.grpc.pb.h"
#include "nic/gen/proto/hal/event.grpc.pb.h"
#include "nic/gen/proto/hal/system.grpc.pb.h"
#include "nic/gen/proto/hal/debug.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using vrf::Vrf;
using vrf::VrfSpec;
using vrf::VrfRequestMsg;
using vrf::VrfResponse;
using vrf::VrfResponseMsg;
using vrf::VrfGetRequestMsg;
using vrf::VrfGetRequest;
using vrf::VrfGetResponseMsg;
using vrf::VrfDeleteRequestMsg;
using vrf::VrfDeleteRequest;
using vrf::VrfDeleteResponseMsg;

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
using port::PortDeleteResponse;

using sys::System;
using sys::SystemResponse;
using sys::Stats;
using sys::DropStats;
using sys::DropStatsEntry;
using sys::TableStats;
using sys::TableStatsEntry;
using types::Empty;

using debug::Debug;
using debug::SlabGetRequest;
using debug::SlabGetRequestMsg;
using debug::SlabGetResponse;
using debug::SlabGetResponseMsg;
using debug::SlabSpec;
using debug::SlabStats;

using event::Event;
using event::EventRequest;
using event::EventResponse;

const std::string&    hal_svc_endpoint_("localhost:50054");

class hal_client {
public:
    hal_client(std::shared_ptr<Channel> channel) : vrf_stub_(Vrf::NewStub(channel)),
    l2seg_stub_(L2Segment::NewStub(channel)), port_stub_(Port::NewStub(channel)),
	event_stub_(Event::NewStub(channel)), system_stub_(System::NewStub(channel)),
    debug_stub_(Debug::NewStub(channel)) {}

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

    int port_create(uint32_t vrf_id, uint32_t port_id) {
        PortSpec            *spec;
        PortRequestMsg      req_msg;
        PortResponseMsg     rsp_msg;
        ClientContext       context;
        Status               status;

        spec = req_msg.add_request();
        spec->mutable_key_or_handle()->set_port_id(port_id);
        spec->mutable_meta()->set_vrf_id(vrf_id);
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

    int port_update(uint32_t vrf_id,
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
        spec->mutable_meta()->set_vrf_id(vrf_id);
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

    int port_get(uint32_t vrf_id, uint32_t port_id) {
        PortGetRequest      *req;
        PortGetRequestMsg   req_msg;
        PortGetResponseMsg  rsp_msg;
        ClientContext       context;
        Status              status;

        req = req_msg.add_request();
        req->mutable_key_or_handle()->set_port_id(port_id);
        req->mutable_meta()->set_vrf_id(vrf_id);

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

    int port_delete(uint32_t vrf_id, uint32_t port_id) {
        PortDeleteRequest      *req;
        PortDeleteRequestMsg   req_msg;
        PortDeleteResponseMsg     rsp_msg;
        ClientContext          context;
        Status                 status;

        req = req_msg.add_request();
        req->mutable_key_or_handle()->set_port_id(port_id);
        req->mutable_meta()->set_vrf_id(vrf_id);

        // port get
        status = port_stub_->PortDelete(&context, req_msg, &rsp_msg);
        if (status.ok()) {
            if (port_handle_api_status(
                    rsp_msg.response(0).api_status(), port_id) == true) {
                std::cout << "Port Delete succeeded for port "
                          << port_id << std::endl;
            }

            return 0;
        }

        std::cout << "Port Delete failed for port"
                  << port_id
                  << " , error = "
                  << rsp_msg.response(0).api_status()
                  << std::endl;
        return -1;
    }

    int slab_get() {
       	ClientContext           context;
        SlabGetRequest          *req;
        SlabGetRequestMsg       req_msg;
        SlabGetResponse         rsp;
        SlabGetResponseMsg      rsp_msg;
        SlabSpec                spec;
        SlabStats               stats;
        Status                  status;
        int                     rsp_count, i;

        req = req_msg.add_request();
        req->set_id(5);

        std::cout << "Slab Get\n\n";
        status = debug_stub_->SlabGet(&context, req_msg, &rsp_msg);

        if (status.ok()) {
            rsp_count = rsp_msg.response_size();
            for (i = 0; i < rsp_count; i ++) {
                rsp = rsp_msg.response(i);
                if ((rsp.api_status() != types::API_STATUS_OK) || !rsp.has_spec() || !rsp.has_stats()) {
                    continue;
                }
                spec = rsp.spec();
                std::cout << "Slab Name" << spec.name() << "\n";

                stats = rsp.stats();
                std::cout << "Num elements in use: " << stats.num_elements_in_use() << "\n";
                std::cout << "Num allocs: " << stats.num_allocs() << "\n";
                std::cout << "Num frees: " << stats.num_frees() << "\n";
                std::cout << "Num alloc errors: " << stats.num_alloc_errors() << "\n";
                std::cout << "Num blocks: " << stats.num_blocks() << "\n\n";
            }
        }

        return 0;
    }

    int system_get() {
       	ClientContext       context;
       	Empty               request;
       	SystemResponse      response;
        Status              status;
       	Stats               stats;
       	DropStats           drop_stats;
       	DropStatsEntry      drop_entry;
       	TableStats          table_stats;
       	TableStatsEntry     table_entry;
       	int                 count;

       	std::cout << "System Get\n";
       	status = system_stub_->SystemGet(&context, request, &response);
       	if (status.ok() && (response.api_status() == types::API_STATUS_OK)) {
           	// Get Statistics
           	Stats stats = response.stats();

           	// First print drop stats

           	// Get DropStats
           	DropStats drop_stats = stats.drop_stats();

           	// Get count of DropEntries
           	count = drop_stats.drop_entries_size();

           	std::cout << "\nDrop Statistics:\n";

           	for (int i = 0; i < count; i ++) {
               	DropStatsEntry drop_entry = drop_stats.drop_entries(i);
               	std::cout << "Stats " << i
                    << ": " << drop_entry.drop_count() << "\n";
           	}

           	// Print table stats

           	// Get TableStats
           	TableStats table_stats = stats.table_stats();

           	// Get count of TableStats
           	count = table_stats.table_stats_size();

           	std::cout << "\nTable Statistics:\n";

           	for (int i = 0; i < count; i ++) {
               	TableStatsEntry table_entry = table_stats.table_stats(i);
               	std::cout << "Table type: " << table_entry.table_type() << "\n"
                    << "Table name: " << table_entry.table_name() << "\n"
                    << "Table size: " << table_entry.table_size() << "\n"
                    << "Overflow size: " << table_entry.overflow_table_size() << "\n"
                    << "Entries in use: " << table_entry.entries_in_use() << "\n"
                    << "Overflow entries in use: " << table_entry.overflow_entries_in_use() << "\n"
                    << "Num inserts: " << table_entry.num_inserts() << "\n"
                    << "Num insert errors: " << table_entry.num_insert_errors() << "\n"
                    << "Num deletes: " << table_entry.num_deletes() << "\n"
                    << "Num delete errors: " << table_entry.num_delete_errors() << "\n\n";
           	}
       	}

       	return 0;
   	}

    uint64_t vrf_create(uint32_t vrf_id) {
        VrfSpec           *spec;
        VrfRequestMsg     req_msg;
        VrfResponseMsg    rsp_msg;
        ClientContext        context;
        Status               status;

        spec = req_msg.add_request();
        spec->mutable_key_or_handle()->set_vrf_id(vrf_id);

        status = vrf_stub_->VrfCreate(&context, req_msg, &rsp_msg);
        if (status.ok()) {
            assert(rsp_msg.response(0).api_status() == types::API_STATUS_OK);
            std::cout << "Vrf create succeeded, handle = "
                      << rsp_msg.response(0).vrf_status().vrf_handle()
                      << std::endl;
            return rsp_msg.response(0).vrf_status().vrf_handle();
        }
        std::cout << "Vrf create failed, error = "
                  << rsp_msg.response(0).api_status()
                  << std::endl;
        return 0;
    }

    uint64_t vrf_get_by_id(uint32_t id) {
        VrfGetRequestMsg     req_msg;
        VrfGetRequest        *req;
        VrfGetResponseMsg    rsp_msg;
        ClientContext           context;
        Status                  status;

        req = req_msg.add_request();
        req->mutable_key_or_handle()->set_vrf_id(id);
        status = vrf_stub_->VrfGet(&context, req_msg, &rsp_msg);
        if (status.ok()) {
            std::cout << "Vrf get succeeded, handle = "
                      << rsp_msg.response(0).status().vrf_handle()
                      << std::endl;
            return rsp_msg.response(0).status().vrf_handle();
        }
        std::cout << "Vrf get failed, error = "
                  << rsp_msg.response(0).api_status()
                  << std::endl;
        return 0;
    }

    uint64_t vrf_get_by_handle(uint64_t hal_handle) {
        VrfGetRequestMsg     req_msg;
        VrfGetRequest        *req;
        VrfGetResponseMsg    rsp_msg;
        ClientContext           context;
        Status                  status;

        req = req_msg.add_request();
        req->mutable_key_or_handle()->set_vrf_handle(hal_handle);
        status = vrf_stub_->VrfGet(&context, req_msg, &rsp_msg);
        if (status.ok()) {
            std::cout << "Vrf get succeeded, handle = "
                      << rsp_msg.response(0).status().vrf_handle()
                      << std::endl;
            return rsp_msg.response(0).status().vrf_handle();
        }
        std::cout << "Vrf get failed, error = "
                  << rsp_msg.response(0).api_status()
                  << std::endl;
        return 0;
    }

    void vrf_delete_by_id(uint32_t id) {
        VrfDeleteRequestMsg     req_msg;
        VrfDeleteRequest        *req;
        VrfDeleteResponseMsg    rsp_msg;
        ClientContext              context;
        Status                     status;

        req = req_msg.add_request();
        req->mutable_key_or_handle()->set_vrf_id(id);

        status = vrf_stub_->VrfDelete(&context, req_msg, &rsp_msg);
        if (status.ok()) {
            std::cout << "Vrf delete succeeded" << std::endl;
            return;
        } else {
            std::cout << "Vrf delete failed" << std::endl;
        }
        return;
    }

    void vrf_delete_by_handle(uint64_t hal_handle) {
        VrfDeleteRequestMsg     req_msg;
        VrfDeleteRequest        *req;
        VrfDeleteResponseMsg    rsp_msg;
        ClientContext              context;
        Status                     status;

        req = req_msg.add_request();
        req->mutable_key_or_handle()->set_vrf_handle(hal_handle);

        status = vrf_stub_->VrfDelete(&context, req_msg, &rsp_msg);
        if (status.ok()) {
            std::cout << "Vrf delete succeeded" << std::endl;
            return;
        }
        std::cout << "Vrf delete failed" << std::endl;
        return;
    }

    uint64_t l2segment_create(uint32_t vrf_id,
                              uint64_t l2segment_id,
                              uint64_t l4_profile_handle) {
        L2SegmentSpec           *spec;
        L2SegmentRequestMsg     req_msg;
        L2SegmentResponseMsg    rsp_msg;
        ClientContext           context;
        Status                  status;

        spec = req_msg.add_request();
        spec->mutable_meta()->set_vrf_id(vrf_id);
        spec->mutable_key_or_handle()->set_segment_id(l2segment_id);
        spec->mutable_wire_encap()->set_encap_type(types::ENCAP_TYPE_DOT1Q);
        spec->mutable_wire_encap()->set_encap_value(100);

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
    std::unique_ptr<Vrf::Stub> vrf_stub_;
    std::unique_ptr<L2Segment::Stub> l2seg_stub_;
    std::unique_ptr<Port::Stub> port_stub_;
    std::unique_ptr<Event::Stub> event_stub_;
	std::unique_ptr<System::Stub> system_stub_;
	std::unique_ptr<Debug::Stub> debug_stub_;
};

int port_enable(hal_client *hclient, int vrf_id, int port)
{
    std::cout <<  "*********** Port "
              << port
              << " enable and get"
              << " **********"
              << std::endl;

    hclient->port_update(
                     vrf_id, port,
                     ::port::PORT_SPEED_NONE, ::port::PORT_ADMIN_STATE_UP);

    hclient->port_get(vrf_id, port);

    return 0;
}

int ports_enable(hal_client *hclient, int vrf_id)
{
    int port = 0;

    for (port = 1; port <= 8; ++port) {
        port_enable (hclient, vrf_id, port);
    }

    return 0;
}

int port_get(hal_client *hclient, int vrf_id, int port)
{
    std::cout <<  "*********** Port "
              << port
              << " get"
              << " **********"
              << std::endl;

    hclient->port_get(vrf_id, port);

    return 0;
}

int ports_get(hal_client *hclient, int vrf_id)
{
    int port = 0;

    for (port = 1; port <= 8; ++port) {
        port_get (hclient, vrf_id, port);
    }

    return 0;
}

int port_test(hal_client *hclient, int vrf_id)
{
    int port = 1;

    // port 1: create and get
    std::cout <<  "*********** Port "
              << port
              << " create, enable and get"
              << " **********"
              << std::endl;
    hclient->port_create(vrf_id, port);
    hclient->port_get(vrf_id, port);

    // port 1: update speed and get
    std::cout <<  "*********** Port "
              << port
              << " update speed and get"
              << " **********"
              << std::endl;
    hclient->port_update(vrf_id, port,
                     ::port::PORT_SPEED_10G, ::port::PORT_ADMIN_STATE_NONE);
    hclient->port_get(vrf_id, port);

    // port 1: delete
    std::cout <<  "*********** Port "
              << port
              << " delete"
              << " **********"
              << std::endl;
    hclient->port_delete(vrf_id, port);

    port = 2;

    // port 2: create and get
    std::cout <<  "*********** Port "
              << port
              << " create, enable and get"
              << " **********"
              << std::endl;
    hclient->port_create(vrf_id, port);
    hclient->port_get(vrf_id, port);

    // port 2: update speed and get
    std::cout <<  "*********** Port "
              << port
              << " update speed and get"
              << " **********"
              << std::endl;
    hclient->port_update(vrf_id, port,
                     ::port::PORT_SPEED_10G, ::port::PORT_ADMIN_STATE_NONE);
    hclient->port_get(vrf_id, port);

    port = 1;

    // port 1: create and get
    std::cout <<  "*********** Port "
              << port
              << " create, enable and get"
              << " **********"
              << std::endl;
    hclient->port_create(vrf_id, port);
    hclient->port_get(vrf_id, port);

    // port 1: update speed and get
    std::cout <<  "*********** Port "
              << port
              << " update speed and get"
              << " **********"
              << std::endl;
    hclient->port_update(vrf_id, port,
                     ::port::PORT_SPEED_10G, ::port::PORT_ADMIN_STATE_NONE);
    hclient->port_get(vrf_id, port);

    // port 1: delete and get
    std::cout <<  "*********** Port "
              << port
              << " delete and get"
              << " **********"
              << std::endl;
    hclient->port_delete(vrf_id, port);
    hclient->port_get(vrf_id, port);

    port = 2;

    // port 2: disable and get
    std::cout <<  "*********** Port "
              << port
              << " disable and get"
              << " **********"
              << std::endl;
    hclient->port_update(vrf_id, port,
                     ::port::PORT_SPEED_NONE, ::port::PORT_ADMIN_STATE_DOWN);
    hclient->port_get(vrf_id, port);

    // port 2: create and get
    std::cout <<  "*********** Port "
              << port
              << " create, enable and get"
              << " **********"
              << std::endl;
    hclient->port_create(vrf_id, port);
    hclient->port_get(vrf_id, port);

    // port 2: delete and get
    std::cout <<  "*********** Port "
              << port
              << " delete and get"
              << " **********"
              << std::endl;
    hclient->port_delete(vrf_id, port);
    hclient->port_get(vrf_id, port);

    return 0;
}

// main test driver
int
main (int argc, char** argv)
{
#if 0
    uint64_t    hal_handle;
    int         vrf_id = 1;
#endif
    hal_client hclient(grpc::CreateChannel(hal_svc_endpoint_,
                                           grpc::InsecureChannelCredentials()));

#if 0
    //port_test(&hclient, vrf_id);
    //ports_enable(&hclient, vrf_id);
    ports_get(&hclient, vrf_id);
    return 0;

    // delete a non-existent vrf
    hclient.vrf_delete_by_id(1);

    // create a vrf and perform GETs
    hal_handle = hclient.vrf_create(1);
    assert(hal_handle != 0);
    assert(hclient.vrf_get_by_handle(hal_handle) != 0);
    assert(hclient.vrf_get_by_id(1) != 0);
    hclient.vrf_delete_by_handle(hal_handle);
    assert(hclient.vrf_get_by_id(1) == 0);   // should fail

    // recreate the vrf
    hal_handle = hclient.vrf_create(1);
    assert(hal_handle != 0);

    // create L2 segment
    assert(hclient.l2segment_create(1, 1, 1) != 0);
    hclient.vrf_delete_by_id(1);

    // Get system statistics
    hclient.system_get();
#endif

    // Get slab statistics
    hclient.slab_get();

    // subscribe and listen to HAL events
    hclient.event_test();
    return 0;
}
