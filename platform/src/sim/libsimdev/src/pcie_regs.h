#ifndef _PCIE_REGS_H_
#define _PCIE_REGS_H_

#define PCIE_REG_NAME_MAX_SIZE  32

#define PCIE_BAR_TYPE_IO        0
#define PCIE_BAR_TYPE_MEM       1

#define PCIE_REG_ACCESS_TYPE_R          1
#define PCIE_REG_ACCESS_TYPE_W          2
#define PCIE_REG_ACCESS_TYPE_INDEXED    8

struct pcie_reg_s;

struct pcie_bar_s;

typedef int (*reg_read_cb_t) (simdev_t *sd, struct pcie_bar_s *bar, struct pcie_reg_s *reg, uint64_t *reg_val);
typedef int (*reg_write_cb_t) (simdev_t *sd, struct pcie_bar_s *bar, struct pcie_reg_s *reg, uint64_t reg_val);

typedef struct pcie_reg_def_s {
    uint8_t         reg_name[PCIE_REG_NAME_MAX_SIZE];
    uint8_t         reg_size;   /* Bytes */
    uint16_t        reg_offset;
    uint8_t         reg_access_type; /* PCIE_REG_ACCESS_TYPE* bitmap */
    union {
                    uint8_t     initreg8;
                    uint16_t    initreg16;
                    uint32_t    initreg32;
                    uint64_t    initreg64;
    };
    reg_read_cb_t   reg_read_cb;
    reg_write_cb_t  reg_write_cb;
} pcie_reg_def_t;

typedef struct pcie_reg_s {
    pcie_reg_def_t  *reg_def;
    uint16_t        reg_idx_count;
    union {
                    uint8_t     reg8;
                    uint16_t    reg16;
                    uint32_t    reg32;
                    uint64_t    reg64;
                    uint64_t    *reg_array; /* for PCIE_REG_ACCESS_TYPE_INDEXED */
    };
} pcie_reg_t;

typedef struct pcie_bar_s {
    uint8_t         bar_number;
    uint8_t         bar_type;   /* PCIE_BAR_TYPE_* */
    uint16_t        reg_min_offset;
    uint16_t        reg_max_offset;
    pcie_reg_t      *reg_list;
} pcie_bar_t;

#endif  /* _PCIE_REGS_H_ */
