/* model.i */
%module model
%include "carrays.i"
%include "cmalloc.i"
%include "cpointer.i"
%include "std_vector.i"
%{
    #include <thread>
    #include "lib_model_client.h"
    namespace hal {
        thread_local std::thread *t_curr_thread;
    }
%}

#define __attribute__(x)
typedef int p4pd_error_t;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;
%template(vector_uint8_t) std::vector<unsigned char>;
%pointer_functions(uint8_t, uint8_ptr_t);
%pointer_functions(uint16_t, uint16_ptr_t);
%pointer_functions(uint32_t, uint32_ptr_t);
%pointer_functions(uint64_t, uint64_ptr_t);
%array_functions(uint8_t, uint8_array_t);
%array_functions(uint16_t, uint16_array_t);
%array_functions(uint32_t, uint32_array_t);
%array_functions(uint64_t, uint64_array_t);
%malloc(uint8_t);
%malloc(uint16_t);
%malloc(uint32_t);
%malloc(uint64_t);
%free(uint8_t);
%free(uint16_t);
%free(uint32_t);
%free(uint64_t);
%include "lib_model_client.h"
%inline %{
    int model_cli_init()
    {
        lib_model_connect();
        return 0;
    }

    void iris_cli_cleanup()
    {
        lib_model_conn_close();
    }
%}
