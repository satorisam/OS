%include "boot.inc"

SECTION loader vstart=LOADER_BASE_ADDR
    mov byte [gs:0x00],'2'
    mov byte [gs:0x02],' '
    mov byte [gs:0x04],'L'
    mov byte [gs:0x06],'O'
    mov byte [gs:0x08],'A'
    mov byte [gs:0x0a],'D'
    mov byte [gs:0x0c],'E'
    mov byte [gs:0x0e],'R'

    jmp $
