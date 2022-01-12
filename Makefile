# 
# 1. c sources (*.c) should be added to set CSRCS
# 2. cpp sources (*.cpp) should be added to set CXXSRCS
# 3. precompiled object files (*.o) should be added to set OBJS
# 4. thirdparty libraries (headers & libs) should be added to common folder,
#    etc: common/${libname}/{include/headers.h,lib/lib*.{a,so}} and a config.mk
#    file that adds library's flags to CPPFLAGS / LDFLAGS / LIBS should be placed 
#    into common/${libname} folder
# 5. a config file should be also placed into subfolder, which 
#    adds elements into CSRCS / CXXSRCS / VPATH / OBJS / CPPFLAGS / CFLAGS / CXXFLAGS
#    / LDFLAGS / LIBS
# 6. since we use VPATH, all header/source/object files in VPATH are considered in the same directory,
#    the same filename is not allowed.


MAKEFILE_DIR_main_mk := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))


# BUILD files
BUILD_DIR ?= build
$(shell test -e $(BUILD_DIR) || mkdir $(BUILD_DIR) )
OBJ_DIR ?= $(BUILD_DIR)/objs
$(shell test -e $(OBJ_DIR) || mkdir $(OBJ_DIR) )
BIN_DIR ?= $(BUILD_DIR)/bin
$(shell test -e $(BIN_DIR) || mkdir $(BIN_DIR))

CFLAGS := ${CFLAGS} -Wall -Werror
CXXFLAGS := ${CXXFLAGS} -Wall -Werror

# include other makefiles
include ../AnyKa_SDK_v1.03/gcc.mk
include common/config.mk
include wpa_funs/config.mk

# the following makefiles have target rules.
include wpa_test/config.mk


#$(info "VPATH:$(VPATH)")


# C OBJECTS
# add csrcs' set into cobjs' set 
COBJS := $(COBJS) $(CSRCS)
# substitube suffix .c to .o of element in set
COBJS := $(patsubst %.c, %.o, $(COBJS))
# add folder prefix
COBJS := $(patsubst %,$(OBJ_DIR)/%,$(COBJS))
#$(info "COBJS:$(COBJS)")

# CXX OBJECTS
# add cxxsrcs' set into cxxobjs' set 
CXXOBJS := $(CXXOBJS) $(CXXSRCS)
# substitube suffix .cpp to .o of element in set
CXXOBJS :=$(patsubst %.cpp, %.o, $(CXXOBJS))
# add folder prefix
CXXOBJS := $(patsubst %,$(OBJ_DIR)/%,$(CXXOBJS))
#$(info "CXXOBJS:$(CXXOBJS)")

# copy precompiled obj to OBJ_DIR
$(shell test -n $(OBJS) && for n in $(OBJS);do(cp $${n} ${OBJ_DIR})done;)

# ALL OBJS
OBJS := $(CXXOBJS) $(COBJS) $(OBJS)
#$(info "OBJS:$(OBJS)")


# strip duplicate items
LIBS := $(shell echo "${LIBS}" | tr ' ' '\n' | awk '!a[$$0]++' | tr '\n' ' ')
LDFLAGS := $(shell echo "${LDFLAGS}" | tr ' ' '\n' | awk '!a[$$0]++' | tr '\n' ' ')
CFLAGS := $(shell echo "${CFLAGS}" | tr ' ' '\n' | awk '!a[$$0]++' | tr '\n' ' ')
CXXFLAGS := $(shell echo "${CXXFLAGS}" | tr ' ' '\n' | awk '!a[$$0]++' | tr '\n' ' ')
CPPFLAGS := $(shell echo "${CPPFLAGS}" | tr ' ' '\n' | awk '!a[$$0]++' | tr '\n' ' ')

# $(TARGET):$(OBJS)
# 	$(CC) $(LDFLAGS) $^ $(LIBS) -o $@
# 	${STRIP} $@

$(COBJS):$(OBJ_DIR)/%.o:%.c
	@echo ---------------------[build $<]----------------------------------
	$(CC) $(CFLAGS) -c $(CPPFLAGS) $< -o $@   

$(CXXOBJS):$(OBJ_DIR)/%.o:%.cpp
	@echo ---------------------[build $<]----------------------------------
	$(CXX) $(CXXFLAGS) -c $(CPPFLAGS) $< -o $@   


clean::
#	rm $(OBJS)
	rm -rf $(BUILD_DIR)
#	rm $(TARGET)


wpa_wifi_psk_test: $(wpa_test_OBJS) $(OBJS)
	$(CC) $(LDFLAGS) $^ $(LIBS) -o $(BIN_DIR)/$@
	${STRIP} $(BIN_DIR)/$@	


.DEFAULT_GOAL := wpa_wifi_psk_test