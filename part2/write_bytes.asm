global mov_all_bytes_asm

section .text

mov_all_bytes_asm:
  xor rax, rax
.loop:
  ;mov [rdx + rax], al
  ; nop
  ; nop
  ; nop
  ; nop
  ; nop

  ; nop
  ; nop
  ; nop
  ; nop
  ; nop

  ; nop
  ; nop

  inc rax
  cmp rax, rcx
  jb .loop
  ret
