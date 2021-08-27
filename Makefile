CC=clang++
TIDY=clang-tidy
FMT=clang-format

CFLAGS=\
	-g \
	-O0 \
	-std=c++20 \
	-Wall \
	-Werror \
	-Weverything \
	-Wextra \
	-Wno-c++98-compat \
	-Wno-c++98-compat-pedantic \
	-Wno-poison-system-directories \
	-pedantic \
	-pedantic-errors \
	-I. \
	-I$(VULKAN_SDK)/include \
	-I$(GLFW_SDK)/include

LDFLAGS=\
	-L$(VULKAN_SDK)/lib \
	-L$(GLFW_SDK)/lib \
	-lvulkan \
	-lglfw

SRCS=\
	src/engine/device.cc \
	src/engine/shader.cc \
	src/engine/swapchain.cc \
	src/engine/vk.cc \
	src/window.cc

HDRS=\
	src/dimensions.h \
	src/engine.h \
	src/engine/device.h \
	src/engine/error.h \
	src/engine/shader.h \
	src/engine/swapchain.h \
	src/engine/version.h \
	src/engine/vk.h \
	src/event_service.h \
	src/glfw3.h \
	src/pad.h \
	src/window.h

.PHONY: all lint tidy fmt clean

all: elysian

lint: tidy

tidy: $(SRCS) src/main.cc
	$(TIDY) --fix -header-filter=src/ $^ -- -x c++ $(CFLAGS)

format: fmt

fmt: $(HDRS) $(SRCS) src/main.cc
	$(FMT) -i $^

%.o: %.cc %.h
	$(CC) $(CFLAGS) $< -c -o $@

elysian: $(SRCS) $(HDRS) src/main.cc
	$(CC) $(CFLAGS) $(LDFLAGS) $(SRCS) $(LDFLAGS) src/main.cc -o $@

clean:
	rm -rf elysian *.o *.dSYM
