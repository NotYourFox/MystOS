FILES = ./build/hskernel.asm.o ./build/hskernel.o ./build/idt/idt.asm.o ./build/idt/idt.o ./build/mem/mem.o ./build/io/io.asm.o
INCLUDES = -I./src
FLAGS = -g -ffreestanding -falign-jumps -falign-functions -falign-labels -falign-loops -fstrength-reduce -fomit-frame-pointer -finline-functions -Wno-unused-function -fno-builtin -Werror -Wno-unused-label -Wno-cpp -Wno-unused-parameter -nostdlib -nostartfiles -nodefaultlibs -Wall -O0 -Iinc

all: ./bin/hsboot.bin ./bin/hskernel.bin ./version.txt $(FILES)
	rm -rf ./bin/helios.bin
	dd if=./bin/hsboot.bin >> ./bin/helios.bin
	dd if=./bin/hskernel.bin >> ./bin/helios.bin
	dd if=./version.txt >> ./bin/helios.bin
	dd if=/dev/zero bs=512 count=128 >> ./bin/helios.bin

./bin/hskernel.bin: $(FILES)
	i686-elf-ld -g -relocatable $(FILES) -o ./build/hsfkrnl.o
	i686-elf-gcc $(FLAGS) -T ./src/linker.ld -o ./bin/hskernel.bin -ffreestanding -O0 -nostdlib ./build/hsfkrnl.o

./bin/hsboot.bin: ./src/boot/hsboot.asm
	nasm -f bin ./src/boot/hsboot.asm -o ./bin/hsboot.bin

./build/hskernel.asm.o: ./src/hskernel.asm
	nasm -f elf -g ./src/hskernel.asm -o ./build/hskernel.asm.o

./build/hskernel.o: ./src/hskernel.c
	i686-elf-gcc $(INCLUDES) $(FLAGS) -std=gnu99 -c ./src/hskernel.c -o ./build/hskernel.o

./build/idt/idt.asm.o: ./src/idt/idt.asm
	nasm -f elf -g ./src/idt/idt.asm -o ./build/idt/idt.asm.o

./build/idt/idt.o: ./src/idt/idt.c
	i686-elf-gcc $(INCLUDES) -I./src/idt $(FLAGS) -std=gnu99 -c ./src/idt/idt.c -o ./build/idt/idt.o

./build/mem/mem.o: ./src/mem/mem.c
	i686-elf-gcc $(INCLUDES) -I./src/mem $(FLAGS) -std=gnu99 -c ./src/mem/mem.c -o ./build/mem/mem.o

./build/io/io.asm.o: ./src/io/io.asm
	nasm -f elf -g ./src/io/io.asm -o ./build/io/io.asm.o

clean:
	rm -rf ./bin/hsboot.bin
	rm -rf ./bin/hskernel.bin
	rm -rf ./bin/helios.bin
	rm -rf $(FILES)
	rm -rf ./build/hsfkrnl.o