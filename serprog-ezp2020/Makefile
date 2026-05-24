# Makefile - Build ezp2020-serprog firmware on Linux / macOS
#
# Requires: sdcc, objcopy (binutils)
#   Debian/Ubuntu: sudo apt install sdcc binutils
#   Arch:          sudo pacman -S sdcc binutils
#   macOS/Homebrew: brew install sdcc
#
# Usage:
#   make            # build ezp2020-serprog.bin
#   make clean      # remove build artefacts

SDCC    ?= sdcc
OBJCOPY ?= objcopy
TARGET   = ezp2020-serprog
OUTDIR   = build

CFLAGS  = -mmcs51 --model-small \
          --xram-size 0x0300 --xram-loc 0x0100 \
          --code-size 0x3800 \
          -DFREQ_SYS=24000000 \
          -Iinclude

RELS    = $(OUTDIR)/debug.rel $(OUTDIR)/spi.rel $(OUTDIR)/main.rel

.PHONY: all clean

all: $(TARGET).bin

$(OUTDIR):
	mkdir -p $(OUTDIR)

$(OUTDIR)/debug.rel: include/debug.c | $(OUTDIR)
	$(SDCC) $(CFLAGS) -c $< -o $@

$(OUTDIR)/spi.rel: include/spi.c | $(OUTDIR)
	$(SDCC) $(CFLAGS) -c $< -o $@

$(OUTDIR)/main.rel: main.c | $(OUTDIR)
	$(SDCC) $(CFLAGS) -c $< -o $@

$(OUTDIR)/$(TARGET).ihx: $(RELS)
	$(SDCC) $(CFLAGS) $^ -o $@

$(TARGET).bin: $(OUTDIR)/$(TARGET).ihx
	$(OBJCOPY) -I ihex -O binary $< $@
	@echo ""
	@echo "Done! Flash file: $@"
	@echo ""
	@echo "Flash with wchisp (put CH552 in ISP mode first):"
	@echo "  wchisp flash $@"

clean:
	rm -rf $(OUTDIR) $(TARGET).bin
