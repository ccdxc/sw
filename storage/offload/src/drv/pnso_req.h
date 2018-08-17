/*
 * {C} Copyright 2018 Pensando Systems Inc.
 * All rights reserved.
 *
 */
#ifndef __PNSO_REQ_H__
#define __PNSO_REQ_H__

#include "pnso_api.h"

void req_pprint_request(const struct pnso_service_request *req);

void req_pprint_result(const struct pnso_service_result *res);

#endif	/* __PNSO_REQ_H__ */
