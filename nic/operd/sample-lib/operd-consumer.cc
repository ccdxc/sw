#include "lib/operd/operd.hpp"

extern "C" {

void
handler(sdk::operd::log_ptr entry)
{
    printf("%s\n", entry->data());
}

}
