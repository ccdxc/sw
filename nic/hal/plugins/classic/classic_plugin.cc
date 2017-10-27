#include "nic/hal/plugins/classic/classic_plugin.hpp"

namespace hal {
namespace classic {

void init() {
    fte::register_feature(fte::FTE_FEATURE_CLASSIC_FWDING, "classic.fwding", fwding_exec);
}

}
}
