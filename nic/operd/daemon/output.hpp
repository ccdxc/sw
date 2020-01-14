#ifndef __OPERD_DEAMON_OUTPUT_H__
#define __OPERD_DEAMON_OUTPUT_H__

#include <memory>

#include "lib/operd/operd.hpp"

class output {
public:
    virtual void handle(sdk::operd::log_ptr entry) = 0;
};
typedef std::shared_ptr<output> output_ptr;

#endif
