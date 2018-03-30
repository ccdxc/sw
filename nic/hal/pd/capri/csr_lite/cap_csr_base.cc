#include "nic/hal/pd/capri/csr_lite/include/cap_csr_base.hpp"
#include "nic/asic/capri/model/cap_top/cap_top_csr_defines.h"
#include "sdk/pal.hpp"

cap_sw_csr_base::cap_sw_csr_base()=default;
cap_sw_csr_base::~cap_sw_csr_base()=default;

void cap_sw_csr_base::set_offset(uint32_t _offset) {
    base__offset = _offset;
}

uint32_t cap_sw_csr_base::get_offset() const {
    return base__offset;
}

void cap_sw_csr_base::set_chip_id(uint32_t _chip_id) {
    base__chip_id = _chip_id;
}

uint32_t cap_sw_csr_base::get_chip_id() const {
    return base__chip_id;
}

cap_sw_csr_base::csr_type_t cap_sw_csr_base::get_csr_type() const {
    return base__csr_type;
}

void cap_sw_csr_base::set_csr_type(csr_type_t _type) {
    base__csr_type = _type;
}

uint32_t cap_sw_csr_base::get_width() const {
    return 0;
}

void cap_sw_csr_base::init(uint32_t _chip_id, uint32_t _addr_base) {
    set_chip_id(_chip_id);
    set_offset(_addr_base);

    if(get_csr_type() == CSR_TYPE_NONE) {set_csr_type(CSR_TYPE_REGISTER);}
    base__csr_end_addr = (get_width() != 0) ? (get_offset() + ((get_width() +7)/8) -1) : get_offset();
}

void cap_sw_csr_base::pack(uint8_t *bytes, uint32_t start) {
    SDK_TRACE_ERR("cap_sw_csr_base::pack() should not be used\n");
}

void cap_sw_csr_base::unpack(uint8_t *bytes, uint32_t start) {
    SDK_TRACE_ERR("cap_sw_csr_base::unpack() should not be used\n");
}

uint32_t cap_sw_csr_base::get_byte_size() const {
    int x;
    uint32_t my_width = get_width();
    if(my_width < 32) my_width = 32;
    x  = (int) my_width;
    x |= (x >> 1);
    x |= (x >> 2);
    x |= (x >> 4);
    x |= (x >> 8);
    x |= (x >> 16);
    x  = (x & ~(x >> 1));    
    if ((uint32_t)x != my_width) {
        x <<= 1;
    }
    return (x/8);
}

void cap_sw_csr_base::write() {
    SDK_TRACE_ERR("cap_sw_csr_base::write() should not be used\n");
}

void cap_sw_csr_base::read() {
    SDK_TRACE_ERR("cap_sw_csr_base::read() should not be used\n");
}

void cap_sw_csr_base::write_hw(uint8_t *bytes, int block_write) {
    SDK_TRACE_ERR("cap_sw_csr_base::write_hw() should not be used\n");
}

void cap_sw_csr_base::read_hw(uint8_t *bytes, int block_read) {
    SDK_TRACE_ERR("cap_sw_csr_base::read_hw() should not be used\n");
}

void cap_sw_csr_base::read_compare(int block_read) {
    SDK_TRACE_ERR("cap_sw_csr_base::read_compare() should not be used\n");
}


cap_sw_register_base::cap_sw_register_base():
    cap_sw_csr_base() {
        set_csr_type(CSR_TYPE_REGISTER);
}

cap_sw_register_base::~cap_sw_register_base()=default;

void cap_sw_register_base::write() {
    uint32_t width = get_width();
    uint32_t words = (width+31)/32;
    uint32_t num_bytes = words*4;
    auto *bytes = (uint8_t *)calloc(num_bytes, sizeof(uint8_t));
    pack(bytes);
    write_hw(bytes);
    free(bytes);
}
void cap_sw_register_base::read() {
    uint32_t width = get_width();
    uint32_t words = (width+31)/32;
    uint32_t num_bytes = words*4;
    auto *bytes = (uint8_t *)calloc(num_bytes, sizeof(uint8_t));
    read_hw(bytes);
    unpack(bytes);
    free(bytes);
}

void cap_sw_register_base::write_hw(uint8_t *write_bytes, int block_write) {
    uint32_t words = (get_width()+31)/32;
    uint64_t offset = get_offset();
    
    SDK_ASSERT(block_write == 0);

    for(uint32_t ii = 0; ii < words; ii++) {
        uint32_t data = *((uint32_t *)&(write_bytes[ii*4]));
        SDK_TRACE_DEBUG("cap_sw_register_base::write_hw(): Addr: {}; Data: {}\n",
                         offset + (ii*4), data);
        sdk::lib::pal_reg_write(offset + (ii*4), &data);
        // cpu::access()->write(get_chip_id(), offset + (ii*4), data, false,
        //               secure_acc_e);
    }
}

void cap_sw_register_base::read_hw(uint8_t *read_bytes, int block_read) {
    uint64_t offset = get_offset();
    uint32_t width = get_width();
    uint32_t words = (width+31)/32;

    SDK_ASSERT(block_read == 0);

    for(uint32_t ii = 0; ii < words; ii++) {
        // uint32_t data = cpu::access()->read(chip_id, offset + (ii*4), false, secure_acc_e);
        uint32_t data = 0;
        sdk::lib::pal_reg_read(offset + (ii*4), &data);
        *((uint32_t *)&(read_bytes[ii*4])) = data;
        SDK_TRACE_DEBUG("cap_sw_register_base::read_hw(): Addr: {}; Data: {}\n",
                         offset + (ii*4), data);
    }
}

void cap_sw_register_base::read_compare(int block_read) {
    uint8_t *bytes1, *bytes2;
    uint32_t width = get_width();
    uint32_t words = (width+31)/32;
    uint32_t num_bytes = words*4;

    bytes1 = (uint8_t *)calloc(num_bytes, sizeof(uint8_t));
    bytes2 = (uint8_t *)calloc(num_bytes, sizeof(uint8_t));

    pack(bytes1);
    read_hw(bytes2, block_read);

    for (uint32_t ii=0; ii<num_bytes; ii++) {
        if (bytes1[ii] != bytes2[ii]) {
            SDK_TRACE_ERR("cap_sw_register_base::read_compare(): Actual: {}, Read: {}\n", bytes1[ii], bytes2[ii]);
        }
    }

    free(bytes1);
    free(bytes2);
}

cap_sw_memory_base::cap_sw_memory_base():
    cap_sw_csr_base() {
        set_csr_type(CSR_TYPE_MEMORY);
}

cap_sw_memory_base::~cap_sw_memory_base()=default;

void cap_sw_memory_base::write() {
    SDK_TRACE_ERR("cap_sw_memory_base::write() should not be used\n");
}

void cap_sw_memory_base::read() {
    SDK_TRACE_ERR("cap_sw_memory_base::read() should not be used\n");
}


cap_sw_decoder_base::cap_sw_decoder_base():
    cap_sw_csr_base() {
        set_csr_type(CSR_TYPE_DECODER);
}

cap_sw_decoder_base::~cap_sw_decoder_base()=default;

void cap_sw_decoder_base::write() {
    SDK_TRACE_ERR("cap_sw_decoder_base::write() should not be used\n");
}

void cap_sw_decoder_base::read() {
    SDK_TRACE_ERR("cap_sw_decoder_base::read() should not be used\n");
}


cap_sw_block_base::cap_sw_block_base():
    cap_sw_csr_base() {
        set_csr_type(CSR_TYPE_BLOCK);
    }

cap_sw_block_base::~cap_sw_block_base()=default;

void cap_sw_block_base::write() {
    SDK_TRACE_ERR("cap_sw_block_base::write() should not be used\n");
}

void cap_sw_block_base::read() {
    SDK_TRACE_ERR("cap_sw_block_base::read() should not be used\n");
}

void cap_sw_block_base::set_byte_size(uint32_t _byte_size) {
    block__byte_size = _byte_size;
}

uint32_t cap_sw_block_base::get_byte_size() const {
    return block__byte_size;
}

