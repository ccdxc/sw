#ifndef CAP_CSR_BASE_LITE_H
#define CAP_CSR_BASE_LITE_H

#include <stdint.h>
#include <iostream>
#include <string.h>

namespace sdk {
namespace lib {
namespace csrlite {
using namespace std;

class cap_sw_csr_base {
    public:
        enum csr_type_t {
            CSR_TYPE_NONE = 0,
            CSR_TYPE_REGISTER,
            CSR_TYPE_MEM_ENTRY,
            CSR_TYPE_MEMORY,
            CSR_TYPE_DECODER,
            CSR_TYPE_BLOCK
        };

    protected:
        csr_type_t base__csr_type;
        uint32_t base__chip_id;
        uint32_t base__offset;

    public:
        cap_sw_csr_base();
        virtual ~cap_sw_csr_base();
        static uint32_t s_get_byte_size(uint32_t my_width);

        csr_type_t get_csr_type() const;
        uint32_t get_offset() const;
        uint32_t get_chip_id() const;
        void set_csr_type(csr_type_t _type);
        void set_offset(uint32_t _offset);
        void set_chip_id(uint32_t _chip_id);

        virtual void write();
        virtual void read();
        virtual void write_hw(uint8_t *bytes, int block_write=0);
        virtual void read_hw(uint8_t *bytes, int block_read=0);
        virtual void read_compare(int block_read=0);
        virtual uint32_t get_byte_size() const;

        virtual void init(uint32_t _chip_id, uint32_t _addr_base);
        virtual void pack(uint8_t *bytes, uint32_t start=0);
        virtual void unpack(uint8_t *bytes, uint32_t start=0);
        virtual uint32_t get_width() const;
};

class cap_sw_register_base : public cap_sw_csr_base {
    public:
        cap_sw_register_base();
        ~cap_sw_register_base() override;

        void write() final;
        void read() final;
        void write_hw(uint8_t *write_bytes, int block_write=0) final;
        void read_hw(uint8_t *write_bytes, int block_read=0) final;
        void read_compare(int block_read=0) final;
};

class cap_sw_memory_base : public cap_sw_csr_base {
    public:
        cap_sw_memory_base();
        ~cap_sw_memory_base() override;

        void write() final;
        void read() final;
};

class cap_sw_decoder_base : public cap_sw_csr_base {
    public:
        cap_sw_decoder_base();
        ~cap_sw_decoder_base() override;

        void write() final;
        void read() final;
};

class cap_sw_block_base : public cap_sw_csr_base {

    protected:
    uint32_t block__byte_size;

    public:
        cap_sw_block_base();
        ~cap_sw_block_base() override;

        void write() final;
        void read() final;

        virtual void set_byte_size(uint32_t _byte_size);
        uint32_t  get_byte_size() const final;
};

}
}
}
#endif // CAP_CSR_BASE_LITE_H
