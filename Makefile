FILES = ./build/hskernel.asm.o

all: ./bin/hsboot.bin ./bin/hskernel.bin $(FILES)
	rm -rf ./bin/helios.bin
	dd if=./bin/hsboot.bin >> ./bin/helios.bin
	dd if=./bin/hskernel.bin >> ./bin/helios.bin
	dd if=/dev/zero bs=512 count=128 >> ./bin/helios.bin

./bin/hskernel.bin: $(FILES)
	i686-elf-ld -g -relocatable $(FILES) -o ./build/hskernel.o
	i686-elf-gcc -T ./src/linker.ld -o ./bin/hskernel.bin -ffreestanding -O0 -nostdlib ./build/hskernel.o

./bin/hsboot.bin: ./src/boot/hsboot.asm
	nasm -f bin ./src/boot/hsboot.asm -o ./bin/hsboot.bin

./build/hskernel.asm.o: ./src/hskernel.asm
	nasm -f elf -g ./src/hskernel.asm -o ./build/hskernel.asm.o

clean:
	rm -rf ./bin/hsboot.bin
	rm -rf ./bin/hskernel.bin
	rm -rf ./bin/helios.bin
	rm -rf $(FILES)
	rm -rf ./build/hskernel.o