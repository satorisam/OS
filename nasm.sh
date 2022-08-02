nasm -I include/ -o ./build/mbr.bin ./boot/mbr.asm
nasm -I include/ -o ./build/loader.bin ./boot/loader.asm
dd if=./build/mbr.bin of=./hd60M.img bs=512 count=1 seek=0 conv=notrunc
dd if=./build/loader.bin of=./hd60M.img bs=512 count=6 seek=2 conv=notrunc
