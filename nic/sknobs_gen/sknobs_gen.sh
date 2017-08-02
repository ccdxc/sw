#!/bin/sh

rm -f output.sknobs

./json_to_sknobs.py -r ../gen/iris/cfg_out/cap_dpp_INGRESS_cfg_reg.json -p 'cap0/dpp/dpp/1/' >> output.sknobs
./json_to_sknobs.py -r ../gen/iris/cfg_out/cap_dpp_EGRESS_cfg_reg.json -p 'cap0/dpp/dpp/0/' >> output.sknobs
./json_to_sknobs.py -r ../gen/iris/cfg_out/cap_dpr_INGRESS_cfg_reg.json -p 'cap0/dpr/dpr/1/' >> output.sknobs
./json_to_sknobs.py -r ../gen/iris/cfg_out/cap_dpr_EGRESS_cfg_reg.json -p 'cap0/dpr/dpr/0/' >> output.sknobs
./json_to_sknobs.py -r ../gen/iris/cfg_out/cap_pics_INGRESS.json -p 'cap0/ssi/pics/' >> output.sknobs
./json_to_sknobs.py -r ../gen/iris/cfg_out/cap_pics_EGRESS.json -p 'cap0/sse/pics/' >> output.sknobs
./json_to_sknobs.py -r ../gen/iris/cfg_out/cap_pict_INGRESS.json -p 'cap0/tsi/pict/' >> output.sknobs
./json_to_sknobs.py -r ../gen/iris/cfg_out/cap_pict_EGRESS.json -p 'cap0/tse/pict/' >> output.sknobs

./json_to_sknobs.py -r ../gen/iris/cfg_out/cap_ppa_INGRESS_cfg_decoder_reg.json -d cap_ppa_decoders.yaml -m ../gen/iris/cfg_out/cap_ppa_INGRESS_cfg_decoder_mem.json -p 'cap0/ppa/ppa/1/' | sed -e 's/cap_ppa_csr_//g' >> output.sknobs
./json_to_sknobs.py -r ../gen/iris/cfg_out/cap_ppa_EGRESS_cfg_decoder_reg.json -d cap_ppa_decoders.yaml -m ../gen/iris/cfg_out/cap_ppa_EGRESS_cfg_decoder_mem.json -p 'cap0/ppa/ppa/0/' | sed -e 's/cap_ppa_csr_//g' >> output.sknobs

./json_to_sknobs.py -r ../gen/iris/cfg_out/cap_te_INGRESS_0_cfg_reg.json -p 'cap0/sgi/te/0/' >> output.sknobs
./json_to_sknobs.py -r ../gen/iris/cfg_out/cap_te_EGRESS_0_cfg_reg.json -p 'cap0/sge/te/0/' >> output.sknobs
./json_to_sknobs.py -r ../gen/iris/cfg_out/cap_te_INGRESS_1_cfg_reg.json -p 'cap0/sgi/te/1/' >> output.sknobs
./json_to_sknobs.py -r ../gen/iris/cfg_out/cap_te_EGRESS_1_cfg_reg.json -p 'cap0/sge/te/1/' >> output.sknobs
./json_to_sknobs.py -r ../gen/iris/cfg_out/cap_te_INGRESS_2_cfg_reg.json -p 'cap0/sgi/te/2/' >> output.sknobs
./json_to_sknobs.py -r ../gen/iris/cfg_out/cap_te_EGRESS_2_cfg_reg.json -p 'cap0/sge/te/2/' >> output.sknobs
./json_to_sknobs.py -r ../gen/iris/cfg_out/cap_te_INGRESS_3_cfg_reg.json -p 'cap0/sgi/te/3/' >> output.sknobs
./json_to_sknobs.py -r ../gen/iris/cfg_out/cap_te_EGRESS_3_cfg_reg.json -p 'cap0/sge/te/3/' >> output.sknobs
./json_to_sknobs.py -r ../gen/iris/cfg_out/cap_te_INGRESS_4_cfg_reg.json -p 'cap0/sgi/te/4/' >> output.sknobs
./json_to_sknobs.py -r ../gen/iris/cfg_out/cap_te_EGRESS_4_cfg_reg.json -p 'cap0/sge/te/4/' >> output.sknobs
./json_to_sknobs.py -r ../gen/iris/cfg_out/cap_te_INGRESS_5_cfg_reg.json -p 'cap0/sgi/te/5/' >> output.sknobs
./json_to_sknobs.py -r ../gen/iris/cfg_out/cap_te_EGRESS_5_cfg_reg.json -p 'cap0/sge/te/5/' >> output.sknobs
