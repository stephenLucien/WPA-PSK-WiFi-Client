# current file directory
MAKEFILE_DIR_wpa_test_mk := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

# glob files into variables
CSRCS := $(CSRCS) $(notdir $(wildcard $(MAKEFILE_DIR_wpa_test_mk)/*.c)) 
CXXSRCS := $(CXXSRCS) $(notdir $(wildcard  $(MAKEFILE_DIR_wpa_test_mk)/*.cpp))

# CPPFLAGS
CPPFLAGS := $(CPPFLAGS) -I$(MAKEFILE_DIR_wpa_test_mk)

# srcs only contain name (no path info), so VPATH is needed
VPATH := $(VPATH) $(MAKEFILE_DIR_wpa_test_mk)



# this file have main-entry
wpa_test_CSRCS := wpa_test.c
wpa_test_COBJS := $(patsubst %.c,$(OBJ_DIR)/%.o,$(wpa_test_CSRCS))

# substract set wpa_test_CSRCS from CSRCS
ifeq (y, y)
CSRCS := $(shell test -n ${wpa_test_CSRCS} && \
	TMP_SRCS="${CSRCS}"; \
	for src in ${wpa_test_CSRCS}; \
	do \
		TMP_SRCS=$$(echo "$${TMP_SRCS}" | tr ' ' '\n' | grep -v "$${src}" | tr '\n' ' '); \
	done; \
	echo $${TMP_SRCS})
endif


# this file have main-entry
wpa_test_CXXSRCS := 
wpa_test_CXXOBJS := $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(wpa_test_CXXSRCS))

# substract set wpa_test_CXXSRCS from CXXSRCS
ifeq (y, y)
CXXSRCS := $(shell test -n ${wpa_test_CXXSRCS} && \
	TMP_SRCS="${CXXSRCS}"; \
	for src in ${wpa_test_CXXSRCS}; \
	do \
		TMP_SRCS=$$(echo "$${TMP_SRCS}" | tr ' ' '\n' | grep -v "$${src}" | tr '\n' ' '); \
	done; \
	echo $${TMP_SRCS})	
endif


wpa_test_OBJS := $(wpa_test_COBJS) $(wpa_test_CXXOBJS)


$(wpa_test_COBJS):$(OBJ_DIR)/%.o:%.c
	@echo ---------------------[build $<]----------------------------------
	$(CC) $(CFLAGS) -c $(CPPFLAGS) $< -o $@   

$(wpa_test_CXXOBJS):$(OBJ_DIR)/%.o:%.cpp
	@echo ---------------------[build $<]----------------------------------
	$(CXX) $(CXXFLAGS) -c $(CPPFLAGS) $< -o $@   

clean::