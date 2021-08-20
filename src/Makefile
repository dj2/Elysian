CC=g++-11
CFLAGS=\
	-std=c++20 \
	-fmodules-ts \
	-Wall \
	-Wextra \
	-pedantic-errors \
	-I$(VULKAN_SDK)/include

CLANG_FLAGS=\
	-fbuiltin-module-map \
	-fimplicit-modules \
	-fimplicit-module-maps \
	-fprebuilt-module-path=. \
	-Weverything \
	-Wno-c++98-compat \
	-Wno-c++98-compat-pedantic \
	-Wno-poison-system-directories \
	--precompile -x c++-module

MODULES=\
	engine.cc

PCM_FILES=$(patsubst %.cc,%.gcm,$(MODULES))

all: elysian

elysian: $(MODULES) main.cc
	$(CC) $(CFLAGS) $(MODULES) main.cc -o elysian

clean:
	rm elysian
