include ${MKDEFS}/pre.mk
MODULE_TARGET       = eventtypes.proto
MODULE_PIPELINE     = iris gft
MODULE_PREREQS      = gogo.proto
MODULE_GEN_TYPES    = CC
MODULE_INCS         = ${MODULE_DIR} \
                      ${TOPDIR}/vendor/github.com/gogo/protobuf/gogoproto \
                      /usr/local/include
MODULE_POSTGEN_MK   = module_protolib.mk
MODULE_GEN_DIR      = ${BLD_PROTOGEN_DIR}
include ${MKDEFS}/post.mk