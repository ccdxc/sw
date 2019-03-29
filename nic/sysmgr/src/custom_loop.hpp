#ifndef CUSTOM_LOOP
#define CUSTOM_LOOP

#include <memory>

class CustomLoopReactor {
public:
    virtual void do_work() = 0;
    virtual bool has_work() = 0;
};
typedef std::shared_ptr<CustomLoopReactor> CustomLoopReactorPtr;

extern void custom_loop_reactor_register(CustomLoopReactorPtr reactor);

#endif // CUSTOM_LOOP
