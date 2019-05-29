#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s2_t2_nvme_sessprexts_tx_k.h"
#include "capri.h"
#include "nvme_common.h"

struct phv_ p;
struct s2_t2_nvme_sessprexts_tx_k_ k;
struct s2_t2_nvme_sessprexts_tx_page_fetch_process_d d;

%%

.align
nvme_sessprexts_tx_page_fetch_process:
    phvwr.e     p.to_s3_info_page_ptr, d.page_ptr
    CAPRI_SET_TABLE_2_VALID(0)  //Exit Slot
