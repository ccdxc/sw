TOPDIR = ..
CUSTOM = all
CUSTOM_CLEAN = clean

include $(TOPDIR)/make/Makefile.inc

OBJ_DIR = ../../obj
GEN_DIR = ../../gen
OUTPUT_SKNOBS = output.sknobs
PYTHON = python
# Files below must come from the asic repo
JSON_TO_SKNOBS = json_to_sknobs.py
CAP_PPA_DECODERS = cap_ppa_decoders.yaml

all:
	$(PYTHON) $(JSON_TO_SKNOBS) -r ../gen/iris/cfg_out/cap_dpp_INGRESS_cfg_reg.json -p 'cap0/dpp/dpp/1/' > $(OUTPUT_SKNOBS)
	$(PYTHON) $(JSON_TO_SKNOBS) -r ../gen/iris/cfg_out/cap_dpp_EGRESS_cfg_reg.json -p 'cap0/dpp/dpp/0/' >> $(OUTPUT_SKNOBS)
	$(PYTHON) $(JSON_TO_SKNOBS) -r ../gen/iris/cfg_out/cap_dpr_INGRESS_cfg_reg.json -p 'cap0/dpr/dpr/1/' >> $(OUTPUT_SKNOBS)
	$(PYTHON) $(JSON_TO_SKNOBS) -r ../gen/iris/cfg_out/cap_dpr_EGRESS_cfg_reg.json -p 'cap0/dpr/dpr/0/' >> $(OUTPUT_SKNOBS)
	$(PYTHON) $(JSON_TO_SKNOBS) -r ../gen/iris/cfg_out/cap_pics_INGRESS.json -p 'cap0/ssi/pics/' >> $(OUTPUT_SKNOBS)
	$(PYTHON) $(JSON_TO_SKNOBS) -r ../gen/iris/cfg_out/cap_pics_EGRESS.json -p 'cap0/sse/pics/' >> $(OUTPUT_SKNOBS)
	$(PYTHON) $(JSON_TO_SKNOBS) -r ../gen/iris/cfg_out/cap_pict_INGRESS.json -p 'cap0/tsi/pict/' >> $(OUTPUT_SKNOBS)
	$(PYTHON) $(JSON_TO_SKNOBS) -r ../gen/iris/cfg_out/cap_pict_EGRESS.json -p 'cap0/tse/pict/' >> $(OUTPUT_SKNOBS)
	$(PYTHON) $(JSON_TO_SKNOBS) -r ../gen/iris/cfg_out/cap_ppa_INGRESS_cfg_decoder_reg.json -d $(CAP_PPA_DECODERS) \
	    			 -m ../gen/iris/cfg_out/cap_ppa_INGRESS_cfg_decoder_mem.json \
	    			 -p 'cap0/ppa/ppa/1/' | sed -e 's/cap_ppa_csr_//g' >> $(OUTPUT_SKNOBS)
	$(PYTHON) $(JSON_TO_SKNOBS) -r ../gen/iris/cfg_out/cap_ppa_EGRESS_cfg_decoder_reg.json -d $(CAP_PPA_DECODERS) \
	    			 -m ../gen/iris/cfg_out/cap_ppa_EGRESS_cfg_decoder_mem.json \
				 -p 'cap0/ppa/ppa/0/' | sed -e 's/cap_ppa_csr_//g' >> $(OUTPUT_SKNOBS)
	$(PYTHON) $(JSON_TO_SKNOBS) -r ../gen/iris/cfg_out/cap_te_INGRESS_0_cfg_reg.json -p 'cap0/sgi/te/0/' >> $(OUTPUT_SKNOBS)
	$(PYTHON) $(JSON_TO_SKNOBS) -r ../gen/iris/cfg_out/cap_te_EGRESS_0_cfg_reg.json -p 'cap0/sge/te/0/' >> $(OUTPUT_SKNOBS)
	$(PYTHON) $(JSON_TO_SKNOBS) -r ../gen/iris/cfg_out/cap_te_INGRESS_1_cfg_reg.json -p 'cap0/sgi/te/1/' >> $(OUTPUT_SKNOBS)
	$(PYTHON) $(JSON_TO_SKNOBS) -r ../gen/iris/cfg_out/cap_te_EGRESS_1_cfg_reg.json -p 'cap0/sge/te/1/' >> $(OUTPUT_SKNOBS)
	$(PYTHON) $(JSON_TO_SKNOBS) -r ../gen/iris/cfg_out/cap_te_INGRESS_2_cfg_reg.json -p 'cap0/sgi/te/2/' >> $(OUTPUT_SKNOBS)
	$(PYTHON) $(JSON_TO_SKNOBS) -r ../gen/iris/cfg_out/cap_te_EGRESS_2_cfg_reg.json -p 'cap0/sge/te/2/' >> $(OUTPUT_SKNOBS)
	$(PYTHON) $(JSON_TO_SKNOBS) -r ../gen/iris/cfg_out/cap_te_INGRESS_3_cfg_reg.json -p 'cap0/sgi/te/3/' >> $(OUTPUT_SKNOBS)
	$(PYTHON) $(JSON_TO_SKNOBS) -r ../gen/iris/cfg_out/cap_te_EGRESS_3_cfg_reg.json -p 'cap0/sge/te/3/' >> $(OUTPUT_SKNOBS)
	$(PYTHON) $(JSON_TO_SKNOBS) -r ../gen/iris/cfg_out/cap_te_INGRESS_4_cfg_reg.json -p 'cap0/sgi/te/4/' >> $(OUTPUT_SKNOBS)
	$(PYTHON) $(JSON_TO_SKNOBS) -r ../gen/iris/cfg_out/cap_te_EGRESS_4_cfg_reg.json -p 'cap0/sge/te/4/' >> $(OUTPUT_SKNOBS)
	$(PYTHON) $(JSON_TO_SKNOBS) -r ../gen/iris/cfg_out/cap_te_INGRESS_5_cfg_reg.json -p 'cap0/sgi/te/5/' >> $(OUTPUT_SKNOBS)
	$(PYTHON) $(JSON_TO_SKNOBS) -r ../gen/iris/cfg_out/cap_te_EGRESS_5_cfg_reg.json -p 'cap0/sge/te/5/' >> $(OUTPUT_SKNOBS)

clean:
	rm -f $(OUTPUT_SKNOBS)

default:
	all
