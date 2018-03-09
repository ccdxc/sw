/*
 * MIME functions for ALG parsing.  This file provides implementations
 * for basic MIME parsing.  MIME headers are used in many protocols, such as
 * HTTP, RTSP, SIP, etc.
 */

#pragma once

#include "nic/include/base.h"
#include "nic/include/ip.h"

namespace hal {
namespace plugins {
namespace alg_utils {

bool alg_mime_nextline(const char* p, uint32_t len, uint32_t* poff,
                       uint32_t* plineoff, uint32_t* plinelen, bool lws_cont = false);
bool alg_mime_token_cmp(const char *buf, uint32_t len, uint32_t *offset,
                        const char *token, char sep, bool space = false, bool ignore_case = true);
void alg_mime_skipws(const char *buf, uint32_t len, uint32_t *offset);
int alg_mime_strtou16(const char* pbuf, uint16_t* pval);
int alg_mime_strtou32(const char* pbuf, uint32_t* pval);
bool alg_mime_strtoip(const char* pbuf, uint32_t len, ip_addr_t* pval);

} // namespace alg_utils
} // namespace plugins
} // namespace hal
