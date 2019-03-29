#include "gtest/gtest.h"

#include "nic/delphi/utils/utest.hpp"
#include "nic/utils/penlog/lib/penlog.hpp"
#include "nic/utils/penlog/lib/null_logger.hpp"

#include "delphi_service.hpp"
#include "service_factory.hpp"
#include "service_watcher.hpp"
#include "utils.hpp"

void *startEventLoop(void* arg)
{
    ev::default_loop *loop = (ev::default_loop *)arg;
    logger->info("Event loop started!");
    loop->run(0);
    logger->info("Event loop exited!");

    return NULL;
}

class SysmgrTest : public ::testing::Test
{
public:
    pthread_t         ev_thread_id;
    ServiceFactoryPtr service_factory;
    delphi::SdkPtr    sdk;
    DelphiServicePtr  svc;
    ev::default_loop  loop;

    void SetUp() override {
	this->service_factory = ServiceFactory::getInstance();
	this->sdk = std::make_shared<delphi::Sdk>();
	this->svc = DelphiService::create(sdk);
	logger = penlog::logger_init(sdk, "sysmgr-test");

	pthread_create(&ev_thread_id, 0, &startEventLoop, (void*)&loop);
    };
 
    void TearDown() override {
        loop.break_loop(ev::ALL);
    };

    void PokeLoop() {
	loop.break_loop();
	usleep(10000);
    };
    
};

class ServiceLoopTest : public ServiceReactor
{
public:
    bool notification_received = false;
    void on_service_start(std::string name) {
	this->notification_received = true;
	logger->info("Got notification for {}", name);
    };
    void on_service_stop(std::string name) {
    };
    void on_service_heartbeat(std::string name) {
    };
};

TEST_F(SysmgrTest, ServiceLoop)
{
    std::shared_ptr<ServiceLoopTest> reactor = std::make_shared<ServiceLoopTest>();
    ServiceLoopPtr service_loop = ServiceLoop::getInstance();
    
    service_loop->register_event_reactor(SERVICE_EVENT_START,
	"sysmgr-test-service", reactor);

    for (auto &service_reactors: service_loop->event_reactors[SERVICE_EVENT_START])
    {
	logger->info("Service: {}", service_reactors.first);
	for (auto reactor: service_reactors.second)
	{
	    logger->info("  Reactor: {}", reactor);
	}
    }
    service_loop->queue_event(ServiceEvent::create("sysmgr-test-service",
	    SERVICE_EVENT_START));
    this->PokeLoop();
    ASSERT_EQ_EVENTUALLY(reactor->notification_received, true) <<
	"sysmgr-test-service start notifcation not received";
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
