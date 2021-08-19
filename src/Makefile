CLANG=clang++
CFLAGS=\
	-std=c++20 \
	-fmodules \
	-fbuiltin-module-map \
	-fimplicit-modules \
	-fimplicit-module-maps \
	-fprebuilt-module-path=. \
	-Wall \
	-Wextra \
	-Weverything \
	-Wno-c++98-compat \
	-Wno-c++98-compat-pedantic \
	-Wno-poison-system-directories \
	-pedantic-errors \
	-I$(VULKAN_SDK)/include

MODULES=\
	engine.cc

PCM_FILES=$(patsubst %.cc,%.pcm,$(MODULES))

all: elysian

%.pcm: %.cc
	$(CLANG) $(CFLAGS) --precompile -x c++-module -o $@ $<

elysian: $(PCM_FILES) main.cc
	$(CLANG) $(CFLAGS) $(PCM_FILES) main.cc -o elysian

clean:
	rm elysian *.pcm
