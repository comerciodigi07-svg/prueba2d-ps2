EE_BIN = PONGPS2.ELF
EE_OBJS = src/main.o
EE_LIBS = -lpad -lgsKit -ldmaKit -lm
EE_CFLAGS = -O2 -G0 -Wall -Wextra
EE_LDFLAGS = -Wl,-Map,PONGPS2.map

all: $(EE_BIN)

iso: $(EE_BIN)
	mkdir -p cd
	cp $(EE_BIN) cd/BOOT.ELF
	printf 'BOOT2 = cdrom0:\\\\BOOT.ELF;1\nVER = 1.00\nVMODE = NTSC\n' > cd/SYSTEM.CNF
	@if command -v genisoimage >/dev/null 2>&1; then \
		genisoimage -o PongPS2.iso -V PONGPS2 -sysid PLAYSTATION -publisher PONGPS2 -allow-lowercase -l -r cd; \
	elif command -v mkisofs >/dev/null 2>&1; then \
		mkisofs -o PongPS2.iso -V PONGPS2 -sysid PLAYSTATION -publisher PONGPS2 -allow-lowercase -l -r cd; \
	else \
		echo "Falta genisoimage o mkisofs para crear la ISO."; exit 1; \
	fi

clean:
	rm -f $(EE_OBJS) $(EE_BIN) PONGPS2.map PongPS2.iso
	rm -f cd/BOOT.ELF cd/SYSTEM.CNF

include $(PS2SDK)/samples/Makefile.prefab
.PHONY: all iso clean
