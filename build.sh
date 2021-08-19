#/bin/bash
export PREFIX="$HOME/build-i686-elf/linux/output/"
export TARGET=i686-elf
export PATH="$PREFIX/bin:$PATH"
make all
