
WARN = -Wall -Werror -Wno-strict-aliasing -Wno-error=strict-aliasing -Wno-write-strings -Wno-deprecated

CPPFLAGS := $(WARN) \
          -g -D__USE_GNU -D_GNU_SOURCE -std=c++11 -fPIC

CFLAGS := $(WARN) \
          -g -D__USE_GNU -D_GNU_SOURCE
