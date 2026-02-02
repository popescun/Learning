; Based on example from https://www.youtube.com/watch?v=3ixTKrE8lv8
.global _main
.balign 4
_main:
  // print
  mov x0, #1 // stdout = `1`
  adr x1, hello_world // adr = address to vector
  mov x2, 14 // 14 characters in our string
  mov x16, #4 // 4 is write syscall
  svc #0x80 // supervisor call

  // exit
  mov x0, #0
  mov x16, #1 // 1 is terminate call
  svc #0x80

hello_world: .ascii "hello, world!\n"