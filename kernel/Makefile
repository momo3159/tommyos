TARGET = kernel.elf 
OBJS = main.o

CXXFLAGS += -O2 -Wall -g --target=x86_64-elf -ffreestanding -mno-red-zone -fno-exceptions -fno-rtti -std=c++17 
LDFLAGS += --entry KernelMain -z norelro --image-base 0x100000 --static

# .PHONYは疑似ターゲットで、必須項目が本当のファイル名ではないことを表す
# make all とすると、all: $(TARGET)→kernel.elfをターゲットするレシピが動く。
.PHONY: all 
all: $(TARGET)

.PHONY: clean
clean:
	rm -rf *.o 

# ターゲット、ターゲットを生成するのに必要な項目、ターゲットを生成するレシピ
kernel.elf: $(OBJS) Makefile 
	ld.lld $(LDFLAGS) -o kernel.elf $(OBJS)

%.o: %.cpp Makefile 
	clang++ $(CPPFLAGS) $(CXXFLAGS) -c $<