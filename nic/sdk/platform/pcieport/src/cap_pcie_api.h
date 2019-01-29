#ifndef CAP_PCIE_API_H
#define CAP_PCIE_API_H

#ifndef CAPRI_SW
#include "cpu.h"
#endif
#include "cap_base_api.h"
#ifndef CAPRI_SW
#include "cap_blk_reg_model.h"
#include "cap_pxb_decoders.h"
#include "cap_pxb_model_decoders.h"
#include "cap_pp_csr.h"
#include "cap_pxb_csr.h"
#endif

#define CAP_PCIE_API_KEY_TYPE_CFG 0
#define CAP_PCIE_API_KEY_TYPE_MEM 1
#define CAP_PCIE_API_RESOURCE_PRT 0
#ifndef CAPRI_SW
#ifndef SWIG
class cap_pcie_port_db {
        cap_pcie_port_db(int);
        static map<int, cap_pcie_port_db*> port_db_ptr_map;
        map< int, map<string, uint32_t> > port_values;
        string apci_test_name;
        int logical_ports, chip_id;
    public:

        static cap_pcie_port_db* access(int chip_id) {
            for(auto i : port_db_ptr_map) {
                if(i.first == chip_id) {
                    return i.second;
                }
            }

            port_db_ptr_map[chip_id] = new cap_pcie_port_db(chip_id);
            return port_db_ptr_map[chip_id];
        }

        virtual ~cap_pcie_port_db() {}

        void set_port_values(int,string, uint32_t);
        uint32_t get_port_values(int,string);
        string get_apci_test_name();
        void populate_sknobs_data();
        int get_logical_ports();
        int logical_to_phy_convert(int);
        int phy_to_logical_convert(int);

};

typedef enum e_pcie_api_mode
{
    PCIE_API_PORT_x1 = 0,
    PCIE_API_PORT_x2,
    PCIE_API_PORT_x4,
    PCIE_API_PORT_x8,
    PCIE_API_PORT_x16
} pcie_api_mode_t;
#endif

// soft reset sequence 
void cap_pcie_soft_reset(int chip_id, int inst_id, string hint="all");
// soft reset, value = 1 means apply reset, value = 0 release soft reset
void cap_pcie_set_soft_reset(int chip_id, int inst_id, int value);
// init start, no polling yet
void cap_pcie_init_start(int chip_id, int inst_id, int cur_l_port=-1);
// poll for init done
void cap_pcie_init_done(int chip_id, int inst_id, string hint="all", int cur_l_port=-1);
// use sknobs base load cfg 
void cap_pcie_load_from_cfg(int chip_id, int inst_id);
// eos check, this functions should call eos_cnt, eos_int, eos_sta 
void cap_pcie_eos(int chip_id, int inst_id);
void cap_pcie_eos_cnt(int chip_id, int inst_id);
void cap_pcie_eos_int(int chip_id, int inst_id);
void cap_pcie_eos_sta(int chip_id, int inst_id);
void cap_pcie_init_sbus(int chip_id, int inst_id, int l_port);
void cap_pcie_tcam_rst(int chip_id, int inst_id, int val);
void cap_pcie_block_level_setup(int chip_id, int inst_id);
void pcie_program_tgt_debug_catch_all(int chip_id, int inst_id, int port, int tcam_idx, int prt_base, int flags);
void inject_raw_config_wr(int chip_id, int inst_id, unsigned port_id, uint32_t addr, uint32_t data);
void pcie_core_interrupts (int chip_id, int inst_id, int int_code, int int_data, int l_port);
#endif

#ifndef CAPRI_SW
int cap_pcie_serdes_setup(int chip_id, int inst_id, int gen1, const void *rom_info);
#else
int cap_pcie_serdes_setup(int chip_id, int inst_id, int gen1, void *rom_info);
int cap_pcie_serdes_reset(int chip_id, int inst_id);
#endif


#if 0
void cap_pcie_program_top_level_setup(int chip_id, int inst_id);
#endif

//bool cap_pcie_intf_count_check(int chip_id, int inst_id, const vector<cpp_int> & in_count,  vector <cpp_int> & out_count);


//void cap_pcie_set_csr_inst_path(int chip_id, int inst_id, string path);
void pcie_serdes_init(int chip_id, int inst_id);
void pcie_pcs_reset (int chip_id, int inst_id, int l_port); 
void pcie_per_port_phystatus_poll(int chip_id, int inst_id, int port_no, int mode);
#ifndef CAPRI_SW
void pcie_per_port_mac_k_cfg(int chip_id, int inst_id, int port_no,string hint);
void pcie_per_port_mac_cfg(int chip_id, int inst_id, int port_no, string hint);
#endif

void cap_pcie_base_test(int chip_id, int inst_id, uint32_t val);

#ifndef CAPRI_SW
#ifndef SWIG
void cap_pcie_program_tcam_sram_pair(int chip_id, int inst_id, uint32_t idx, 
        cap_pxb_csr_dhs_tgt_pmr_entry_t _sram_entry,
        cap_pxb_csr_dhs_tgt_pmt_entry_t _tcam_entry);


void cap_pcie_program_key_type_cfg_tcam_sram_pair(int chip_id, int inst_id, uint32_t idx, 
        cap_pxb_decoders_pmr_key_type_cfg_t _sram,
        cap_pxb_decoders_pmt_tcam_key_type_cfg_entry_t _tcam);

void cap_pcie_program_key_type_memio_tcam_sram_pair(int chip_id, int inst_id, uint32_t idx, 
        cap_pxb_decoders_pmr_key_type_memio_t _sram,
        cap_pxb_decoders_pmt_tcam_key_type_memio_entry_t _tcam);
void cap_pcie_program_key_type_rcdma_tcam_sram_pair(int chip_id, int inst_id, uint32_t idx, 
        cap_pxb_decoders_pmr_key_type_memio_t _sram,
        cap_pxb_decoders_pmt_tcam_key_type_rcdma_entry_t _tcam);

void cap_pcie_program_prt_type_memio_sram(int chip_id, int inst_id, uint32_t idx,
        cap_pxb_decoders_prt_memio_t _sram);
void cap_pcie_program_prt_type_db_sram(int chip_id, int inst_id, uint32_t idx,
        cap_pxb_decoders_prt_db_t _sram);

unsigned cap_pcie_read_indirect_req(int chip_id, int inst_id, int port,cap_pxb_target_indirect_info_t  & info);
void cap_pcie_process_indirect_req(int chip_id, int inst_id, int port, unsigned axi_id, vector<uint32_t> dw, unsigned cpl_stat);
void cap_pcie_retry_en(int chip_id, int inst_id, int val);
void cap_pcie_per_port_retry_en(int chip_id, int inst_id, int port_no, int val);
void cap_pcie_disable_backdoor_paths(int chip_id, int inst_id);

#ifndef CAPRI_SW
void cap_pxb_csr_set_hdl_path(int chip_id, int inst_id, string path);
void cap_pp_csr_set_hdl_path(int chip_id, int inst_id, string path);
#endif
void inject_raw_tlp(int chip_id, int inst_id, vector<uint32_t> & hdr_array, vector<uint32_t> & payload_array, unsigned port_id, bool poll_for_finish);
bool is_pcie_raw_tlp_ready(int chip_id, int inst_id);
void poll_done_raw_tlp(int chip_id, int inst_id);
unsigned pcie_query_port_val(int chip_id, int inst_id, unsigned lif);


class cap_pcie_atomic_db {
        cap_pcie_atomic_db(int);
        static map<int, cap_pcie_atomic_db*> atomic_db_ptr_map;
        vector< std::shared_ptr< cap_pxb_decoders_itr_atomicop_req_t> > atomic_req_array;
        vector< int > atomic_state;
        unsigned max_atomic_context;


        int chip_id;
    public:

        typedef enum e_pcie_api_mode
        {
            ATOMIC_IDLE = 0,
            ATOMIC_ALLOC,
            ATOMIC_WR,
            ATOMIC_RD,
        } atomic_db_enum;

        static cap_pcie_atomic_db* access(int chip_id) {
            for(auto i : atomic_db_ptr_map) {
                if(i.first == chip_id) {
                    return i.second;
                }
            }

            atomic_db_ptr_map[chip_id] = new cap_pcie_atomic_db(chip_id);
            return atomic_db_ptr_map[chip_id];
        }

        virtual ~cap_pcie_atomic_db() {}

        int generate_atomic_xn(uint64_t host_addr, int atomic_type, unsigned tlp_length, vector<uint32_t> dw);
        std::shared_ptr<cap_pxb_decoders_itr_atomicop_req_t> get_atomic_wr_req(int id);
        std::shared_ptr<cap_pxb_decoders_itr_atomicop_req_t> get_atomic_rd_req(int id);
        void set_atomic_rd_rsp(int id);
        void set_max_atomic_context(unsigned max_id);
};
#endif

void cap_pcie_serdes_setup_wrapper(int chip_id, int inst_id, int gen1, string rom_info);
#endif
void cap_pcie_toggle_pcs_reset(int chip_id, int inst_id, int l_port);
int cap_pcie_complete_serdes_initialization(int chip_id, int inst_id);

void  pcie_enable_interrupts(void);
void cap_pp_set_rom_enable(int chip_id, int inst_id, int val); 
void cap_pcie_bist_test(int chip_id, int inst_id); 
#ifndef CAPRI_SW
int  cap_run_pcie_tcam_rdwr_test(int chip_id, int inst_id, int fast_mode = 1, int verbosity = 0);
int  cap_run_pcie_tcam_rdwr_test_NEW(int chip_id, int inst_id, int fast_mode = 0, int verbosity = 1);
int cap_pcie_setup_pll_raw(void);
void cap_pcie_set_port_values(int chip_id, int l_port, string name, int data);
#endif
#ifdef _COSIM_
void cap_pcie_program_vnic_qspi();
void cap_pcie_program_vnic_asic();
void cap_pcie_program_vnic_asic_arm();
#endif

void cap_pcie_program_vnic_entries(int chip_id, int inst_id, uint64_t bar , uint64_t capri_address, int port, int tcam_idx, int bar_size, int prt_base, int prt_count);
#endif // CAP_PCIE_API_H
