FILES = ./build/moskernel.asm.o ./build/moskernel.o ./build/disk/stream.o ./build/idt/idt.asm.o ./build/idt/idt.o ./build/mem/mem.o ./build/io/io.asm.o ./build/mem/heap/heap.o ./build/mem/heap/kheap.o ./build/mem/page/pagefile.o ./build/mem/page/pagefile.asm.o ./build/io/vgaio/vgaio.o ./build/disk/disk.o ./build/fs/pparser.o ./build/fs/fs.o ./build/fs/fat/fat16.o
INCLUDES = -I./src
FLAGS = -g -ffreestanding -falign-jumps -falign-functions -falign-labels -falign-loops -fstrength-reduce -fomit-frame-pointer -finline-functions -Wno-unused-function -fno-builtin -Werror -Wno-unused-label -Wno-cpp -Wno-unused-parameter -nostdlib -nostartfiles -nodefaultlibs -Wall -O0 -Iinc

all: ./bin/mosboot.bin ./bin/moskernel.bin ./version.txt $(FILES)
	rm -rf ./bin/mystos.bin
	dd if=./bin/mosboot.bin >> ./bin/mystos.bin
	dd if=./bin/moskernel.bin >> ./bin/mystos.bin
	dd if=./version.txt >> ./bin/mystos.bin
	dd if=/dev/zero bs=1048576 count=16 >> ./bin/mystos.bin

./bin/moskernel.bin: $(FILES)
	i686-elf-ld -g -relocatable $(FILES) -o ./build/mosfkrnl.o
	i686-elf-gcc $(FLAGS) -T ./src/linker.ld -o ./bin/moskernel.bin -ffreestanding -O0 -nostdlib ./build/mosfkrnl.o

./bin/mosboot.bin: ./src/boot/mosboot.asm
	nasm -f bin ./src/boot/mosboot.asm -o ./bin/mosboot.bin

./build/moskernel.asm.o: ./src/moskernel.asm
	nasm -f elf -g ./src/moskernel.asm -o ./build/moskernel.asm.o

./build/moskernel.o: ./src/moskernel.c
	i686-elf-gcc $(INCLUDES) $(FLAGS) -std=gnu99 -c ./src/moskernel.c -o ./build/moskernel.o

./build/idt/idt.asm.o: ./src/idt/idt.asm
	nasm -f elf -g ./src/idt/idt.asm -o ./build/idt/idt.asm.o

./build/idt/idt.o: ./src/idt/idt.c
	i686-elf-gcc $(INCLUDES) -I./src/idt $(FLAGS) -std=gnu99 -c ./src/idt/idt.c -o ./build/idt/idt.o

./build/mem/mem.o: ./src/mem/mem.c
	i686-elf-gcc $(INCLUDES) -I./src/mem $(FLAGS) -std=gnu99 -c ./src/mem/mem.c -o ./build/mem/mem.o

./build/io/io.asm.o: ./src/io/io.asm
	nasm -f elf -g ./src/io/io.asm -o ./build/io/io.asm.o

./build/mem/heap/heap.o: ./src/mem/heap/heap.c
	i686-elf-gcc $(INCLUDES) -I./src/mem/heap $(FLAGS) -std=gnu99 -c ./src/mem/heap/heap.c -o ./build/mem/heap/heap.o

./build/mem/heap/kheap.o: ./src/mem/heap/kheap.c
	i686-elf-gcc $(INCLUDES) -I./src/mem/heap $(FLAGS) -std=gnu99 -c ./src/mem/heap/kheap.c -o ./build/mem/heap/kheap.o	

./build/mem/page/pagefile.o: ./src/mem/page/pagefile.c
	i686-elf-gcc $(INCLUDES) -I./src/mem/page $(FLAGS) -std=gnu99 -c ./src/mem/page/pagefile.c -o ./build/mem/page/pagefile.o	

./build/mem/page/pagefile.asm.o: ./src/mem/page/pagefile.asm
	nasm -f elf -g ./src/mem/page/pagefile.asm -o ./build/mem/page/pagefile.asm.o

./build/io/vgaio/vgaio.o: ./src/io/vgaio/vgaio.c
	i686-elf-gcc $(INCLUDES) -I./src/io/vgaio/ $(FLAGS) -std=gnu99 -c ./src/io/vgaio/vgaio.c -o ./build/io/vgaio/vgaio.o	

./build/disk/disk.o: ./src/disk/disk.c
	i686-elf-gcc $(INCLUDES) -I./src/disk $(FLAGS) -std=gnu99 -c ./src/disk/disk.c -o ./build/disk/disk.o

./build/fs/fs.o: ./src/fs/fs.c
	i686-elf-gcc $(INCLUDES) -I./src/fs $(FLAGS) -std=gnu99 -c ./src/fs/fs.c -o ./build/fs/fs.o

./build/fs/fat/fat16.o: ./src/fs/fat/fat16.c
	i686-elf-gcc $(INCLUDES) -I./src/fs/ -I./src/fat $(FLAGS) -std=gnu99 -c ./src/fs/fat/fat16.c -o ./build/fs/fat/fat16.o

./build/fs/pparser.o: ./src/fs/pparser.c
	i686-elf-gcc $(INCLUDES) -I./src/fs $(FLAGS) -std=gnu99 -c ./src/fs/pparser.c -o ./build/fs/pparser.o

./build/disk/stream.o: ./src/disk/stream.c
	i686-elf-gcc $(INCLUDES) -I./src/disk $(FLAGS) -std=gnu99 -c ./src/disk/stream.c -o ./build/disk/stream.o

clean:
	rm -rf ./bin/mosboot.bin
	rm -rf ./bin/moskernel.bin
	rm -rf ./bin/mystos.bin
	rm -rf $(FILES)
	rm -rf ./build/mosfkrnl.o