#include <stdint.h>
class ipc {
public:
    // static methods to setup, teardown and instantiate
    static int setup_shm(int, int);
    static void tear_down_shm(void);
    static ipc *factory(void);

    uint8_t *get_buffer(int);
    int put_buffer(uint8_t*, int);

private:
    // private methods and state of ipc
    ipc(void) {};
    int get_avail_size(uint32_t);
    void inc_write_index();
    uint32_t *control_out_;
    uint32_t *control_in_;
    volatile uint32_t *write_index_;
    volatile uint32_t *read_index_;
    uint8_t *base_addr_;
    uint32_t size_;
    int num_buffers_;
    volatile uint64_t *put_count_;
    volatile uint64_t *err_count_;

    static int shm_fd_;
    static void *mmap_addr_;
    static int mmap_size_;
    static int inst_count_;
    static int max_inst_;
};
