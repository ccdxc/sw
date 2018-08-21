include makedefs/common.mk

CMD_GCC         := g++
CMD_GCC_FLAGS   := -MMD -g -O2

CMD_AR          := ar
CMD_AR_FLAGS    := rcs

CONFIG_LIB_A_FLAGS  := -DARLIB
CONFIG_LIB_SO_FLAGS := -fPIC -shared
