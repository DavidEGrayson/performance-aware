; Results on David's computer: 2 read ports

global read_loop1, read_loop2, read_loop3, read_loop4
global write_loop1, write_loop2, write_loop3, write_loop4

section .text

align 64
read_loop1:
  mov rax, [rdx]
  sub rcx, 1
  jnle read_loop1
  ret

align 64
read_loop2:
  mov rax, [rdx]
  mov rax, [rdx]
  sub rcx, 2
  jnle read_loop2
  ret

align 64
read_loop3:
  mov rax, [rdx]
  mov rax, [rdx]
  mov rax, [rdx]
  sub rcx, 3
  jnle read_loop3
  ret

align 64
read_loop4:
  mov rax, [rdx]
  mov rax, [rdx]
  mov rax, [rdx]
  mov rax, [rdx]
  sub rcx, 4
  jnle read_loop4
  ret

align 64
write_loop1:
  mov [rdx], rax
  sub rcx, 1
  jnle write_loop1
  ret

align 64
write_loop2:
  mov [rdx], rax
  mov [rdx + 1], rax
  sub rcx, 2
  jnle write_loop2
  ret

align 64
write_loop3:
  mov [rdx], rax
  mov [rdx + 1], rax
  mov [rdx + 2], rax
  sub rcx, 3
  jnle write_loop3
  ret

align 64
write_loop4:
  mov [rdx], rax
  mov [rdx + 1], rax
  mov [rdx + 2], rax
  mov [rdx + 3], rax
  sub rcx, 4
  jnle write_loop4
  ret
