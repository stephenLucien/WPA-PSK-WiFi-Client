# current file directory
MAKEFILE_DIR_common_mk := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

# glob files into variables
CSRCS := $(CSRCS) $(notdir $(wildcard $(MAKEFILE_DIR_common_mk)/*.c)) 
CXXSRCS := $(CXXSRCS) $(notdir $(wildcard  $(MAKEFILE_DIR_common_mk)/*.cpp))

# include headers in current folder
CPPFLAGS := $(CPPFLAGS) -I$(MAKEFILE_DIR_common_mk)

# srcs only contain name (no path info), so VPATH is needed
VPATH := $(VPATH) $(MAKEFILE_DIR_common_mk)

# include subdirs
include $(MAKEFILE_DIR_common_mk)/wpa_supplicant/config.mk

# strip duplicate items
#CPPFLAGS := $(shell echo "${CPPFLAGS}" | tr ' ' '\n' | awk '!a[$$0]++' | tr '\n' ' ')
#LIBS := $(shell echo "${LIBS}" | tr ' ' '\n' | awk '!a[$$0]++' | tr '\n' ' ')
#LDFLAGS := $(shell echo "${LDFLAGS}" | tr ' ' '\n' | awk '!a[$$0]++' | tr '\n' ' ')