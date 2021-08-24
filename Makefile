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
	src/engine.cc \
	src/window.cc

HDRS=\
	src/dimensions.h \
	src/engine.h \
	src/event_service.h \
	src/glfw3.h \
	src/pad.h \
	src/vk.h \
	src/window.h

.PHONY: all lint tidy fmt clean

all: elysian

lint: tidy

tidy: $(HDRS) $(SRCS)
	$(TIDY) --fix -header-filter=src/.* $(SRCS) src/main.cc -- -x c++ $(CFLAGS)

fmt: $(HDRS) $(SRCS)
	$(FMT) -i $(HDRS) $(SRCS) src/main.cc

%.o: %.cc %.h
	$(CC) $(CFLAGS) $< -c -o $@

elysian: $(SRCS) $(HDRS) src/main.cc
	@echo $(SRCS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(SRCS) $(LDFLAGS) src/main.cc -o elysian

clean:
	rm -rf elysian *.o
