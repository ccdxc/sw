// --------------------------------------------------------------------------------
// Warning: This is an Auto-generated file.
// --------------------------------------------------------------------------------

// --------------------------------------------------------------------------------
// NORM_PROF HAL API related public data structures and APIs
// --------------------------------------------------------------------------------
#ifndef __NORM_PROF_H__
#define __NORM_PROF_H__

#include <base.h>
#include <eth.h>

// --------------------------------------------------------------------------------
// NORM_PROF key
// --------------------------------------------------------------------------------
typedef struct norm_prof_key_s { 
    uint32_t id;
} __PACK__ norm_prof_key_t; 

// --------------------------------------------------------------------------------
// NORM_PROF Valid bits for key/handle
// --------------------------------------------------------------------------------
typedef struct norm_prof_key_handle_s { 
    norm_prof_key_t key;
    hal_handle_t hndl;
    uint8_t __key_valid:1;
    uint8_t __hndl_valid:1;
} __PACK__ norm_prof_key_handle_t; 

// --------------------------------------------------------------------------------
// NORM_PROF Multiple Key Handles 
// --------------------------------------------------------------------------------
typedef struct norm_prof_key_handles_s { 
    uint32_t num_khs;
    norm_prof_key_handle_t khs[0];
} __PACK__ norm_prof_key_handles_t; 

// --------------------------------------------------------------------------------
// NORM_PROF data. Valid bits for each field 
// --------------------------------------------------------------------------------
typedef struct norm_prof_data_s { 
    uint8_t stateful_check_en:1;
    uint8_t __stateful_check_en_valid:1;

    uint8_t ip_normalization_en:1;
    uint8_t __ip_normalization_en_valid:1;

} __PACK__ norm_prof_data_t; 

// --------------------------------------------------------------------------------
// NORM_PROF operational state 
// --------------------------------------------------------------------------------
typedef struct norm_prof_oper_s { 
    hal_ret_t      reason_code;
} __PACK__ norm_prof_oper_t; 

// --------------------------------------------------------------------------------
// NORM_PROF Full Entry 
// --------------------------------------------------------------------------------
typedef struct norm_prof_s { 
    norm_prof_key_handle_t kh;
    norm_prof_data_t data;
    norm_prof_oper_t oper;
} __PACK__ norm_prof_t; 

// --------------------------------------------------------------------------------
// NORM_PROF Full Entries 
// --------------------------------------------------------------------------------
typedef struct norm_profs_s { 
    uint32_t num_entries;
    norm_prof_t entries[0];
} __PACK__ norm_profs_t; 

// --------------------------------------------------------------------------------
// NORM_PROF Filter structure
// --------------------------------------------------------------------------------
typedef struct norm_prof_filter_s { 
    uint32_t id;
    uint8_t __id_valid:1;

    uint8_t stateful_check_en:1;
    uint8_t __stateful_check_en_valid:1;

    uint8_t ip_normalization_en:1;
    uint8_t __ip_normalization_en_valid:1;

} __PACK__ norm_prof_filter_t; 

// --------------------------------------------------------------------------------
// NORM_PROF CRUD APIs
// --------------------------------------------------------------------------------
hal_ret_t hal_create_norm_prof(norm_profs_t *norm_profs __INOUT__);
hal_ret_t hal_read_norm_prof(norm_prof_key_handles_t *khs __IN__,
    norm_prof_filter_t *filter __IN__,
    norm_profs_t **norm_profs __OUT__);
hal_ret_t hal_update_norm_prof(norm_profs_t *norm_profs __IN__);
hal_ret_t hal_update_norm_prof_by_key(norm_prof_key_handles_t *khs,
    norm_prof_data_t *data __IN__);
hal_ret_t hal_delete_norm_prof(norm_prof_key_handles_t *khs __IN__);

// --------------------------------------------------------------------------------
// NORM_PROF Key Handle Init
// --------------------------------------------------------------------------------
static inline void
hal_init_norm_prof_key_handle (norm_prof_key_handle_t *kh __INOUT__)
{
    kh->__key_valid = FALSE;
    kh->__hndl_valid = FALSE;
}

// --------------------------------------------------------------------------------
// NORM_PROF Key Handles Init
// --------------------------------------------------------------------------------
static inline void
hal_init_norm_prof_key_handles (norm_prof_key_handles_t *kh __INOUT__)
{
    khs->num_khs = 0;
}

// --------------------------------------------------------------------------------
// NORM_PROF Data Fields Init
// --------------------------------------------------------------------------------
static inline void
hal_init_norm_prof_data (norm_prof_data_t *data __INOUT__)
{
    data->__stateful_check_en_valid = FALSE;
    data->__ip_normalization_en_valid = FALSE;
}

// --------------------------------------------------------------------------------
// NORM_PROF Filter Fields Init
// --------------------------------------------------------------------------------
static inline void
hal_init_norm_prof_filter (norm_prof_filter_t *filter __INOUT__)
{
    filter->__id_valid = FALSE;
    filter->__stateful_check_en_valid = FALSE;
    filter->__ip_normalization_en_valid = FALSE;
}

// --------------------------------------------------------------------------------
// NORM_PROF Full Entries Init
// --------------------------------------------------------------------------------
static inline void
hal_init_norm_profs (norm_profs_t *entries __INOUT__)
{
    entries->num_entries = 0;
}

// --------------------------------------------------------------------------------
// NORM_PROF Setting Key
// --------------------------------------------------------------------------------
static inline void
hal_set_norm_prof_key_handle_key (norm_prof_key_handle_t *kh __INOUT__,
    uint32_t id __IN__)
{
    kh->key.id = id;
    kh->__key_valid = TRUE;
}

// --------------------------------------------------------------------------------
// NORM_PROF Setting Key Handle
// --------------------------------------------------------------------------------
static inline void
hal_set_norm_prof_key_handle_handle (norm_prof_key_handle_t *kh __INOUT__,
    hal_handle_t hndl __IN__)
{
    kh->hndl = hndl;
    kh->__hndl_valid = TRUE;
}

// --------------------------------------------------------------------------------
// NORM_PROF Setting Data Field stateful_check_en
// --------------------------------------------------------------------------------
static inline void
hal_set_norm_prof_data_stateful_check_en (norm_prof_data_t *data __INOUT__,
    uint8_t stateful_check_en __IN__)
{
    data->stateful_check_en = stateful_check_en;
    data->__stateful_check_en_valid = TRUE;
}

// --------------------------------------------------------------------------------
// NORM_PROF Setting Data Field ip_normalization_en
// --------------------------------------------------------------------------------
static inline void
hal_set_norm_prof_data_ip_normalization_en (norm_prof_data_t *data __INOUT__,
    uint8_t ip_normalization_en __IN__)
{
    data->ip_normalization_en = ip_normalization_en;
    data->__ip_normalization_en_valid = TRUE;
}

// --------------------------------------------------------------------------------
// NORM_PROF Setting Filter Field id
// --------------------------------------------------------------------------------
static inline void
hal_set_norm_prof_filter_id (norm_prof_filter_t *filter __INOUT__,
    uint32_t id __IN__)
{
    filter->id = id;
    filter->__id_valid = TRUE;
}

// --------------------------------------------------------------------------------
// NORM_PROF Setting Filter Field stateful_check_en
// --------------------------------------------------------------------------------
static inline void
hal_set_norm_prof_filter_stateful_check_en (norm_prof_filter_t *filter __INOUT__,
    uint8_t stateful_check_en __IN__)
{
    filter->stateful_check_en = stateful_check_en;
    filter->__stateful_check_en_valid = TRUE;
}

// --------------------------------------------------------------------------------
// NORM_PROF Setting Filter Field ip_normalization_en
// --------------------------------------------------------------------------------
static inline void
hal_set_norm_prof_filter_ip_normalization_en (norm_prof_filter_t *filter __INOUT__,
    uint8_t ip_normalization_en __IN__)
{
    filter->ip_normalization_en = ip_normalization_en;
    filter->__ip_normalization_en_valid = TRUE;
}

#endif // __NORM_PROF_H__
