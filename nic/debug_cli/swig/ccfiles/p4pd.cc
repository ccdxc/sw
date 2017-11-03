#include <stdio.h>
#include <stdint.h>
#include "nic/gen/iris/include/p4pd.h"

char p4pd_tbl_names[P4TBL_ID_TBLMAX][P4TBL_NAME_MAX_LEN];
uint16_t p4pd_tbl_swkey_size[P4TBL_ID_TBLMAX];
uint16_t p4pd_tbl_sw_action_data_size[P4TBL_ID_TBLMAX];

uint32_t get_table_id (char *table_name)
{
    return 0x1;
}

void dump_table(uint32_t table_id, egress_policer_actiondata *epad_p)
{
    printf("Action ID: %d, entry_valid: %d\n",
           epad_p->actionid,
           epad_p->egress_policer_action_u.egress_policer_execute_egress_policer.entry_valid);
    return;
}

char* get_data(egress_policer_actiondata *epad_p)
{
    return (char*)epad_p;
}
