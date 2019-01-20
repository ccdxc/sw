#ifndef __LINKMGR_PKNOBS_H__
#define __LINKMGR_PKNOBS_H__

#include <stdint.h>
#include <string>

class PKnobsReader {
public:
    static uint64_t evalKnob(const std::string & name);
};

#endif // __LINKMGR_PKNOBS_H__
