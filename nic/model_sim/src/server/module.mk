# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MAKEDEFS}/pre.mk
MODULE_TARGET   = cap_model
MODULE_LDPATHS  = ${MODULE_DIR}/../../libs
MODULE_LDLIBS   = model_cc \
                  capsim \
				  common_cc \
				  top_csr_compile \
				  sknobs \
				  asmsym \
				  mpuobj \
				  capisa \
				  isa \
				  Judy \
				  python2.7 \
				  zmq \
				  gmp \
				  gmpxx \
				  pthread \
				  dl \
				  util \
				  crypto
include ${MAKEDEFS}/post.mk
