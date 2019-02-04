#include "LogMsg.h" 
#include "pen_csr_base.h"
#include "cap_top_csr.h"
using namespace std;

void cap_top_quiesce_txs_start(int chip_id) {
	PLOG_MSG("cap_top_quiesce_txs_start starts" << endl)
	cap_top_csr_t & top_csr= CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
	top_csr.txs.txs.cfg_sch.pause(1);
	top_csr.txs.txs.cfg_sch.write();
	PLOG_MSG("cap_top_quiesce_txs_start exits" << endl)
} 

void cap_top_quiesce_sw_phv_insert(int chip_id) {
	PLOG_MSG("cap_top_quiesce_sw_phv_insert starts" << endl)
	cap_top_csr_t & top_csr= CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);

    	const uint32_t NUM_PSP = sizeof(top_csr.pr.pr.psp.cfg_sw_phv_control) / sizeof(top_csr.pr.pr.psp.cfg_sw_phv_control[0]);

	for (uint32_t i=0; i<NUM_PSP; i++) {
		top_csr.pr.pr.psp.cfg_sw_phv_control[i].read();
		if (top_csr.pr.pr.psp.cfg_sw_phv_control[i].start_enable()) {
			top_csr.pr.pr.psp.cfg_sw_phv_control[i].start_enable(0);
			top_csr.pr.pr.psp.cfg_sw_phv_control[i].write();

			bool done;
			uint32_t max_tries = 100;

			do {
				top_csr.pr.pr.psp.sta_sw_phv_state[i].read();
				done = top_csr.pr.pr.psp.sta_sw_phv_state[i].done().convert_to<uint32_t>();

				PLOG_MSG("cap_top_quiesce_sw_phv_insert i=" << i 
					<< " done=" << (done ? 1 : 0)
					<< endl
					);

			} while (!done && max_tries--);

			if (!done) {
				PLOG_MSG("cap_top_quiesce_sw_phv_insert timed out polling for pr " << i << " done" << endl)
			} else {
				PLOG_MSG("cap_top_quiesce_sw_phv_insert pr poll done" << endl)
			}
		}
	}

    	const uint32_t NUM_NPV = sizeof(top_csr.pt.pt.psp.cfg_sw_phv_control) / sizeof(top_csr.pt.pt.psp.cfg_sw_phv_control[0]);

	for (uint32_t i=0; i<NUM_NPV; i++) {

		top_csr.pt.pt.psp.cfg_sw_phv_control[i].read();
		if (top_csr.pt.pt.psp.cfg_sw_phv_control[i].start_enable().convert_to<uint32_t>()) {
			top_csr.pt.pt.psp.cfg_sw_phv_control[i].start_enable(0);
			top_csr.pt.pt.psp.cfg_sw_phv_control[i].write();

			bool done;
			uint32_t max_tries = 100;

			do {
				top_csr.pt.pt.psp.sta_sw_phv_state[i].read();
				done = top_csr.pt.pt.psp.sta_sw_phv_state[i].done().convert_to<uint32_t>();

				PLOG_MSG("cap_top_quiesce_sw_phv_insert pt i=" << i 
					<< " done=" << (done ? 1 : 0)
					<< endl
					);

			} while (!done && max_tries--);

			if (!done) {
				PLOG_MSG("cap_top_quiesce_sw_phv_insert timed out polling for pt " << i << " done" << endl)
			} else {
				PLOG_MSG("cap_top_quiesce_sw_phv_insert pt poll done" << endl)
			}
		}
	}

    	const uint32_t NUM_PPA = sizeof(top_csr.ppa.ppa) / sizeof(top_csr.ppa.ppa[0]);
	const uint32_t NUM_CTRL = sizeof(top_csr.ppa.ppa[0].cfg_sw_phv_control) / sizeof(top_csr.ppa.ppa[0].cfg_sw_phv_control[0]);

	for (uint32_t i=0; i<NUM_PPA; i++) {

		for (uint32_t j=0; j<NUM_CTRL; j++) {
			top_csr.ppa.ppa[i].cfg_sw_phv_control[j].read();

			if (top_csr.ppa.ppa[i].cfg_sw_phv_control[j].start_enable()) {

				top_csr.ppa.ppa[i].cfg_sw_phv_control[j].start_enable(0);
				top_csr.ppa.ppa[i].cfg_sw_phv_control[j].write();

				bool done;
				uint32_t max_tries = 100;

				do {
					top_csr.ppa.ppa[i].sta_sw_phv_state[j].read();
					done = top_csr.ppa.ppa[i].sta_sw_phv_state[j].done().convert_to<uint32_t>();

					PLOG_MSG("cap_top_quiesce_sw_phv_insert ppa i=" << i 
						<< " ctrl " << j 
						<< " done=" << (done ? 1 : 0)
						<< endl
						);

				} while (!done && max_tries--);

				if (!done) {
					PLOG_MSG("cap_top_quiesce_sw_phv_insert timed out polling for ppa " << i << " ctrl " << j << " done" << endl)
				} else {
					PLOG_MSG("cap_top_quiesce_sw_phv_insert ppa poll done" << endl)
				}
			}
		}
	}

	PLOG_MSG("cap_top_quiesce_sw_phv_insert exits" << endl)
}

// flush all the packets going to p4 ingress and p4 egress, except for recircs which
// need to go thru to release the credits
void cap_top_quiesce_pb_start(int chip_id) {
	PLOG_MSG("cap_top_quiesce_pb_start starts" << endl)
	cap_top_csr_t & top_csr= CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);

	// Ingress: disable all output queues 
	top_csr.pb.pbc.port_11.cfg_oq.enable(0);
	top_csr.pb.pbc.port_11.cfg_oq.write();

	// no need to disable ethernet output ports (0-8), they'll drain just fine

	PLOG_MSG("cap_top_quiesce_pb_start exits" << endl)
}

void cap_top_quiesce_txs_stop(int chip_id) {
	PLOG_MSG("cap_top_quiesce_txs_stop starts" << endl)
	cap_top_csr_t & top_csr= CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
	top_csr.txs.txs.cfg_sch.pause(0);
	top_csr.txs.txs.cfg_sch.write();
	PLOG_MSG("cap_top_quiesce_txs_stop exits" << endl)
} 

void cap_top_quiesce_pb_stop(int chip_id) {
	PLOG_MSG("cap_top_quiesce_pb_stop starts" << endl)
	cap_top_csr_t & top_csr= CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);

	// Ingress: enable all output queues 
	top_csr.pb.pbc.port_11.cfg_oq.enable(1);
	top_csr.pb.pbc.port_11.cfg_oq.write();

	PLOG_MSG("cap_top_quiesce_pb_stop exits" << endl)
}


void cap_top_quiesce_txs_poll(int chip_id, const int max_tries=100) {
	PLOG_MSG("cap_top_quiesce_txs_poll starts" << endl)
	cap_top_csr_t & top_csr= CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
	const int inactivity = 2;
	uint32_t last_cnt = 0;
	int tries = inactivity;
	int give_up = max_tries;
	do {
		uint32_t cnt = 0;
		top_csr.txs.txs.cnt_sch_txdma_cos0.read();
		cnt += top_csr.txs.txs.cnt_sch_txdma_cos0.val().convert_to<uint32_t>();
		top_csr.txs.txs.cnt_sch_txdma_cos1.read();
		cnt += top_csr.txs.txs.cnt_sch_txdma_cos1.val().convert_to<uint32_t>();
		top_csr.txs.txs.cnt_sch_txdma_cos2.read();
		cnt += top_csr.txs.txs.cnt_sch_txdma_cos2.val().convert_to<uint32_t>();
		top_csr.txs.txs.cnt_sch_txdma_cos3.read();
		cnt += top_csr.txs.txs.cnt_sch_txdma_cos3.val().convert_to<uint32_t>();
		top_csr.txs.txs.cnt_sch_txdma_cos4.read();
		cnt += top_csr.txs.txs.cnt_sch_txdma_cos4.val().convert_to<uint32_t>();
		top_csr.txs.txs.cnt_sch_txdma_cos5.read();
		cnt += top_csr.txs.txs.cnt_sch_txdma_cos5.val().convert_to<uint32_t>();
		top_csr.txs.txs.cnt_sch_txdma_cos6.read();
		cnt += top_csr.txs.txs.cnt_sch_txdma_cos6.val().convert_to<uint32_t>();
		top_csr.txs.txs.cnt_sch_txdma_cos7.read();
		cnt += top_csr.txs.txs.cnt_sch_txdma_cos7.val().convert_to<uint32_t>();
		top_csr.txs.txs.cnt_sch_txdma_cos8.read();
		cnt += top_csr.txs.txs.cnt_sch_txdma_cos8.val().convert_to<uint32_t>();
		top_csr.txs.txs.cnt_sch_txdma_cos9.read();
		cnt += top_csr.txs.txs.cnt_sch_txdma_cos9.val().convert_to<uint32_t>();
		top_csr.txs.txs.cnt_sch_txdma_cos10.read();
		cnt += top_csr.txs.txs.cnt_sch_txdma_cos10.val().convert_to<uint32_t>();
		top_csr.txs.txs.cnt_sch_txdma_cos11.read();
		cnt += top_csr.txs.txs.cnt_sch_txdma_cos11.val().convert_to<uint32_t>();
		top_csr.txs.txs.cnt_sch_txdma_cos12.read();
		cnt += top_csr.txs.txs.cnt_sch_txdma_cos12.val().convert_to<uint32_t>();
		top_csr.txs.txs.cnt_sch_txdma_cos13.read();
		cnt += top_csr.txs.txs.cnt_sch_txdma_cos13.val().convert_to<uint32_t>();
		top_csr.txs.txs.cnt_sch_txdma_cos14.read();
		cnt += top_csr.txs.txs.cnt_sch_txdma_cos14.val().convert_to<uint32_t>();
		top_csr.txs.txs.cnt_sch_txdma_cos15.read();
		cnt += top_csr.txs.txs.cnt_sch_txdma_cos15.val().convert_to<uint32_t>();
		if (cnt != last_cnt) {
			tries = inactivity;
		}
		last_cnt = cnt;
		PLOG_MSG("cap_top_quiesce_txs_poll count=" << cnt << " tries=" << tries << endl)
	} while (tries-- && give_up--);

	if (give_up > 0) {
		PLOG_MSG("cap_top_quiesce_txs_poll exits " << endl)
	} else {
		PLOG_ERR("cap_top_quiesce_txs_poll timed out polling for stable cnt" << endl);
	}
} 

// pb polls for credits to detect quiescence. in order to do this it needs to know the number of credits given to each OQ originally.
// only the programmer knows that, so he/she needs to pass that information to the polling function
void cap_top_quiesce_pb_poll(int chip_id, const uint32_t* port_10_orig_credits, const uint32_t* port_11_orig_credits, int max_tries=100) {
	PLOG_MSG("cap_top_quiesce_pb_poll starts" << endl)
	cap_top_csr_t & top_csr= CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);

	bool idle;

	do {
		idle = true;

		// port 10: check all credits match original programming
		for (int i=0; i<top_csr.pb.pbc.port_10.dhs_oq_flow_control.get_depth_entry(); i++) {
			top_csr.pb.pbc.port_10.dhs_oq_flow_control.entry[i].read();
			idle = idle && (top_csr.pb.pbc.port_10.dhs_oq_flow_control.entry[i].entry().convert_to<uint32_t>() == *(port_10_orig_credits+i));

			PLOG_MSG("cap_top_quiesce_pb_poll port 10, entry " << i 
				<< " got=" << top_csr.pb.pbc.port_10.dhs_oq_flow_control.entry[i].entry().convert_to<uint32_t>() 
				<< " exp=" << *(port_10_orig_credits+i)
				<< endl
				);
		}

		// port 11: check all credits match original programming
		for (int i=0; i<top_csr.pb.pbc.port_11.dhs_oq_flow_control.get_depth_entry(); i++) {
			top_csr.pb.pbc.port_11.dhs_oq_flow_control.entry[i].read();
			idle = idle && (top_csr.pb.pbc.port_11.dhs_oq_flow_control.entry[i].entry().convert_to<uint32_t>() == *(port_11_orig_credits+i));

			PLOG_MSG("cap_top_quiesce_pb_poll port 11, entry " << i 
				<< " got=" << top_csr.pb.pbc.port_11.dhs_oq_flow_control.entry[i].entry().convert_to<uint32_t>() 
				<< " exp=" << *(port_11_orig_credits+i)
				<< endl
				);
		}

		// RXDMA: make sure sop/eop match. assume at least 14 cycles from queue disable
		top_csr.pb.pbc.cnt_flits[9].read();
		idle = idle && (top_csr.pb.pbc.cnt_flits[9].sop_out().convert_to<uint32_t>() == top_csr.pb.pbc.cnt_flits[9].eop_out().convert_to<uint32_t>());

		PLOG_MSG("cap_top_quiesce_pb_poll port 9 "
			<< " sop=" << top_csr.pb.pbc.cnt_flits[9].sop_out().convert_to<uint32_t>() 
			<< " eop=" << top_csr.pb.pbc.cnt_flits[9].eop_out().convert_to<uint32_t>()
			<< endl
			);

	} while (!idle && max_tries--);

	if (!idle) {
		PLOG_ERR("cap_top_quiesce_pb_poll timed out polling for idle" << endl);
	} else {
		PLOG_MSG("cap_top_quiesce_pb_poll exits"<<endl);
	}
}

void cap_top_quiesce_psp_poll(int chip_id, int max_tries=100) {
	PLOG_MSG("cap_top_quiesce_psp_poll starts" << endl)
	cap_top_csr_t & top_csr= CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);

	bool idle;

	do { 
		top_csr.pr.pr.psp.sta.read();

		idle = top_csr.pr.pr.psp.sta.empty().convert_to<uint32_t>()
		    && top_csr.pr.pr.psp.sta.pkt_empty().convert_to<uint32_t>()
		    && top_csr.pr.pr.psp.sta.phv_empty().convert_to<uint32_t>()
		    ;

	} while (!idle && max_tries--);

	if (!idle) {
		PLOG_ERR("cap_top_quiesce_psp_poll timed out polling for idle" << endl);
	} else {
		PLOG_MSG("cap_top_quiesce_psp_poll exits" << endl);
	}
}

void cap_top_quiesce_p4p_prd_poll(int chip_id, int max_tries=100) {
	PLOG_MSG("cap_top_quiesce_p4p_prd_poll starts" << endl)
	bool idle;
	cap_top_csr_t & top_csr= CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);

	do { 

	   top_csr.pr.pr.prd.sta_xoff.read();
	   idle = top_csr.pr.pr.prd.sta_xoff.numphv_xoff().convert_to<uint32_t>() == 0;

	} while (!idle && max_tries--);

	if (!idle) {
		PLOG_ERR("cap_top_quiesce_p4p_prd_poll timed out polling for idle" << endl);
	} else {
		PLOG_MSG("cap_top_quiesce_p4p_prd_poll exits." << endl); 
	}
}

void cap_top_quiesce_p4p_tx_poll(int chip_id, int max_tries=100) {
	PLOG_MSG("cap_top_quiesce_p4p_tx_poll starts" << endl)
	bool idle;
	cap_top_csr_t & top_csr= CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);
	const uint32_t NUM_MPUS= sizeof(top_csr.pct.mpu)/sizeof(top_csr.pct.mpu[0]);

	do { 

	   top_csr.pct.te[0].cnt_phv_in_sop.read();
	   top_csr.pct.mpu[NUM_MPUS-1].CNT_sdp.read();
	   for (int i=0; i<NUM_MPUS; i++) {
		   top_csr.pct.mpu[i].STA_stg.read();
	   }

	   idle = top_csr.pct.te[0].cnt_phv_in_sop.val().convert_to<uint32_t>()
	   	  ==
	   	  top_csr.pct.mpu[NUM_MPUS-1].CNT_sdp.eop_out().convert_to<uint32_t>() 
		  ;
	   for (int i=0; i<NUM_MPUS; i++) {
	   	idle = idle && (top_csr.pct.mpu[i].STA_stg.mpu_processing().convert_to<uint32_t>()==0);
	   }

	} while (!idle && max_tries--);

	if (!idle) {
		PLOG_ERR("cap_top_quiesce_p4p_tx_poll timed out polling for idle" << endl);
	} else {
		PLOG_MSG("cap_top_quiesce_p4p_tx_poll exits." << endl); 
	}
}

void cap_top_quiesce_continue_test(int chip_id) {
	PLOG_MSG("cap_top_int_quiesce_test starts" << endl)
	cap_top_csr_t & top_csr= CAP_BLK_REG_MODEL_ACCESS(cap_top_csr_t, 0, 0);

	uint32_t port_10_orig_credits[top_csr.pb.pbc.port_10.dhs_oq_flow_control.get_depth_entry()];
	uint32_t port_11_orig_credits[top_csr.pb.pbc.port_11.dhs_oq_flow_control.get_depth_entry()];

	// prepare original programmed credit tables
	for (int i=0; i<top_csr.pb.pbc.port_10.dhs_oq_flow_control.get_depth_entry(); i++) {
		port_10_orig_credits[i] = top_csr.pb.pbc.port_10.dhs_oq_flow_control.entry[i].entry().convert_to<uint32_t>();
	}
	for (int i=0; i<top_csr.pb.pbc.port_11.dhs_oq_flow_control.get_depth_entry(); i++) {
		port_11_orig_credits[i] = top_csr.pb.pbc.port_11.dhs_oq_flow_control.entry[i].entry().convert_to<uint32_t>();
	}

	cap_top_quiesce_pb_start(chip_id); 
	cap_top_quiesce_txs_start(chip_id);
	cap_top_quiesce_sw_phv_insert(chip_id);

	PLOG_MSG("cap_top_int_quiesce_test polling starts" << endl)

	cap_top_quiesce_txs_poll(chip_id);
	cap_top_quiesce_p4p_tx_poll(chip_id);
	cap_top_quiesce_pb_poll(chip_id, &port_10_orig_credits[0], &port_11_orig_credits[0]);
	cap_top_quiesce_psp_poll(chip_id);
	cap_top_quiesce_p4p_prd_poll(chip_id);

	SLEEP(1000); // wait for some time

	cap_top_quiesce_pb_stop(chip_id); 
	cap_top_quiesce_txs_stop(chip_id);

	PLOG_MSG("cap_top_int_quiesce_test ends" << endl)
}
