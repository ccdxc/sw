/*
 * See README.md
 */
const int IPC_TOTAL_WRITES = 2*4;
const int IPC_WRITE_OFFSET = 6*4;
const int IPC_READ_OFFSET  = 7*4;
const int IPC_OVH_SIZE     = 32;     // metadata (read, write indices, err count, put count, etc.)
const int IPC_HDR_SIZE     = 4;      // header on each buffer (stores the size of the message on each buffer)