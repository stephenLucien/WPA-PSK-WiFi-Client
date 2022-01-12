# current file directory
MAKEFILE_DIR_wpa_funs_mk := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

# glob files into variables
CSRCS := $(CSRCS) $(notdir $(wildcard $(MAKEFILE_DIR_wpa_funs_mk)/*.c)) 
CXXSRCS := $(CXXSRCS) $(notdir $(wildcard  $(MAKEFILE_DIR_wpa_funs_mk)/*.cpp))

# CPPFLAGS
CPPFLAGS := $(CPPFLAGS) -I$(MAKEFILE_DIR_wpa_funs_mk)

# srcs only contain name (no path info), so VPATH is needed
VPATH := $(VPATH) $(MAKEFILE_DIR_wpa_funs_mk)