gcc -I lib/kernel/ -I lib/ -I device/ -Wall -m32 -fno-stack-protector -nostdinc -c -o build/main.o kernel/main.c
gcc -I lib/kernel/ -I lib/ -I device/ -Wall -m32 -fno-stack-protector -nostdinc -c -o build/interrupt.o kernel/interrupt.c
gcc -I lib/kernel/ -I lib/ -I device/ -Wall -m32 -fno-stack-protector -nostdinc -c -o build/init.o kernel/init.c
gcc -I lib/kernel/ -I lib/ -I device/ -Wall -m32 -fno-stack-protector -nostdinc -c -o build/timer.o device/timer.c
nasm -f elf -o ./build/print.o lib/kernel/print.asm
nasm -f elf -o ./build/kernel.o kernel/kernel.asm
ld -m elf_i386 -Ttext 0xc0001500 -e main -o build/kernel.bin build/main.o build/print.o build/init.o  build/interrupt.o build/kernel.o build/timer.o
dd if=build/kernel.bin of=./hd60M.img bs=512 count=200 seek=9 conv=notrunc

