# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

ifeq "${RELEASE}" "1"
    RELEASE_GXX_FLAGS := -O2
else
    RELEASE_GXX_FLAGS := -g
endif
