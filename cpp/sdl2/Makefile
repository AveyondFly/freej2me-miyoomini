TOOLCHAIN ?= /home/ubuntu/distribution/build.ROCKNIX-RK3326.aarch64/toolchain/bin/aarch64-rocknix-linux-gnueabi-

CXX      = $(TOOLCHAIN)g++
CXXFLAGS = -std=c++11 -fPIC -O3 -fno-strict-aliasing -march=armv8-a+crc -mtune=cortex-a53
LDFLAGS  = -lSDL2 -lSDL2_ttf -lpthread -lfreeimage

TARGET   = sdl_interface
SOURCES  = gkdminiplus.cpp cJSON.c

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(TARGET)
