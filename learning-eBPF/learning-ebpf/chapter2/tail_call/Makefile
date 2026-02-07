CLANG ?= clang
CC ?= gcc

all: xdp_tailcall.bpf.o xdp_tailcall.skel.h xdp_tailcall

xdp_tailcall.bpf.o: xdp_tailcall.bpf.c vmlinux.h
	$(CLANG) -O2 -g -target bpf \
		-D__TARGET_ARCH_x86 \
		-c $< -o $@

xdp_tailcall.skel.h: xdp_tailcall.bpf.o
	bpftool gen skeleton $< > $@

xdp_tailcall: xdp_tailcall.c
	$(CC) $< -o $@ -lbpf

clean:
	rm -f *.o *.skel.h xdp_tailcall
