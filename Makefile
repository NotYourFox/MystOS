FILES = ./build/moskernel.asm.o ./build/moskernel.o ./build/disk/stream.o ./build/idt/idt.asm.o ./build/idt/idt.o ./build/mem/mem.o ./build/io/io.asm.o ./build/mem/heap/heap.o ./build/mem/heap/kheap.o ./build/mem/page/pagefile.o ./build/mem/page/pagefile.asm.o ./build/io/vgaio/vgaio.o ./build/disk/disk.o ./build/fs/pparser.o ./build/fs/vfs.o ./build/fs/fat/fat16.o ./build/cmos/cmos.o ./build/idt/irq/irq.o ./build/idt/irq/irq0.o ./build/gdt/gdt.o ./build/gdt/gdt.asm.o ./build/task/tss.asm.o #./build/cpu/cpu.asm.o ./build/cpu/cpu.o
INCLUDES = -I./src
FLAGS = -g -ffreestanding -falign-jumps -falign-functions -falign-labels -falign-loops -fstrength-reduce -fomit-frame-pointer -finline-functions -Wno-unused-function -fno-builtin -Werror -Wno-unused-label -Wno-cpp -Wno-unused-parameter -nostdlib -nostartfiles -nodefaultlibs -Wall -O0 -Iinc

all: ./bin/mosboot.bin ./bin/moskernel.bin $(FILES)
	rm -rf ./bin/mystos.bin
	dd if=./bin/mosboot.bin >> ./bin/mystos.bin
	dd if=./bin/moskernel.bin >> ./bin/mystos.bin
	dd if=/dev/zero bs=1048576 count=16 >> ./bin/mystos.bin
	sudo mount -t vfat ./bin/mystos.bin /mnt/d
	sudo cp ./file.txt /mnt/d
	sudo umount /mnt/d
	
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

./build/fs/vfs.o: ./src/fs/vfs.c
	i686-elf-gcc $(INCLUDES) -I./src/fs $(FLAGS) -std=gnu99 -c ./src/fs/vfs.c -o ./build/fs/vfs.o

./build/fs/fat/fat16.o: ./src/fs/fat/fat16.c
	i686-elf-gcc $(INCLUDES) -I./src/fs/ -I./src/fat $(FLAGS) -std=gnu99 -c ./src/fs/fat/fat16.c -o ./build/fs/fat/fat16.o

./build/fs/pparser.o: ./src/fs/pparser.c
	i686-elf-gcc $(INCLUDES) -I./src/fs $(FLAGS) -std=gnu99 -c ./src/fs/pparser.c -o ./build/fs/pparser.o

./build/disk/stream.o: ./src/disk/stream.c
	i686-elf-gcc $(INCLUDES) -I./src/disk $(FLAGS) -std=gnu99 -c ./src/disk/stream.c -o ./build/disk/stream.o

./build/cmos/cmos.o: ./src/cmos/cmos.c
	i686-elf-gcc $(INCLUDES) -I./src/cmos $(FLAGS) -std=gnu99 -c ./src/cmos/cmos.c -o ./build/cmos/cmos.o

./build/idt/irq/irq.o: ./src/idt/irq/irq.c
	i686-elf-gcc $(INCLUDES) -I./src/irq $(FLAGS) -std=gnu99 -c ./src/idt/irq/irq.c -o ./build/idt/irq/irq.o

./build/idt/irq/irq0.o: ./src/idt/irq/irq0.c
	i686-elf-gcc $(INCLUDES) -I./src/irq $(FLAGS) -std=gnu99 -c ./src/idt/irq/irq0.c -o ./build/idt/irq/irq0.o

#./build/cpu/cpu.asm.o: ./src/cpu/cpu.asm
#	nasm -f elf -g ./src/cpu/cpu.asm -o ./build/cpu/cpu.asm.o

#./build/cpu/cpu.o: ./src/cpu/cpu.c
#	i686-elf-gcc $(INCLUDES) -I./src/cpu $(FLAGS) -std=gnu99 -c ./src/cpu/cpu.c -o ./build/cpu/cpu.o

./build/gdt/gdt.o: ./src/gdt/gdt.c
	i686-elf-gcc $(INCLUDES) -I./src/gdt $(FLAGS) -std=gnu99 -c ./src/gdt/gdt.c -o ./build/gdt/gdt.o

./build/gdt/gdt.asm.o: ./src/gdt/gdt.asm
	nasm -f elf -g ./src/gdt/gdt.asm -o ./build/gdt/gdt.asm.o

./build/task/tss.asm.o: ./src/task/tss.asm
	nasm -f elf -g ./src/task/tss.asm -o ./build/task/tss.asm.o

clean:
	rm -rf ./bin/mosboot.bin
	rm -rf ./bin/moskernel.bin
	rm -rf ./bin/mystos.bin
	rm -rf $(FILES)
	rm -rf ./build/mosfkrnl.o