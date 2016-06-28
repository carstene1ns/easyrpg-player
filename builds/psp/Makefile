TOOLCHAIN_DIR?=/home/carsten/projects/easyrpg/buildscripts/psp

TARGET = EasyRPGPlayer
SOURCES = ../../src ../../lib/liblcf/src ../../lib/liblcf/src/generated
CPPFILES = $(foreach dir,$(SOURCES), $(wildcard $(dir)/*.cpp))
OBJS = $(CPPFILES:.cpp=.o)
INCLUDES = $(SOURCES)

INCDIR = $(foreach dir,$(INCLUDES), $(CURDIR)/$(dir))
CFLAGS = -G0 -g0 -O2 -fomit-frame-pointer -ffast-math -Wall -DPSP -DNDEBUG \
         -DLCF_SUPPORT_ICU -DPSPUI
#-DPPSSPP_COMPATIBLE
CXXFLAGS = $(CFLAGS) -std=c++11 -fno-exceptions -fno-rtti

PSP_EBOOT_TITLE = EasyRPG Player
PSP_EBOOT_ICON = "144x80.png"
PSP_EBOOT_PIC1 = "480x272.png"
EXTRA_TARGETS = EBOOT.PBP

PSPSDK=$(shell psp-config --pspsdk-path)
CFLAGS += -I$(TOOLCHAIN_DIR)/include/pixman-1 \
          -I$(TOOLCHAIN_DIR)/include

LIBS = -L$(TOOLCHAIN_DIR)/lib -L$(PSPSDK)/../lib \
       -lpixman-1 -lpng -lz \
       -licui18n -licuuc -licudata \
       -lglib2d -lpsprtc -lpsphprm \
       -lpspgu -lpspvram -lpsppower \
       -lpspdebug -lstdc++ -lm

%.o: %.cpp
	@echo "Compiling $(notdir $^)..."
	@$(CXX) $(CXXFLAGS) -c $< -o $@

BUILD_PRX = 1
#PSP_FW_VERSION = 330
PSP_LARGE_MEMORY ?= 0
USE_USER_LIBS = 1
ENCRYPT ?= 0

include $(PSPSDK)/lib/build.mak
