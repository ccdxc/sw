/*
 * swig.h
 * Pensando Systems
 */
/* This file contains data structures and APIs invoked by python through swig 
 */

#ifndef __SWIG_H__
#define __SWIG_H__

#include <string.h>
#include "nic/include/hal_pd_error.hpp"
#include <stdint.h>

uint32_t get_table_id (char *table_name);

void dump_table(uint32_t table_id, egress_policer_actiondata *epad_p);

char* get_data(egress_policer_actiondata *epad_p);

#endif /* __SWIG_H__ */
