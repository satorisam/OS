nasm -I include/ -o ./build/mbr.bin ./boot/mbr.S
#nasm -I include/ -o loader.bin loader.S
dd if=./mbr.bin of=./hd60M.img bs=512 count=1 seek=0 conv=notrunc
#dd if=./loader.bin of=./hd60M.img bs=512 count=2 seek=2 conv=notrunc
