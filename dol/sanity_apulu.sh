#! /bin/bash

export PIPELINE=apulu

# initial setup
source ./sanity_setup.sh
setup ${PIPELINE}

build

# run all apulu DOL dry runs
DryRunSanity hostvxlan networking
DryRunSanity l2_hostvxlan networking
DryRunSanity learn learn
#DryRunSanity overlay networking SUB="local2remote,remote2local"

# operd jobs
DryRunSanity hostvxlan operd

# rfc jobs
DryRunSanity rfc_disjoint rfc SUB="ipv4_egress_disjoint,ipv4_ingress_disjoint"
DryRunSanity rfc_adjacent rfc SUB="ipv4_egress_adjacent,ipv4_ingress_adjacent"
DryRunSanity rfc_overlap_1 rfc SUB="ipv4_egress_overlap_1,ipv4_ingress_overlap_1"
DryRunSanity rfc_overlap_2 rfc SUB="ipv4_egress_overlap_2,ipv4_ingress_overlap_2"
DryRunSanity rfc_overlap_3 rfc SUB="ipv4_egress_overlap_3,ipv4_ingress_overlap_3"
DryRunSanity rfc_negative_1 rfc SUB="ipv4_egress_negative_1,ipv4_ingress_negative_1"
DryRunSanity rfc_negative_2 rfc SUB="ipv4_egress_negative_2,ipv4_ingress_negative_2"
DryRunSanity rfc_empty rfc SUB="ipv4_egress_empty,ipv4_ingress_empty"
DryRunSanity rfc_tags_pfx_range rfc SUB="ipv4_egress_tag_pfx_range,ipv4_ingress_tag_pfx_range"
DryRunSanity rfc_tags_crud rfc SUB="ltag_crud,rtag_crud"

# lpm jobs
DryRunSanity lpm_adjacent lpm SUB="v4_adjacent"
DryRunSanity lpm_overlap lpm SUB="v4_overlap"
DryRunSanity lpm_full_overlap lpm SUB="v4_full_overlap"
DryRunSanity lpm_adjacent_priority lpm SUB="v4_adjacent"
DryRunSanity lpm_overlap_priority lpm SUB="v4_overlap"
DryRunSanity lpm_full_overlap_priority lpm SUB="v4_full_overlap"

# end of script
clean_exit
