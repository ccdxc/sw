#include "nic/hal/plugins/cfg/nw/interface.hpp"
#include "gen/proto/interface.pb.h"
#include "nic/hal/hal.hpp"
#include "nic/hal/src/internal/proxy.hpp"
#include <gtest/gtest.h>
#include <stdio.h>
#include <stdlib.h>
#include "nic/hal/test/utils/hal_test_utils.hpp"
#include "nic/hal/test/utils/hal_base_test.hpp"
#include "gen/proto/event.grpc.pb.h"
#include "nic/hal/iris/include/hal_state.hpp"
#include "nic/hal/src/internal/event.hpp"

using event::Event;
using event::EventRequest;
using event::EventResponse;

class event_test : public hal_base_test {
protected:
  event_test() {
  }

  virtual ~event_test() {
  }

  // will be called immediately after the constructor before each test
  virtual void SetUp() {
  }

  // will be called immediately after each test before the destructor
  virtual void TearDown() {
  }

  // Will be called at the beginning of all test cases in this class
  static void SetUpTestCase() {
    hal_base_test::SetUpTestCase();
    hal_test_utils_slab_disable_delete();
  }

};

// ----------------------------------------------------------------------------
// Creating a event notification
// ----------------------------------------------------------------------------
TEST_F(event_test, notify_listeners)
{
     auto channel =
        grpc::CreateChannel("localhost:50054", grpc::InsecureChannelCredentials());
     auto stub = Event::NewStub(channel);
     grpc::ClientContext context;
     EventRequest req;

     req.set_event_id(event::EVENT_ID_PORT_STATE);
     req.set_event_operation(event::EVENT_OP_SUBSCRIBE);
     std::unique_ptr<grpc::ClientReader<EventResponse>> stream(stub->EventListen(&context, req));
     hal::handle_event_request(&req, (grpc::ServerWriter<EventResponse> *)stream.get());

     std::thread writer([]() {
         auto walk_cb =  [](uint32_t event_id, void *entry, void *ctxt) {
             grpc::ServerWriter<EventResponse> *stream =
                              (grpc::ServerWriter<EventResponse> *)ctxt;
             EventResponse   evtresponse;

             std::cout << "Listener available";
             evtresponse.set_event_id(event::EVENT_ID_PORT_STATE);
             stream->Write(evtresponse);
             return true;
         };

         std::cout << "Running this" << std::endl;
         hal::g_hal_state->event_mgr()->walk_listeners(event::EVENT_ID_PORT_STATE, (void *)1, walk_cb);
     });

     EventResponse event_response;
     while (stream->Read(&event_response)) {
        std::cout << "Got event " << event_response.event_id() << std::endl;
        break;
     }
     writer.join();
     grpc::Status status = stream->Finish();
     if (!status.ok()) {
         std::cout << "Event test failed: " << status.ok() << std::endl;
     }
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
