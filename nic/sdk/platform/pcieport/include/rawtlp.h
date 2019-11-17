/*
 * Copyright (c) 2019, Pensando Systems Inc.
 */

#ifndef __RAWTLP_H__
#define __RAWTLP_H__

#ifdef __cplusplus
extern "C" {
#if 0
} /* close to calm emacs autoindent */
#endif
#endif

/*
 * rawtlp - This module implements an interface to the raw tlp facility
 * This interface as implemented here is a synchronous, polled interface
 * to send one transaction and receive an optional response.
 * Locking is provided so multiple threads or multiple processes
 * can call this interface simultaneously.
 */

#define RAWTLP_REQDW    12
#define RAWTLP_REQB     (RAWTLP_REQDW * 4)

#define RAWTLP_RSPDW    8
#define RAWTLP_RSPB     (RAWTLP_RSPDW * 4)

typedef union rawtlp_req_u {
    uint32_t w[RAWTLP_REQDW];
    uint8_t  b[RAWTLP_REQB];
} rawtlp_req_t;

typedef union rawtlp_rsp_u {
    uint32_t w[RAWTLP_RSPDW];
    uint8_t  b[RAWTLP_RSPB];
} rawtlp_rsp_t;

typedef union cfg_itr_raw_tlp_cmd_u {
    struct {
        uint32_t dw_cnt:4;
        uint32_t port_id:3;
        uint32_t cmd_go:1;
    } __attribute__((packed));
    uint32_t w;
} cfg_itr_raw_tlp_cmd_t;

typedef union sta_itr_raw_tlp_u {
    struct {
        uint32_t resp_rdy:1;
        uint32_t cpl_stat:3;
        uint32_t cpl_data_err:1;
        uint32_t cpl_timeout_err:1;
        uint32_t req_err:1;
    } __attribute__((packed));
    uint32_t w;
} sta_itr_raw_tlp_t;

/*
 * rawtlp_send - This is the primary interface to use the
 *               pcie rawtlp interface to send pcie transactions
 *               and receive responses.
 *
 * @port:       PCIe port to send the transaction to.
 * @reqtlp:     Raw request tlp, encoded to PCIe spec.
 *              See pcietlp_encode() for encoding help.
 * @reqtlpsz:   Size of the request in @reqtlp.
 * @rspdata:    Buffer for the response data, if any.  If @rspdata is
 *              non-NULL then @rspdatasz bytes will be filled in from
 *              the response data.  If @rspdata is NULL then no response
 *              data is returned.
 * @rspdatasz:  Number of response data bytes to return in @rspdata.
 * @sta:        Raw tlp status register.
 * @return:     Returns 0 on success, <0 on failure.  On success the
 *              send process completed successfully, but caller must
 *              then check the response status in @sta for any response
 *              errors.  See rawtlp_sta_errs().
 */
int rawtlp_send(const int port,
                const uint32_t *reqtlp, const size_t reqtlpsz,
                uint32_t *rspdata, const size_t rspdatasz,
                sta_itr_raw_tlp_t *sta);

/*
 * rawtlp_sta_errs - Translate send status from rawtlp_send().
 *                   rawtlp_sta_errs() will translate the pcie raw tlp send
 *                   to a -errno value, or 0 if no errors.  If any errors
 *                   are detected they are logged.
 *
 * @port:       PCIe port this status was reported from (for logging).
 * @sta:        Raw tlp status register returned from rawtlp_send().
 * @return:     Returns 0 if no errors, or -errno if an error is detected.
 *              -ETIMEDOUT      cpl_timeout_err
 *              -EINVAL         cpl_data_err
 *              -EFAULT         req_err
 *              -EIO            cpl_stat != CPL_SUCCESS
 */
int rawtlp_sta_errs(const int port, sta_itr_raw_tlp_t *sta);

/*
 * hostmem_read - Read from host memory
 *
 * @port:       PCIe port to send the transaction to.
 * @reqid:      PCIe "requester id", the bdf of the device
 *              that is the originator of this read request.
 *              This id is used as credentials while accessing
 *              protected by an IOMMU.
 * @addr:       Host address to read from.
 * @buf:        Caller-supplied buffer used as destination for the read.
 *              Assumed to be at least @count bytes.
 * @count:      Count of bytes to be read from @addr into @buf.
 * @return:     Returns the number of bytes actually read, or <0 on error.
 */
int hostmem_read(const int port, const uint16_t reqid,
                 uint64_t addr, void *buf, size_t count);

/*
 * hostmem_write - Write to host memory
 *
 * @port:       PCIe port to send the transaction to.
 * @reqid:      PCIe "requester id", the bdf of the device
 *              that is the originator of this read request.
 *              This id is used as credentials while accessing
 *              protected by an IOMMU.
 * @addr:       Host address to write to.
 * @buf:        Caller-supplied buffer used contains the bytes to write.
 * @count:      Count of bytes to be written to @addr from @buf.
 * @return:     Returns the number of bytes actually written, or <0 on error.
 */
int hostmem_write(const int port, const uint16_t reqid,
                  uint64_t addr, void *buf, size_t count);

#ifdef __cplusplus
}
#endif

#endif /* __RAWTLP_H__ */
