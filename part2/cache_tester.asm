; word = 16
; dword = 32
; qword = 64
; dqword = 128 bits, 16 bytes

global read_loop

section .text

; rcx = arg 0 = data_length
; rdx = arg 1 = data
; r8  = arg 2 = mask
; r9  = bytes read
; r10 = temporary pointer
align 64
read_loop:
  mov r9, 0
loop:
  mov r10, r9
  and r10, r8
  add r10, rdx
  movdqu xmm0, [r10 + 0]
  movdqu xmm1, [r10 + 16]
  movdqu xmm2, [r10 + 32]
  movdqu xmm3, [r10 + 48]
  add r9, 64
  sub rcx, 64
  jnle loop
  ret
