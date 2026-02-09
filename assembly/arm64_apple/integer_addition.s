.global _main
.balign  4
_main:
  mov x0, #7 // C++: long x0 = 7;
  add x0, x0, #8 // C++: x0 += 8;
  mov x16, #1 ; // 1 is terminate call
  svc #0x80 // supervisor call
