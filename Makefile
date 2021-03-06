SHELL	= bash
CXX	= g++-6
LD	= ld
NASM	= nasm
QEMU	= qemu-system-i386
BUILD	= build
ISODIR	= isodir
KERNEL	= $(BUILD)/kernel
ISOFILE	= $(BUILD)/hanaos.iso
CXXFLAGS= -ffreestanding -fno-exceptions -fno-rtti -m32 -fpermissive -Wwrite-strings -Iinclude -w -std=c++14
LDFLAGS	= -m elf_i386 -N
CXXSRC	= $(wildcard *.cpp)
ASMSRC	= $(wildcard *.asm)
CXXOBJ	= $(addprefix $(BUILD)/,$(notdir $(CXXSRC:.cpp=.o)))
ASMOBJ	= $(addprefix $(BUILD)/,$(notdir $(ASMSRC:.asm=.a.o)))
DEPS	= $(addprefix $(BUILD)/,$(notdir $(CXXSRC:.cpp=.d)))
APPS	= $(wildcard apps/*.cpp)

define colorecho
      @echo -e "\033[36m$1\033[0m"
endef

default: iso

.PHONY: kernel
kernel: $(KERNEL) | $(BUILD)

.PHONY: $(BUILD)
$(BUILD):
	@mkdir -p $(BUILD)

$(BUILD)/%.d: %.cpp | $(BUILD)
	$(call colorecho,GENDEP\t$<)
	@set -e;rm -f $@; \
	$(CXX) -MM -Iinclude $< > $@.$$$$; \
	sed 's,.*\.o[ :]*,$*.o $@ : ,g' < $@.$$$$ > $@; \
	echo -n '$(BUILD)/'|cat - $@ > $@.tmp;mv $@.tmp $@; \
	rm -f $@.$$$$

$(BUILD)/%.a.o: %.asm
	$(call colorecho,NASM\t$<)
	@$(NASM) -f elf $*.asm -o $(BUILD)/$*.a.o

$(BUILD)/%.o: %.cpp
	$(call colorecho,CXX\t$<)
	@$(CXX) -c $*.cpp -o $(BUILD)/$*.o $(CXXFLAGS)

$(BUILD)/apps.o: $(APPS) apps/Makefile
	@make -C apps

$(KERNEL): $(ASMOBJ) $(CXXOBJ) $(BUILD)/apps.o
	$(call colorecho,Link kernel...)
	@$(LD) $(ASMOBJ) $(CXXOBJ) $(BUILD)/apps.o -T link.ld -o $(KERNEL) $(LDFLAGS)

$(ISOFILE): kernel
	$(call colorecho,Generate iso image...)
	@cp $(KERNEL) $(ISODIR)/boot/
	@strip $(ISODIR)/boot/kernel
	@grub-mkrescue --directory=/usr/lib/grub/i386-pc --output=$(ISOFILE) $(ISODIR)

iso: $(ISOFILE)

run: iso
	$(QEMU) -m 32 -vga std -cdrom $(ISOFILE)

clean:
	rm -rf $(BUILD)

ifneq ($(MAKECMDGOALS),clean)
    -include $(DEPS)
endif
