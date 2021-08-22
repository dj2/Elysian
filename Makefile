CC=g++-11
TIDY=clang-tidy
FMT=clang-format

CFLAGS=\
	-std=c++20 \
	-Wall \
	-Wcast-align \
	-Wcast-qual \
	-Wconversion \
	-Wctor-dtor-privacy \
	-Wextra \
	-Wfloat-equal \
	-Wformat=2 \
	-Wformat-nonliteral \
	-Wformat-security \
	-Wformat-y2k \
	-Wimport \
	-Winit-self \
	-Winline \
	-Wlogical-op \
	-Wmissing-declarations \
	-Wmissing-field-initializers \
	-Wmissing-include-dirs \
	-Wmissing-noreturn \
	-Wnoexcept \
	-Wold-style-cast \
	-Woverloaded-virtual \
	-Wpacked \
	-Wpadded \
	-Wpointer-arith \
	-Wredundant-decls \
	-Wshadow \
	-Wsign-conversion \
	-Wsign-promo \
	-Wstrict-null-sentinel \
	-Wstrict-overflow=5 \
	-Wswitch-enum \
	-Wundef \
	-Wunreachable-code \
	-Wunused \
	-Wunused-parameter \
	-Wuseless-cast \
	-Wvariadic-macros \
	-Wwrite-strings \
	-Wzero-as-null-pointer-constant \
	-pedantic \
	-pedantic-errors \
	-I. \
	-I$(VULKAN_SDK)/include


SRCS=\
	src/engine.cc

HDRS=\
	src/engine.h \
	src/pad.h

O_FILES=$(patsubst %.cc,%.o,$(SRCS))

.PHONY: all lint tidy fmt clean

all: elysian

lint: tidy

tidy: $(HDRS) $(SRCS)
	$(TIDY) -header-filter=src/.* $(SRCS) -- -x c++ $(CFLAGS)

fmt: $(HDRS) $(SRCS)
	$(FMT) -i $(HDRS) $(SRCS)

%.o: %.cc %.h
	$(CC) $(CFLAGS) $< -c -o $@

elysian: $(O_FILES) $(HDRS) src/main.cc
	@echo $(O_FILES)
	$(CC) $(CFLAGS) $(LDFLAGS) $(O_FILES) -L$(VULKAN_SDK)/lib -lvulkan src/main.cc -o elysian

clean:
	rm -rf elysian *.o
