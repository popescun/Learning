.global _main
.balign  4
_main:
  fmov d0, #2.0 // goal: compute 1/sqrt(x) for x = 2
  fmov d1, #0.5 // initial guess: y0

  fmul d2, d1, d1 // square y0: z = y0*y0
  frsqrts d3, d0, d2 // (3 - x*z)/2
  fmul d1, d1, d3 // y1 = y0*(3 - x*z)/2

  // second iteration
  fmul d2, d1, d1 // second guess squared
  frsqrts d3, d0, d2
  fmul d1, d1, d3

  // third iteration
  fmul d2, d1, d1 // third guess squared
  frsqrts d3, d0, d2
  fmul d1, d1, d3

  mov x0, #0
  mov x16, #1
  svc #0x80