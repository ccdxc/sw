#ifndef SNORT_API_H
#define SNORT_API_H

#include <string>

#define SO_PUBLIC  __attribute__ ((visibility("default")))

#define NUM_APP_IDS 4
struct App
{
	int appids[NUM_APP_IDS];
	int app_detection_status;
	int error;
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
SO_PUBLIC struct App run_pkt_thread_loop(int thread_num, void* pkt, uint32_t pkt_len, void *priv_ptr=NULL);
// Cleans up the thread.
// thread_num is a zero-based number
SO_PUBLIC int cleanup_pkt_thread(int thread_num);

// Placeholder for flow lifecycle mgmt
SO_PUBLIC int cleanup_flow(int flow_id);


#endif
