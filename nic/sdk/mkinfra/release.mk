# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

ifeq "${RELEASE}" "1"
    RELEASE_GXX_FLAGS := -O2 -fno-strict-aliasing
else ifeq "${DEBUG}" "1"
    RELEASE_GXX_FLAGS := -g -O0 -fno-strict-aliasing
else
    RELEASE_GXX_FLAGS := -g -O2 -fno-strict-aliasing
endif
