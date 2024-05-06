TARGET = kernel.elf
OBJS = main/main.o graphics/graphics.o font/font.o hankaku.o newlib_support.o
DEPENDS = $(join $(dir $(OBJS)),$(addprefix .,$(notdir $(OBJS:.o=.d))))

CFLAGS   += -O2 -Wall -g --target=x86_64-elf -ffreestanding -mno-red-zone
CXXFLAGS += -O2 -Wall -g --target=x86_64-elf -ffreestanding -mno-red-zone \
						-fno-exceptions -fno-rtti -std=c++17
LDFLAGS  += --entry KernelMain -z norelro --image-base 0x100000 --static

.PHONY: all
all: $(TARGET)

.PHONY: clean
clean:
	rm -rf *.o & rm -rf graphics/*.o & rm -rf font/*.o

hankaku.bin: ../tools/hankaku.txt
	../tools/makefont.py -o hankaku.bin ../tools/hankaku.txt

hankaku.o: hankaku.bin 
	objcopy -I binary -O elf64-x86-64 -B i386:x86-64 hankaku.bin hankaku.o

%.o: %.c Makefile
	clang $(CPPFLAGS) $(CFLAGS) -c $<

kernel.elf: $(OBJS) Makefile
	ld.lld $(LDFLAGS) -o kernel.elf $(OBJS) -lc

font/font.o: font/font.cpp Makefile
	clang++ $(CPPFLAGS) $(CXXFLAGS) -c $<  -o $@

graphics/graphics.o: graphics/graphics.cpp Makefile
	clang++ $(CPPFLAGS) $(CXXFLAGS) -c $<  -o $@

main/main.o: main/main.cpp Makefile
	clang++ $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

.%.d: %.c
	clang $(CPPFLAGS) $(CFLAGS) -MM $< > $@
	$(eval OBJ = $(<:.c=.o))
	sed --in-place 's|$(notdir $(OBJ))|$(OBJ)|' $@

.%.d: %.cpp
	clang++ $(CPPFLAGS) $(CXXFLAGS) -MM $< > $@
	$(eval OBJ = $(<:.cpp=.o))
	sed --in-place 's|$(notdir $(OBJ))|$(OBJ)|' $@

.PHONY: depends
depends:
	$(MAKE) $(DEPENDS)

-include $(DEPENDS)