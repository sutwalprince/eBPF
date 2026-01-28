BPFTOOL=bpftool
FILENAME=hello_buf_config
USER_SPACE_FILENAME=hello_buf

.PHONY: ${FILENAME}
$(FILENAME): skel
	clang $(USER_SPACE_FILENAME).c -lbpf -o $(USER_SPACE_FILENAME)

.PHONY: vmlinux
vmlinux:
	$(BPFTOOL) btf dump file /sys/kernel/btf/vmlinux format c > vmlinux.h

.PHONY: bpf
bpf: vmlinux
	clang -g -O2 -target bpf -c $(FILENAME).bpf.c -o $(FILENAME).bpf.o
.PHONY: skel
skel: bpf
	$(BPFTOOL) gen skeleton $(FILENAME).bpf.o name $(FILENAME) > $(FILENAME).skel.h
	rm $(FILENAME).bpf.o

.PHONY: run
run: clean $(FILENAME)
	sudo ./$(USER_SPACE_FILENAME)

.PHONY: gdb
gdb: bpf
	sudo gdb --args $(BPFTOOL) gen skeleton $(FILENAME).bpf.o name $(FILENAME)
.PHONY: clean
clean:
	-rm -rf *.o *.skel.h vmlinux.h $(FILENAME) $(USER_SPACE_FILENAME) 