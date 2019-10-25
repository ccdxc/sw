/*
 * Copyright (c) 2018-2019, Pensando Systems Inc.
 */

#ifndef __LIB_PCIEMGRD_IMPL_H__
#define __LIB_PCIEMGRD_IMPL_H__

typedef struct pciemgrenv_s {
    u_int8_t interactive:1;
    u_int8_t gold:1;
    u_int8_t reboot_on_hostdn:1;
    u_int8_t poll_port:1;
    u_int8_t poll_dev:1;
    u_int8_t mlockall:1;
    u_int8_t enabled_ports;
    u_int32_t cpumask;
    u_int32_t fifopri;
    pciemgr_params_t params;
} pciemgrenv_t;

pciemgrenv_t *pciemgrenv_get(void);
void pciemgrd_catalog_defaults(pciemgrenv_t *pme);
void pciemgrd_params(pciemgrenv_t *pme);
void pciemgrd_sys_init(pciemgrenv_t *pme);
int server_loop(pciemgrenv_t *pme);
int open_hostports(void);
void close_hostports(void);
int intr_init(void);
void logger_init(void);

int upgrade_state_save(void);
int upgrade_state_restore(void);
int upgrade_in_progress(void);
int upgrade_complete(void);
int upgrade_failed(void);
int upgrade_rollback_begin(void);
int upgrade_rollback_in_progress(void);
int upgrade_rollback_complete(void);

int delphi_client_start();

typedef enum pciemgr_port_status_e {
    PCIEMGR_UP,
    PCIEMGR_DOWN,
    PCIEMGR_FAULT,
} pciemgr_port_status_t;

void
update_pcie_port_status(const int port,
                        const pciemgr_port_status_t status,
                        const int gen = 0,
                        const int width = 0,
                        const char *faultstr = "");
void update_pcie_metrics(const int port);

#endif /* __LIB_PCIEMGRD_IMPL_H__ */
