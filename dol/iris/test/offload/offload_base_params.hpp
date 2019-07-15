#ifndef _OFFLOAD_BASE_PARAMS_HPP_
#define _OFFLOAD_BASE_PARAMS_HPP_

/*
 * Base parameters
 */
class offload_base_params_t
{
public:

    offload_base_params_t() :
        destructor_free_buffers_(true),
        suppress_info_log_(false)
    {
    }

    offload_base_params_t&
    destructor_free_buffers(bool destructor_free_buffers)
    {
        destructor_free_buffers_ = destructor_free_buffers;
        return *this;
    }
    offload_base_params_t&
    suppress_info_log(bool suppress_info_log)
    {
        suppress_info_log_ = suppress_info_log;
        return *this;
    }

    bool destructor_free_buffers(void) { return destructor_free_buffers_; }
    bool suppress_info_log(void) { return suppress_info_log_; }

private:
    bool                        destructor_free_buffers_;
    bool                        suppress_info_log_;
};

#endif // _OFFLOAD_BASE_PARAMS_HPP_
