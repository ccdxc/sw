# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
#####################################################################
######################## UTILITY FUNCTIONS ##########################
#####################################################################
ifeq "${V}" ""
AT := @
NAT:= 
else
AT :=
NAT:= @
endif

define TARGET_TO_TGID
$(subst .,_,$(strip ${1}))
endef

define MODULE_PATH_TO_SRC_DIR
$(subst /module,,$(basename ${1}))
endef

define MODULE_PATH_TO_GID
$(subst /,_,$(subst /module,,$(basename ${1})))
endef

define CANPATH
$(patsubst ${CURDIR}/%,%,$(abspath ${1}))
endef

ifdef DEBUG
define DEBUGMSG
$(info ${1})
endef
endif
