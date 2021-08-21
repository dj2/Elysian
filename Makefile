CC=clang++
TIDY=clang-tidy
FMT=clang-format

CFLAGS=\
	-std=c++20 \
	-stdlib=libc++ \
	-Wall \
	-Weverything \
	-Wextra \
	-Wno-c++98-compat \
	-Wno-c++98-compat-pedantic \
	-Wno-poison-system-directories \
	-pedantic-errors \
	-I. \
	-I$(VULKAN_SDK)/include

SRCS=\
	src/engine.cc

HDRS=\
	src/engine.h \
	src/pad.h

O_FILES=$(patsubst %.cc,%.o,$(SRCS))

.PHONY: all tidy fmt clean

all: elysian

tidy: $(HDRS) $(SRCS)
	$(TIDY) $(HDRS) $(SRCS) -- $(CFLAGS)

fmt: $(HDRS) $(SRCS)
	$(FMT) -i $(HDRS) $(SRCS) -- $(CFLAGS)

%.o: %.cc %.h
	$(CC) $(CFLAGS) $< -c -o $@

elysian: $(O_FILES) $(HDRS) src/main.cc
	@echo $(O_FILES)
	$(CC) $(CFLAGS) $(O_FILES) -L$(VULKAN_SDK)/lib -lvulkan src/main.cc -o elysian

clean:
	rm -rf elysian *.o
