gcc -I lib/kernel/ -I lib/ -Wall -m32 -fno-stack-protector -nostdinc -c -o build/main.o kernel/main.c
nasm -f elf -o ./build/print.o lib/kernel/print.asm
ld -m elf_i386 -Ttext 0xc0001500 -e main -o build/kernel.bin build/main.o build/print.o
dd if=build/kernel.bin of=./hd60M.img bs=512 count=200 seek=9 conv=notrunc

