
#ifndef __LIB_DRIVER__
#define __LIB_DRIVER__

void hal_create_mr_wrapper(struct create_mr_cmd  *cmd,
                           struct create_mr_comp *comp,
                           u_int32_t             *done);
void hal_create_cq_wrapper(struct create_cq_cmd  *cmd,
                           struct create_cq_comp *comp,
                           u_int32_t             *done);
void hal_create_qp_wrapper(struct create_qp_cmd  *cmd,
                           struct create_qp_comp *comp,
                           u_int32_t             *done);
void hal_modify_qp_wrapper(struct modify_qp_cmd *cmd,
                           struct modify_qp_comp *comp,
                           u_int32_t             *done);
void hal_set_lif_base(const u_int32_t lif);
void init_lib_driver(void);
#endif
