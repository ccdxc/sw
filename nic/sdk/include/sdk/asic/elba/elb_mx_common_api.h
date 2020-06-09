#ifndef ELB_MX_COMMON_API_H
#define ELB_MX_COMMON_API_H

#define MAX_CHANNEL    8
#define MAX_PORT       5
#define MAX_MAC        1
#define MAX_MAC_STATS  110

typedef enum {
   ELB_MX_MAC_CHMODE_ADDR0			= 0x0000,
   ELB_MX_MAC_CHMODE_ADDR_INC			= 0x200,
   ELB_MX_MAC_CHMODE_FLD_MODE_POS		= 0,
   ELB_MX_MAC_CHMODE_FLD_MODE_SZ		= 6,
   ELB_MX_MAC_CHMODE_FLD_TXSWRST_POS		= 6,
   ELB_MX_MAC_CHMODE_FLD_TXSWRST_SZ		= 1,
   ELB_MX_MAC_CHMODE_FLD_RXSWRST_POS		= 7,
   ELB_MX_MAC_CHMODE_FLD_RXSWRST_SZ		= 1,
   ELB_MX_MAC_CHMODE_FLD_TXEN_POS		= 8,
   ELB_MX_MAC_CHMODE_FLD_TXEN_SZ		= 1,
   ELB_MX_MAC_CHMODE_FLD_TXDRAIN_POS		= 9,
   ELB_MX_MAC_CHMODE_FLD_TXDRAIN_SZ		= 1,
   ELB_MX_MAC_CHMODE_FLD_RXEN_POS		= 10,
   ELB_MX_MAC_CHMODE_FLD_RXEN_SZ		= 1,
   ELB_MX_MAC_CHMODE_FLD_GMIILPBK_POS		= 11,
   ELB_MX_MAC_CHMODE_FLD_GMIILPBK_SZ		= 1,
   ELB_MX_MAC_CHMODE_FLD_TXJABBER_POS		= 12,
   ELB_MX_MAC_CHMODE_FLD_TXJABBER_SZ		= 16,
   ELB_MX_MAC_CHMODE_FLD_RXJABBER_POS		= 28,
   ELB_MX_MAC_CHMODE_FLD_RXJABBER_SZ		= 16,
   ELB_MX_MAC_CHMODE_FLD_DISFCS_POS		= 44,
   ELB_MX_MAC_CHMODE_FLD_DISFCS_SZ		= 1,
   ELB_MX_MAC_CHMODE_FLD_INVFCS_POS		= 45,
   ELB_MX_MAC_CHMODE_FLD_INVFCS_SZ		= 1,
   ELB_MX_MAC_CHMODE_FLD_IGNFCS_POS		= 46,
   ELB_MX_MAC_CHMODE_FLD_IGNFCS_SZ		= 1,
   ELB_MX_MAC_CHMODE_FLD_STRIPFCS_POS		= 47,
   ELB_MX_MAC_CHMODE_FLD_STRIPFCS_SZ		= 1,
   ELB_MX_MAC_CHMODE_FLD_IFGLEN_POS		= 48,
   ELB_MX_MAC_CHMODE_FLD_IFGLEN_SZ		= 8,
   ELB_MX_MAC_CHMODE_FLD_IFGPACING_POS		= 56,
   ELB_MX_MAC_CHMODE_FLD_IFGPACING_SZ		= 8,

   ELB_MX_MAC_MACCFG_ADDR0			= 0x0008,
   ELB_MX_MAC_MACCFG_ADDR_INC			= 0x200,
   ELB_MX_MAC_MACCFG_FLD_TXFCEN_POS		= 8,
   ELB_MX_MAC_MACCFG_FLD_TXFCEN_SZ 		= 1,
   ELB_MX_MAC_MACCFG_FLD_RXFCEN_POS		= 10,
   ELB_MX_MAC_MACCFG_FLD_RXFCEN_SZ 		= 1,
   ELB_MX_MAC_MACCFG_FLD_RXPFCEN_POS		= 11,
   ELB_MX_MAC_MACCFG_FLD_RXPFCEN_SZ 		= 1,
   ELB_MX_MAC_MACCFG_FLD_RXFCTOTX_POS		= 12,
   ELB_MX_MAC_MACCFG_FLD_RXFCTOTX_SZ 		= 1,
   ELB_MX_MAC_MACCFG_FLD_RXFILTERFC_POS		= 13,
   ELB_MX_MAC_MACCFG_FLD_RXFILTERFC_SZ 		= 1,
   ELB_MX_MAC_MACCFG_FLD_RXFILTERPFC_POS	= 14,
   ELB_MX_MAC_MACCFG_FLD_RXFILTERPFC_SZ 	= 1,
   ELB_MX_MAC_MACCFG_FLD_TXPADRUNT_POS		= 15,
   ELB_MX_MAC_MACCFG_FLD_TXPADRUNT_SZ 		= 8,
   ELB_MX_MAC_MACCFG_FLD_TXWRTHRESH_POS		= 23,
   ELB_MX_MAC_MACCFG_FLD_TXWRTHRESH_SZ 		= 8,
   ELB_MX_MAC_MACCFG_FLD_TXRDTHRESH_POS		= 31,
   ELB_MX_MAC_MACCFG_FLD_TXRDTHRESH_SZ 		= 8,
   ELB_MX_MAC_MACCFG_FLD_TXLFAULT_POS		= 39,
   ELB_MX_MAC_MACCFG_FLD_TXLFAULT_SZ 		= 1,
   ELB_MX_MAC_MACCFG_FLD_TXRFAULT_POS		= 40,
   ELB_MX_MAC_MACCFG_FLD_TXRFAULT_SZ 		= 1,
   ELB_MX_MAC_MACCFG_FLD_TXIDLE_POS		= 41,
   ELB_MX_MAC_MACCFG_FLD_TXIDLE_SZ 		= 1,
   ELB_MX_MAC_MACCFG_FLD_RXPADRUNT_POS		= 42,
   ELB_MX_MAC_MACCFG_FLD_RXPADRUNT_SZ 		= 8,
   ELB_MX_MAC_MACCFG_FLD_RXLFAULT_POS		= 50,
   ELB_MX_MAC_MACCFG_FLD_RXLFAULT_SZ 		= 1,
   ELB_MX_MAC_MACCFG_FLD_RXRFAULT_POS		= 51,
   ELB_MX_MAC_MACCFG_FLD_RXRFAULT_SZ 		= 1,
   ELB_MX_MAC_MACCFG_FLD_RXIDLE_POS		= 52,
   ELB_MX_MAC_MACCFG_FLD_RXIDLE_SZ 		= 1,
   ELB_MX_MAC_MACCFG_FLD_STATSCLR_POS		= 54,
   ELB_MX_MAC_MACCFG_FLD_STATSCLR_SZ 		= 1,
   ELB_MX_MAC_MACCFG_FLD_TXIGNORERX_POS		= 55,
   ELB_MX_MAC_MACCFG_FLD_TXIGNORERX_SZ 		= 1,
   ELB_MX_MAC_MACCFG_FLD_TXPFCEN_POS		= 56,
   ELB_MX_MAC_MACCFG_FLD_TXPFCEN_SZ 		= 8,

   ELB_MX_MAC_CHSTS_ADDR0              		= 0x0010,
   ELB_MX_MAC_CHSTS_ADDR_INC          		= 0x200,
   ELB_MX_MAC_CHSTS_FLD_TXCLKPRESENTALL_POS	= 0,
   ELB_MX_MAC_CHSTS_FLD_TXCLKPRESENTALL_SZ	= 1,
   ELB_MX_MAC_CHSTS_FLD_RXCLKPRESENTALL_POS	= 1,
   ELB_MX_MAC_CHSTS_FLD_RXCLKPRESENTALL_SZ	= 1,
   ELB_MX_MAC_CHSTS_FLD_RXSIGOKALL_POS		= 2,
   ELB_MX_MAC_CHSTS_FLD_RXSIGOKALL_SZ		= 1,
   ELB_MX_MAC_CHSTS_FLD_BLOCKLOCKALL_POS	= 3,
   ELB_MX_MAC_CHSTS_FLD_BLOCKLOCKALL_SZ		= 1,
   ELB_MX_MAC_CHSTS_FLD_AMLOCKALL_POS		= 4,
   ELB_MX_MAC_CHSTS_FLD_AMLOCKALL_SZ		= 1,
   ELB_MX_MAC_CHSTS_FLD_ALIGNED_POS		= 5,
   ELB_MX_MAC_CHSTS_FLD_ALIGNED_SZ		= 1,
   ELB_MX_MAC_CHSTS_FLD_NOHIBER_POS		= 6,
   ELB_MX_MAC_CHSTS_FLD_NOHIBER_SZ		= 1,
   ELB_MX_MAC_CHSTS_FLD_NOLOCALFAULT_POS	= 7,
   ELB_MX_MAC_CHSTS_FLD_NOLOCALFAULT_SZ		= 1,
   ELB_MX_MAC_CHSTS_FLD_NOREMOTEFAULT_POS	= 8,
   ELB_MX_MAC_CHSTS_FLD_NOREMOTEFAULT_SZ	= 1,
   ELB_MX_MAC_CHSTS_FLD_LINKUP_POS		= 9,
   ELB_MX_MAC_CHSTS_FLD_LINKUP_SZ		= 1,
   ELB_MX_MAC_CHSTS_FLD_HISER_POS		= 10,
   ELB_MX_MAC_CHSTS_FLD_HISER_SZ		= 1,
   ELB_MX_MAC_CHSTS_FLD_FECDEGSER_POS		= 11,
   ELB_MX_MAC_CHSTS_FLD_FECDEGSER_SZ		= 1,
   ELB_MX_MAC_CHSTS_FLD_RXAMSF_POS		= 12,
   ELB_MX_MAC_CHSTS_FLD_RXAMSF_SZ		= 3,

   ELB_MX_MAC_CHCONFIG3_ADDR0              	= 0x0018,
   ELB_MX_MAC_CHCONFIG3_ADDR_INC          	= 0x200,
   ELB_MX_MAC_CHCONFIG3_FLD_IFGPPM_POS		= 0,
   ELB_MX_MAC_CHCONFIG3_FLD_IFGPPM_SZ		= 16,
   ELB_MX_MAC_CHCONFIG3_FLD_RXMAXFRMSIZE_POS	= 16,
   ELB_MX_MAC_CHCONFIG3_FLD_RXMAXFRMSIZE_SZ	= 16,
   ELB_MX_MAC_CHCONFIG3_FLD_TXDRAINONFAULT_POS	= 37,
   ELB_MX_MAC_CHCONFIG3_FLD_TXDRAINONFAULT_SZ	= 1,
   ELB_MX_MAC_CHCONFIG3_FLD_RXERRMASK_POS	= 39,
   ELB_MX_MAC_CHCONFIG3_FLD_RXERRMASK_SZ	= 5,
   ELB_MX_MAC_CHCONFIG3_FLD_RXCRCERRMASK_POS	= 39,
   ELB_MX_MAC_CHCONFIG3_FLD_RXCRCERRMASK_SZ	= 1,
   ELB_MX_MAC_CHCONFIG3_FLD_RXJABBERERRMASK_POS	= 40,
   ELB_MX_MAC_CHCONFIG3_FLD_RXJABBERERRMASK_Sz	= 1,
   ELB_MX_MAC_CHCONFIG3_FLD_RXPCSERRMASK_POS	= 41,
   ELB_MX_MAC_CHCONFIG3_FLD_RXPCSERRMASK_SZ	= 1,
   ELB_MX_MAC_CHCONFIG3_FLD_RXMAXLENERRMASK_POS	= 42,
   ELB_MX_MAC_CHCONFIG3_FLD_RXMAXLENERRMASK_SZ	= 1,
   ELB_MX_MAC_CHCONFIG3_FLD_RXETHLENERRMASK_POS	= 43,
   ELB_MX_MAC_CHCONFIG3_FLD_RXETHLENERRMASK_SZ	= 1,
   ELB_MX_MAC_CHCONFIG3_FLD_SJSIZE_POS		= 44,
   ELB_MX_MAC_CHCONFIG3_FLD_SJSIZE_SZ		= 12,

   ELB_MX_MAC_CHCONFIG4_ADDR0              	= 0x0020,
   ELB_MX_MAC_CHCONFIG4_ADDR_INC          	= 0x200,
   ELB_MX_MAC_CHCONFIG4_FLD_MACADDR_POS		= 0,
   ELB_MX_MAC_CHCONFIG4_FLD_MACADDR_SZ		= 48,
   ELB_MX_MAC_CHCONFIG4_FLD_PAUSEONTIME_POS	= 48,
   ELB_MX_MAC_CHCONFIG4_FLD_PAUSEONTIME_SZ	= 16,

   ELB_MX_MAC_CHCONFIG5_ADDR0              	= 0x0028,
   ELB_MX_MAC_CHCONFIG5_ADDR_INC          	= 0x200,
   ELB_MX_MAC_CHCONFIG5_FLD_PAUSEDEST_POS	= 0,
   ELB_MX_MAC_CHCONFIG5_FLD_PAUSEDEST_SZ	= 48,
   ELB_MX_MAC_CHCONFIG5_FLD_PAUSEREFRESH_POS	= 48,
   ELB_MX_MAC_CHCONFIG5_FLD_PAUSEREFRESH_SZ	= 16,

   ELB_MX_MAC_CHCONFIG6_ADDR0              	= 0x0030,
   ELB_MX_MAC_CHCONFIG6_ADDR_INC          	= 0x200,
   ELB_MX_MAC_CHCONFIG6_FLD_CORRBYP_POS		= 0,
   ELB_MX_MAC_CHCONFIG6_FLD_CORRBYP_SZ		= 1,
   ELB_MX_MAC_CHCONFIG6_FLD_INDIBYP_POS		= 1,
   ELB_MX_MAC_CHCONFIG6_FLD_INDIBYP_SZ		= 1,
   ELB_MX_MAC_CHCONFIG6_FLD_HISERTHRESH_POS	= 2,
   ELB_MX_MAC_CHCONFIG6_FLD_HISERTHRESH_SZ	= 13,
   ELB_MX_MAC_CHCONFIG6_FLD_DEGSERENABLE_POS	= 15,
   ELB_MX_MAC_CHCONFIG6_FLD_DEGSERENABLE_SZ	= 1,
   ELB_MX_MAC_CHCONFIG6_FLD_DEGSERINTERVAL_POS	= 16,
   ELB_MX_MAC_CHCONFIG6_FLD_DEGSERINTERVAL_SZ	= 32,

   ELB_MX_MAC_CHCONFIG7_ADDR0              		= 0x0038,
   ELB_MX_MAC_CHCONFIG7_ADDR_INC          		= 0x200,
   ELB_MX_MAC_CHCONFIG7_FLD_DEGSERACTIVATETHRESH_POS	= 0,
   ELB_MX_MAC_CHCONFIG7_FLD_DEGSERACTIVATETHRESH_SZ	= 32,
   ELB_MX_MAC_CHCONFIG7_FLD_DEGSERDEACTIVATETHRESH_POS	= 32,
   ELB_MX_MAC_CHCONFIG7_FLD_DEGSERDEACTIVATETHRESH_SZ	= 32,

   ELB_MX_MAC_CHCONFIG8_ADDR0              	= 0x0040,
   ELB_MX_MAC_CHCONFIG8_ADDR_INC          	= 0x200,
   ELB_MX_MAC_CHCONFIG8_FLD_VLANTAG1_POS	= 0,
   ELB_MX_MAC_CHCONFIG8_FLD_VLANTAG1_SZ		= 16,
   ELB_MX_MAC_CHCONFIG8_FLD_VLANTAG2_POS	= 16,
   ELB_MX_MAC_CHCONFIG8_FLD_VLANTAG2_SZ		= 16,
   ELB_MX_MAC_CHCONFIG8_FLD_VLANTAG3_POS	= 32,
   ELB_MX_MAC_CHCONFIG8_FLD_VLANTAG3_SZ		= 16,
   ELB_MX_MAC_CHCONFIG8_FLD_MAXVLANCNT_POS	= 48,
   ELB_MX_MAC_CHCONFIG8_FLD_MAXVLANCNT_SZ	= 2,
   ELB_MX_MAC_CHCONFIG8_FLD_USCLKCNT_POS	= 50,
   ELB_MX_MAC_CHCONFIG8_FLD_USCLKCNT_SZ		= 12,

   ELB_MX_MAC_INTCONTROL_ADDR0             	= 0x00b0,
   ELB_MX_MAC_INTCONTROL_ADDR_INC          	= 0x200,
   ELB_MX_MAC_INTCONTROL_FLD_INTSTS_POS		= 0,
   ELB_MX_MAC_INTCONTROL_FLD_INTSTS_SZ		= 16,
   ELB_MX_MAC_INTCONTROL_FLD_INTCLR_POS		= 16,
   ELB_MX_MAC_INTCONTROL_FLD_INTCLR_SZ		= 16,
   ELB_MX_MAC_INTCONTROL_FLD_INTENA_POS		= 32,
   ELB_MX_MAC_INTCONTROL_FLD_INTENA_SZ		= 16,
   ELB_MX_MAC_INTCONTROL_FLD_INTRAW_POS		= 48,
   ELB_MX_MAC_INTCONTROL_FLD_INTRAW_SZ		= 16,

   // 2 pcs serdes registers per channels
   ELB_MX_MAC_SDCFG_ADDR0			= 0x1000,
   ELB_MX_MAC_SDCFG_ADDR1			= 0x1080,
   ELB_MX_MAC_SDCFG_ADDR_INC			= 0x100,
   ELB_MX_MAC_SDCFG_FLD_SERDESLPBK_POS		= 0,
   ELB_MX_MAC_SDCFG_FLD_SERDESLPBK_SZ		= 1,

   ELB_MX_MAC_APPCFG0_ADDR             		= 0x6000,
   ELB_MX_MAC_APPCFG0_FLD_STATSCOR_POS    	= 11,
   ELB_MX_MAC_APPCFG0_FLD_STATSCOR_SZ    	= 1,
} elb_mx_mac_t;

typedef enum {
   ELB_MX_AN_ANCFG_ADDR0			= 0x5000,
   ELB_MX_AN_ANCFG_ADDR_INC			= 0x20,
   ELB_MX_AN_ANCFG_FLD_ANSWRST_POS		= 0,
   ELB_MX_AN_ANCFG_FLD_ANSWRST_SZ		= 1,
   ELB_MX_AN_ANCFG_FLD_ANENABLE_POS		= 1,
   ELB_MX_AN_ANCFG_FLD_ANENABLE_SZ		= 1,
   ELB_MX_AN_ANCFG_FLD_ANTNONCEFORCEEN_POS	= 2,
   ELB_MX_AN_ANCFG_FLD_ANTNONCEFORCEEN_SZ	= 1,
   ELB_MX_AN_ANCFG_FLD_ANIGNORENONCE_POS	= 3,
   ELB_MX_AN_ANCFG_FLD_ANIGNORENONCE_SZ		= 1,
   ELB_MX_AN_ANCFG_FLD_TXADDRESS_POS		= 4,
   ELB_MX_AN_ANCFG_FLD_TXADDRESS_SZ		= 8,
   ELB_MX_AN_ANCFG_FLD_RXADDRESS_POS		= 12,
   ELB_MX_AN_ANCFG_FLD_RXADDRESS_SZ		= 8,
   ELB_MX_AN_ANCFG_FLD_FASTSIM_POS		= 20,
   ELB_MX_AN_ANCFG_FLD_FASTSIM_SZ		= 1,
   ELB_MX_AN_ANCFG_FLD_BREAKLINKTIMER_POS	= 32,
   ELB_MX_AN_ANCFG_FLD_BREAKLINKTIMER_SZ	= 32,

   ELB_MX_AN_ANSTS_ADDR0			= 0x5008,
   ELB_MX_AN_ANSTS_ADDR_INC			= 0x20,
   ELB_MX_AN_ANSTS_FLD_LPANENABLE_POS		= 0,
   ELB_MX_AN_ANSTS_FLD_LPANENABLE_SZ		= 1,
   ELB_MX_AN_ANSTS_FLD_ANCOMPLETE_POS		= 1,
   ELB_MX_AN_ANSTS_FLD_ANCOMPLETE_SZ		= 1,
   ELB_MX_AN_ANSTS_FLD_RECEIVEDLAST_POS		= 2,
   ELB_MX_AN_ANSTS_FLD_RECEIVEDLAST_SZ		= 1,
   ELB_MX_AN_ANSTS_FLD_ANMODE_POS		= 3,
   ELB_MX_AN_ANSTS_FLD_ANMODE_SZ		= 25,
   ELB_MX_AN_ANSTS_FLD_FECMODE_POS		= 28,
   ELB_MX_AN_ANSTS_FLD_FECMODE_SZ		= 2,
   ELB_MX_AN_ANSTS_FLD_PAUSE_POS		= 30,
   ELB_MX_AN_ANSTS_FLD_PAUSE_SZ			= 2,
   ELB_MX_AN_ANSTS_FLD_ABILITYMATCH_POS		= 32,
   ELB_MX_AN_ANSTS_FLD_ABILITYMATCH_SZ		= 1,
   ELB_MX_AN_ANSTS_FLD_ACKMATCH_POS		= 33,
   ELB_MX_AN_ANSTS_FLD_ACKMATCH_SZ		= 1,
   ELB_MX_AN_ANSTS_FLD_RXNP_POS			= 34,
   ELB_MX_AN_ANSTS_FLD_RXNP_SZ			= 1,
   ELB_MX_AN_ANSTS_FLD_TXDONE_POS		= 35,
   ELB_MX_AN_ANSTS_FLD_TXDONE_SZ		= 1,
   ELB_MX_AN_ANSTS_FLD_TXBP_POS			= 36,
   ELB_MX_AN_ANSTS_FLD_TXBP_SZ			= 1,
   ELB_MX_AN_ANSTS_FLD_NONCECOLLISION_POS	= 37,
   ELB_MX_AN_ANSTS_FLD_NONCECOLLISION_SZ	= 1,
   ELB_MX_AN_ANSTS_FLD_MAXTIMERDONE_POS		= 38,
   ELB_MX_AN_ANSTS_FLD_MAXTIMERDONE_SZ		= 1,
   ELB_MX_AN_ANSTS_FLD_MINTIMERNOTDONE_POS	= 39,
   ELB_MX_AN_ANSTS_FLD_MINTIMERNOTDONE_SZ	= 1,

   ELB_MX_AN_ANINTCONTROL_ADDR0			= 0x5010,
   ELB_MX_AN_ANINTCONTROL_ADDR_INC		= 0x20,
   ELB_MX_AN_ANINTCONTROL_FLD_INTSTS_POS	= 0,
   ELB_MX_AN_ANINTCONTROL_FLD_INTSTS_SZ		= 4,
   ELB_MX_AN_ANINTCONTROL_FLD_INTCLR_POS	= 4,
   ELB_MX_AN_ANINTCONTROL_FLD_INTCLR_SZ		= 4,
   ELB_MX_AN_ANINTCONTROL_FLD_INTENA_POS	= 8,
   ELB_MX_AN_ANINTCONTROL_FLD_INTENA_SZ		= 4,
   ELB_MX_AN_ANINTCONTROL_FLD_INTRAW_POS	= 12,
   ELB_MX_AN_ANINTCONTROL_FLD_INTRAW_SZ		= 4,

   ELB_MX_AN_PAGETESTTIMER_ADDR0			= 0x5018,
   ELB_MX_AN_PAGETESTTIMER_ADDR_INC			= 0x20,
   ELB_MX_AN_PAGETESTTIMER_FLD_PAGETESTMINTIMER_POS	= 0,
   ELB_MX_AN_PAGETESTTIMER_FLD_PAGETESTMINTIMER_SZ	= 32,
   ELB_MX_AN_PAGETESTTIMER_FLD_PAGETESTMAXTIMER_POS	= 32,
   ELB_MX_AN_PAGETESTTIMER_FLD_PAGETESTMAXTIMER_SZ	= 32,
} elb_mx_an_t;

typedef enum {
   ELB_MX_MAC_STAT_ANY			= 0,
   ELB_MX_MAC_STAT_ERR			= 1,
   ELB_MX_MAC_STAT_PAUSE		= 2,
   ELB_MX_MAC_STAT_PCS			= 3,
   ELB_MX_MAC_STAT_TXDRAIN		= 4,
} elb_mx_mac_stat_type_t;

typedef enum {
   ELB_MX_INT_GROUPS_INT_SD		= 0,
   ELB_MX_INT_GROUPS_INT_MAC		= 1,
   ELB_MX_INT_GROUPS_INT_ECC		= 2,
   ELB_MX_INT_GROUPS_INT_FIXER		= 3,
   ELB_MX_INT_GROUPS_INT_FLUSH		= 4,
} elb_mx_int_groups_t;

typedef enum {
   ELB_MX_INT_MAC_INT_MAC		= 0,
   ELB_MX_INT_MAC_INT_AN		= 1,
} elb_mx_int_mac_t;

typedef enum {
   ELB_MX_MAC_INT_TX_FIFO_LEVEL_POS	= 0,
   ELB_MX_MAC_INT_TX_PROTOCOL_ERR_POS	= 1,
   ELB_MX_MAC_INT_TX_JABBER_PKT_POS	= 2,
   ELB_MX_MAC_INT_TX_SERDES_FIFO_POS	= 3,
   ELB_MX_MAC_INT_RX_PCS_GEARBOX_POS	= 4,
   ELB_MX_MAC_INT_RX_PCS_DESKEW_POS	= 5,
   ELB_MX_MAC_INT_RX_PCS_HIBER_POS	= 6,
   ELB_MX_MAC_INT_RX_PCS_ERROR_POS	= 7,
   ELB_MX_MAC_INT_RX_FAULT_POS		= 8,
   ELB_MX_MAC_INT_RX_FRAME_DROP_POS	= 9,
   ELB_MX_MAC_INT_RX_FCS_ERR_POS	= 10,
   ELB_MX_MAC_INT_RX_JABBER_PKT_POS	= 11,
   ELB_MX_MAC_INT_RX_RUNT_DROP_POS	= 12,
   ELB_MX_MAC_INT_RX_FIFO_LEVEL_POS	= 13,
   ELB_MX_MAC_INT_RX_LINK_LOST_POS	= 14,
   ELB_MX_MAC_INT_RX_LINK_GAIN_POS	= 15,
} elb_mx_mac_int_t;

typedef enum {
   ELB_MX_AN_BASE_SELECTOR_POS		= 0,	// Set to 0x1 for base pages
   ELB_MX_AN_BASE_SELECTOR_SZ		= 5,
   ELB_MX_AN_BASE_ECHOED_NONCE_POS	= 5,	// Controlled internally - can be ignored
   ELB_MX_AN_BASE_ECHOED_NONCE_SZ	= 5,
   ELB_MX_AN_BASE_PAUSE_CAPABILITY_POS	= 10,	// Bit0: Pause is supported. Bit1: Pause is asynchronous
   ELB_MX_AN_BASE_PAUSE_CAPABILITY_SZ	= 2,
   ELB_MX_AN_BASE_RESERVED_POS		= 12,	
   ELB_MX_AN_BASE_RESERVED_SZ		= 1,
   ELB_MX_AN_BASE_REMOTE_FAULT_POS	= 13,	// Controlled internally - can be ignored
   ELB_MX_AN_BASE_REMOTE_FAULT_SZ	= 1,
   ELB_MX_AN_BASE_ACK			= 14,	// Controlled internally - can be ignored
   ELB_MX_AN_BASE_ACK_SZ		= 1,
   ELB_MX_AN_BASE_NP_POS		= 15,	// Next Page indication, must be set if a next page is requird to be sent
   ELB_MX_AN_BASE_NP_SZ			= 1,
   ELB_MX_AN_BASE_TX_NONCE_POS		= 16,	// Controlled internally - can be ignored. Internal generation of TX nonce can 
   ELB_MX_AN_BASE_TX_NONCE_SZ		= 5,	// also be overridden by setting this value in memory and setting antnonceforceen register to 1.
   ELB_MX_AN_BASE_TECHABILITY_POS	= 21,	// see mx_an_techability_t
   ELB_MX_AN_BASE_TECHABILITY_SZ	= 23,
   ELB_MX_AN_BASE_FEC_CAPABILITY_POS	= 44,	// see mx_an_fec_capability_t
   ELB_MX_AN_BASE_FEC_CAPABILITY_SZ	= 4,
} elb_mx_an_base_config_t;

typedef enum {
   ELB_MX_AN_PAUSE_SUPPORTED		= 0,
   ELB_MX_AN_PAUSE_ASYNC		= 1,
} elb_mx_an_pause_capability_t;

typedef enum {
   ELB_MX_AN_TECH_KX_1G			= 0,
   ELB_MX_AN_TECH_KX4_10G		= 1,
   ELB_MX_AN_TECH_KR_10G		= 2,
   ELB_MX_AN_TECH_KR4_40G		= 3,
   ELB_MX_AN_TECH_CR4_40G		= 4,
   ELB_MX_AN_TECH_CR10_100G		= 5,
   ELB_MX_AN_TECH_KP4_100G_		= 6,
   ELB_MX_AN_TECH_KP4_100G 		= 7,
   ELB_MX_AN_TECH_CR4_100G		= 8,
   ELB_MX_AN_TECH_KRS_CRS_25G		= 9,
   ELB_MX_AN_TECH_KR_CR_25G		= 10,
   ELB_MX_AN_TECH_KX_2P5_G		= 11,
   ELB_MX_AN_TECH_KR_5G			= 12,
   ELB_MX_AN_TECH_KR_CR_50G		= 13,
   ELB_MX_AN_TECH_KR2_CR2_100G		= 14,
   ELB_MX_AN_TECH_KR4_CR4_200G		= 15,
   ELB_MX_AN_TECH_KR_CR_100G		= 16,
   ELB_MX_AN_TECH_KR2_CR2_200G		= 17,
   ELB_MX_AN_TECH_KR4_CR4_400G		= 18,
   ELB_MX_AN_TECH_RSVD0			= 19,
   ELB_MX_AN_TECH_RSVD1			= 20,
   ELB_MX_AN_TECH_RSVD2			= 21,
   ELB_MX_AN_TECH_RSVD3			= 22,
} elb_mx_an_techability_t;

typedef enum {
   ELB_MX_AN_FEC_RSFEC_25G_REQ		= 0,
   ELB_MX_AN_FEC_FCFEC_25G_REQ		= 1,
   ELB_MX_AN_FEC_FEC_10G_ABI		= 2,
   ELB_MX_AN_FEC_FEC_10G_REQ		= 3,
} elb_mx_an_fec_capability_t;

// Keep old struct for bx. There is no need for mac_profile_t for elb_mx.
struct mac_profile_t {
   mac_mode_t mac_mode;
   int low_power_mode;
   int enable_pause;
   int enable_legacy_pause;
   int tx_pad_disable;
   int enable_mx_sd_init;
   int serdes_speed;
   int ld_sd_rom_bd;
   int for_simulation;
   int glbl_mode;
   int port_enable[MAX_PORT];
   int ch_mode[MAX_CHANNEL];
   int speed[MAX_CHANNEL];
   int enable[MAX_CHANNEL];
   int serdes_lp;
   int tdm[MAX_CHANNEL];
};
extern mac_profile_t bx[MAX_MAC];

uint64_t elb_mx_apb_read(int chip_id, int inst_id, int addr);
void elb_mx_apb_write(int chip_id, int inst_id, int addr, uint64_t data);

void elb_mx_set_soft_reset(int chip_id, int inst_id, int port, int ch, int value);

void elb_mx_set_clock_info(int chip_id, int inst_id, int value);
int elb_mx_set_ch_mode(int chip_id, int inst_id, int ch, int speed, int num_lanes, int fec);
int elb_mx_gmii_lpbk_get (int chip_id, int inst_id, int ch);
void elb_mx_gmii_lpbk_set (int chip_id, int inst_id, int ch, int value);
int elb_mx_pcs_lpbk_get (int chip_id, int inst_id, int ch);
void elb_mx_pcs_lpbk_set (int chip_id, int inst_id, int ch, int num_lanes, int value);
void elb_mx_set_auto_flush(int chip_id, int inst_id, int port, int flush_timeout);
void elb_mx_set_flush(int chip_id, int inst_id, int port, int flush);
void elb_mx_set_rxsm_enable(int chip_id, int inst_id, int port, int enable);
extern "C" void elb_mx_set_tx_autodrain(int chip_id, int inst_id, int ch, int value);
void elb_mx_set_fixer_timeout(int chip_id, int inst_id, int port, int value);
void elb_mx_set_tx_drain(int chip_id, int inst_id, int ch, bool drain);

int elb_mx_slot_gen(int inst_id, int *port_vec, int *port_ch_map, int *port_speed, int *slot_port, int *slot_ch);
void elb_mx_get_slot(int chip_id, int inst_id);
int elb_mx_set_slot(int chip_id, int inst_id, int *slot_port, int *slot_ch);
void elb_mx_slot_gen(int inst_id, int port_vec0, int port_ch_map0, int port_speed0, int port_vec1, int port_ch_map1, int port_speed1, int port_vec2, int port_ch_map2, int port_speed2, int port_vec3, int port_ch_map3, int port_speed3, int port_vec4, int port_ch_map4, int port_speed4);
void elb_mx_set_slot(int chip_id, int inst_id, int slot_port0, int slot_ch0, int slot_port1, int slot_ch1, int slot_port2, int slot_ch2, int slot_port3, int slot_ch3, int slot_port4, int slot_ch4, int slot_port5, int slot_ch5, int slot_port6, int slot_ch6, int slot_port7, int slot_ch7);

void elb_mx_set_mtu_jabber(int chip_id , int inst_id, int ch, int max_value, int jabber_value);
int elb_mx_set_pause(int chip_id, int inst_id, int port, int ch, int pri_vec, int legacy, bool tx_pause_enable, bool rx_pause_enable);
int elb_mx_set_pause_src_addr(int chip_id, int inst_id, int ch, uint8_t *mac_addr);
int elb_mx_set_pause_dest_addr(int chip_id, int inst_id, int ch, uint8_t *mac_addr);
int elb_mx_set_pauseontime(int chip_id, int inst_id, int ch, int value);
int elb_mx_set_pauserefresh(int chip_id, int inst_id, int ch, int value);
int elb_mx_check_ch_sync(int chip_id, int inst_id, int ch);
void elb_mx_set_txthresh(int chip_id, int inst_id, int ch, int txwrthresh, int txrdthresh);
void elb_mx_set_ifglen(int chip_id, int inst_id, int ch, int value);
void elb_mx_disable_eth_len_err(int chip_id, int inst_id, int ch, int value);
void elb_mx_set_ch_enable(int chip_id, int inst_id, int ch, int txen, int rxen);
void elb_mx_set_vlan_check (int chip_id, int inst_id, int mac_ch, int num_tags, uint32_t tag1, uint32_t tag2, uint32_t tag3);

void elb_mx_set_txpadrunt(int chip_id, int inst_id, int ch, int value);
void elb_mx_set_tx_padding(int chip_id, int inst_id, int ch, int enable);
void elb_mx_set_rx_padding(int chip_id, int inst_id, int ch, int value);
void elb_mx_send_remote_faults(int chip_id, int inst_id, int ch, bool send);
void elb_mx_set_hiser_thresh(int chip_id, int inst_id, int ch, int value);
void elb_mx_enable_false_linkup_detection(int chip_id , int inst_id, int ch, int value);

int elb_mx_get_mac_chsts(int chip_id, int inst_id, int ch, bool display=true);
int elb_mx_get_ch_sync(int chip_id, int inst_id, int ch);
int elb_mx_get_ch_fault(int chip_id, int inst_id, int ch);
int elb_mx_get_ch_pcs_err(int chip_id, int inst_id, int ch);

uint64_t elb_mx_read_stats(int chip_id, int inst_id, int ch, int index, bool clear_on_read=false);
void elb_mx_mac_stat(int chip_id, int inst_id, int ch, uint64_t *stats_data);
void elb_mx_clear_mac_stat(int chip_id, int inst_id, int ch, int value);
void elb_mx_get_mac_stat(int chip_id, int inst_id, int ch, int stat_type=0, bool skip_zero=true, bool clear_on_read=false, bool display=true);
void elb_mx_get_mac_stat_err(int chip_id, int inst_id, int ch, bool skip_zero=true, bool clear_on_read=false, bool display=true);
void elb_mx_get_mac_stat_pause(int chip_id, int inst_id, int ch, bool skip_zero=true, bool clear_on_read=false, bool display=true);

int elb_mx_get_mac_int(int chip_id, int inst_id, int ch, bool clear_on_read=false, bool display=true);
void elb_mx_enable_mac_int(int chip_id, int inst_id, int ch, int vec);
void elb_mx_clear_mac_int(int chip_id, int inst_id, int ch, int vec);
int elb_mx_get_an_int(int chip_id, int inst_id, int ch, bool clear_on_read=false, bool display=true);
void elb_mx_enable_an_int(int chip_id, int inst_id, int ch, int vec);
void elb_mx_clear_an_int(int chip_id, int inst_id, int ch, int vec);
int elb_mx_get_int_groups(int chip_id, int inst_id, bool display=true);
int elb_mx_get_int_sd(int chip_id, int inst_id, bool clear_on_read=false, bool display=true);
void elb_mx_enable_int_sd(int chip_id, int inst_id, int vec);
void elb_mx_disable_int_sd(int chip_id, int inst_id, int vec);
void elb_mx_clear_int_sd(int chip_id, int inst_id, int vec);
int elb_mx_get_int_mac(int chip_id, int inst_id, bool clear_on_read=false, bool display=true);
void elb_mx_enable_int_mac(int chip_id, int inst_id, int vec);
void elb_mx_disable_int_mac(int chip_id, int inst_id, int vec);
void elb_mx_clear_int_mac(int chip_id, int inst_id, int vec);
int elb_mx_get_int_ecc(int chip_id, int inst_id, bool clear_on_read=false, bool display=true);
void elb_mx_enable_int_ecc(int chip_id, int inst_id, int vec);
void elb_mx_disable_int_ecc(int chip_id, int inst_id, int vec);
void elb_mx_clear_int_ecc(int chip_id, int inst_id, int vec);
int elb_mx_get_int_fixer(int chip_id, int inst_id, bool clear_on_read=false, bool display=true);
void elb_mx_enable_int_fixer(int chip_id, int inst_id, int vec);
void elb_mx_disable_int_fixer(int chip_id, int inst_id, int vec);
void elb_mx_clear_int_fixer(int chip_id, int inst_id, int vec);
int elb_mx_get_int_flush(int chip_id, int inst_id, bool clear_on_read=false, bool display=true);
void elb_mx_enable_int_flush(int chip_id, int inst_id, int vec);
void elb_mx_disable_int_flush(int chip_id, int inst_id, int vec);
void elb_mx_clear_int_flush(int chip_id, int inst_id, int vec);
void elb_mx_get_int(int chip_id, int inst_id, bool clear_on_read=false);
void elb_mx_enable_int(int chip_id, int inst_id);
void elb_mx_clear_int(int chip_id, int inst_id);

void elb_mx_dump_mx_cfg(int chip_id, int inst_id);
void elb_mx_dump_ch_cfg(int chip_id, int inst_id, int ch);

int elb_mx_an_get_page_addr(int ch, int tx);
void elb_mx_an_clean_rx_page(int chip_id, int inst_id, int ch);
uint64_t elb_mx_an_get_rx_page(int chip_id, int inst_id, int ch, int page_idx);
uint64_t elb_mx_an_get_rx_base_page(int chip_id, int inst_id, int ch, bool display=true);
void elb_mx_an_write_tx_page(int chip_id, int inst_id, int ch, int idx, uint64_t page);
int elb_mx_an_write_tx_page(int chip_id, int inst_id, int ch, uint64_t *pages);
void elb_mx_an_reset(int chip_id, int inst_id, int ch, int value);
void elb_mx_an_enable(int chip_id, int inst_id, int ch, int value);
void elb_mx_an_ignore_nonce(int chip_id, int inst_id, int ch, int value);
void elb_mx_an_set_breaklinktimer(int chip_id, int inst_id, int ch, uint32_t value);
void elb_mx_an_init(int chip_id, int inst_id, int ch, int user_cap=0, int user_fec=0, int user_pause=0, int ignore_nonce=0, uint32_t breaklinktimer=0);
uint64_t elb_mx_get_ansts(int chip_id, int inst_id, int ch, bool display=true);
bool elb_mx_an_get_an_complete(int chip_id, int inst_id, int ch);
uint32_t elb_mx_an_get_hcd(int chip_id, int inst_id, int ch);
int elb_mx_an_get_25g_fcfec(int chip_id, int inst_id, int ch);
int elb_mx_an_get_25g_rsfec(int chip_id, int inst_id, int ch);

void cap_bx_set_glbl_mode(int chip_id, int inst_id, int value);
void cap_bx_apb_write(int chip_id, int inst_id, int addr, int data);
void cap_bx_set_tx_rx_enable(int chip_id, int inst_id, int value);
void cap_bx_set_mtu(int chip_id , int inst_id, int max_value, int jabber_value);
void cap_bx_set_ch_enable(int chip_id, int inst_id, int value);
void cap_bx_set_soft_reset(int chip_id, int inst_id, int value);
int cap_bx_check_sync(int chip_id, int inst_id);
void cap_bx_mac_stat(int chip_id, int inst_id, int ch, int short_report,
                     uint64_t *stats_data);
int cap_bx_tx_drain(int chip_id, int inst_id, int mac_ch, bool drain);
void cap_bx_stats_reset(int chip_id, int inst_id, int ch, int value);

#endif
