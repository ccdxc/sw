const int   SHM_SIZE      = 200 *1024;      // 204800 bytes; total size of the shared mem.
const int   SHM_BUF_SIZE  = 128;            // 128 byte sized buffers.
const char* SHM_DIR       = "/pen-events/"; // all the shared memory files will be create /dev/shm/pen-events/