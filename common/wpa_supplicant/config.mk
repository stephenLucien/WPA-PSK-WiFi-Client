MAKEFILE_DIR_wpa_supplicant_mk := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

CPPFLAGS := $(CPPFLAGS) -I$(MAKEFILE_DIR_wpa_supplicant_mk)/include
LDFLAGS := $(LDFLAGS) -L$(MAKEFILE_DIR_wpa_supplicant_mk)/lib
LIBS := $(LIBS) -lwpa_client
