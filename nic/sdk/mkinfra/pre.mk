# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

MODULE_ARCH                 := x86_64 aarch64
MODULE_PIPELINE             := iris gft apollo hello l2switch elektra phoebus artemis apulu gft16 poseidon
MODULE_ASIC                 := ${ASIC}
MODULE_FWTYPE               := ${FWTYPE}
MODULE_SRC_DIR              := ${MODULE_DIR}
MODULE_EXPORT_DIR           := ${MODULE_DIR}/${ARCH}
MODULE_BASECMD              :=
MODULE_GENERATOR            :=
MODULE_TEMPLATE             :=
MODULE_OUTFILE              :=
MODULE_ARGS                 :=
MODULE_EXPORT_LIBS          :=
MODULE_EXPORT_BINS          :=
MODULE_GOPKG                := github.com/pensando/sw/nic/${MODULE_DIR}
