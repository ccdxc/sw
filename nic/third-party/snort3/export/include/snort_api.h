#ifndef SNORT_API_H
#define SNORT_API_H

#include <string>

#define SO_PUBLIC  __attribute__ ((visibility("default")))

#define NUM_APP_IDS 4
struct SnortFlowInfo
{
    int appids[NUM_APP_IDS];
    int app_detection_status;
    int error;
    void* flow_handle;
};

// Does basic initializations of snort main thread
SO_PUBLIC int init_main_thread(int num_pkt_threads, std::string config_file_path, char* pcap_path=NULL);
// Core of the main thread. Should be repeatedly invoked for life of the thread
// A non-zero return status indicates that all packet threads have exited and
// cleanup_main_thread needs to be called
SO_PUBLIC int run_main_thread_loop();
// Cleans up the thread. Should only be called after all packet threads have cleaned up
SO_PUBLIC int cleanup_main_thread();

// Does basic initializations of snort packet thread
// thread_num is a zero-based number
// This function needs to be called in the context of a packet thread
SO_PUBLIC int init_pkt_thread(int thread_num);
// Core of the packet thread. Should be invoked whenever a packet needs to be inspected
// A non-zero return status indicates that packet thread is exiting and
// cleanup_pkt_thread needs to be called
// thread_num is a zero-based number
// Returns flow_handle to be used for lookup or cleanup
SO_PUBLIC int run_pkt_thread_loop(int thread_num, uint8_t* pkt, uint32_t pkt_len,
        void **flow_handle, void *priv_ptr=NULL);
// Returns info on the last flow processed by the thread
SO_PUBLIC int get_pkt_thread_flow_info(int thread_num, struct SnortFlowInfo* flow_info);
// Returns info on a specific flow
SO_PUBLIC int get_flow_info_by_handle(void* flow_handle, struct SnortFlowInfo* flow_info);
SO_PUBLIC int get_flow_info_by_key(uint8_t ip_proto, bool is_v6,
        const uint8_t* sip, const uint8_t* dip, uint16_t sport, uint16_t dport,
        uint16_t vlan_id, uint16_t tenant_id, struct SnortFlowInfo* flow_info);
// Cleans up the thread.
// thread_num is a zero-based number
SO_PUBLIC int cleanup_pkt_thread(int thread_num);

// Cleans up the flow
SO_PUBLIC int cleanup_flow(void* flow_handle);
SO_PUBLIC int cleanup_flow_by_key(uint8_t ip_proto, bool is_v6,
        const uint8_t* sip, const uint8_t* dip, uint16_t sport, uint16_t dport,
        uint16_t vlan_id, uint16_t tenant_id);


#endif
