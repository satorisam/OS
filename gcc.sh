gcc -Wall -m32 -fno-stack-protector -c -o build/main.o kernel/main.c
ld -m elf_i386 build/main.o -Ttext 0xc0001500 -e main -o build/kernel.bin
dd if=build/kernel.bin of=./hd60M.img bs=512 count=200 seek=9 conv=notrunc

