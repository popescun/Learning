.global _main
.balign  4
_main:
  ; w<no> is for 32 bits, x<no> is for 64 bits
  mov x0, #7 ; this is the exit code for `exit`
  mov x16, #1 ; #1 is `exit`
  svc #0x80 ; supervisor call: executes the routine specified in register 16
