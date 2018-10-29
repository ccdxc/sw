#include "sdk/pal.hpp"
#include "sdk/thread.hpp"
#include "asicerrord.h"

extern struct asic_registers capri_registers[];

const char
*errortostring(etype errortype)
{
    switch (errortype) {
    case ERROR:
        return "Error";
    case INFO:
        return "Info";
    case FATAL:
        return "Fatal";
    default:
        return "Unknown";
    }
}

/**
 * The function is called when the data read is non-zero.
 * In this case find out which interrupt was trigerred and log it
 * */
void
isr(struct asic_registers *intreg, uint32_t data)
{
    if (intreg->name == NULL) {
        INFO("Interrupt register at address {} has value {} and type {}",
             intreg->regaddr, data, errortostring(intreg->errortype));
    }

    if (intreg->map != NULL) {
        for (int i = 0; i < intreg->ninterrupts; i++) {
            if (data & BIT(i)) {
                intreg->map[i].count++;
                if (intreg->name == NULL) {
                    INFO("type {} interrupt {}",
                         errortostring(intreg->errortype), intreg->map[i].name);
                }
                else {
                    if (intreg->map[i].count <= 10000)
                    {
                        INFO("type {} register name {} interrupt {} times {}",
                             errortostring(intreg->errortype), intreg->name,
                             intreg->map[i].name, intreg->map[i].count);
                    }
                }
            }
        }
    }
}

/**
 * Poll for interrupts. 
 * */
void
pollinterrupts()
{
    uint32_t size = 0;
    uint64_t addr = 0;
    uint32_t data = 0;
    sdk::lib::pal_ret_t rc = sdk::lib::PAL_RET_NOK;

    for (int i = 0; i < asic_registers_count; i++) {
        // find out how many words to read?
        if ((capri_registers[i].ninterrupts % 32) == 0){
            size = capri_registers[i].ninterrupts / 32;
        }
        else {
            size = capri_registers[i].ninterrupts / 32 + 1;
        }
        addr = capri_registers[i].regaddr;
        // check if the address if valid
        if (addr == 0) {
            continue;
        }
        // read the data
        rc = sdk::lib::pal_reg_read(addr, &data, size);
        if (rc == sdk::lib::PAL_RET_NOK) {
            continue;
        }
        else if (rc == sdk::lib::PAL_RET_OK && data != 0) {
            isr(&capri_registers[i], data);
            // rewrite the same data back to clear the interrupt
            rc = sdk::lib::pal_reg_write(addr, &data, size);
            if (rc == sdk::lib::PAL_RET_NOK) {
                INFO("clearing the interrupt {} failed", capri_registers[i].name);
            }
        }
    }
}

int
main(int argc, char *argv[])
{
    // initialize the pal
#ifdef __x86_64__
    assert(sdk::lib::pal_init(sdk::types::platform_type_t::PLATFORM_TYPE_SIM) == sdk::lib::PAL_RET_OK);
#elif __aarch64__
    assert(sdk::lib::pal_init(sdk::types::platform_type_t::PLATFORM_TYPE_HAPS) == sdk::lib::PAL_RET_OK);
#endif

    while (1) {
        // Dont block context switches, let the process sleep for some time
        sleep(10);
        // Poll for interrupts
        pollinterrupts();
    }
    return (SUCCESS);
}
