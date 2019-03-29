#include "custom_loop.hpp"

#include <list>
#include <memory>

#include "check_watcher.hpp"

class CustomLoop : public CheckReactor
{
private:
    std::list<CustomLoopReactorPtr> reactors;
    CheckWatcherPtr                 check_watcher;
public:
    static std::shared_ptr<CustomLoop> create();
    virtual void on_check();
    void register_reactor(CustomLoopReactorPtr reactor);
};
typedef std::shared_ptr<CustomLoop> CustomLoopPtr;

CustomLoopPtr CustomLoop::create()
{
    CustomLoopPtr loop = std::make_shared<CustomLoop>();

    loop->check_watcher = CheckWatcher::create(loop);
    return loop;
}

void CustomLoop::register_reactor(CustomLoopReactorPtr reactor)
{
    this->reactors.push_back(reactor);
}

void CustomLoop::on_check()
{
    std::list<CustomLoopReactorPtr>::iterator it;
    bool acted;
    do {
	acted = false;
	for (auto &reactor: this->reactors)
	{
	    if (reactor->has_work())
	    {
		reactor->do_work();
		acted = true;
	    }
	}
    } while (acted == true);
}

CustomLoopPtr default_custom_loop = CustomLoop::create();

void custom_loop_reactor_register(CustomLoopReactorPtr reactor)
{
    default_custom_loop->register_reactor(reactor);
}
